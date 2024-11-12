#include "SceneDB.h"

std::vector<Entity*> SceneDB::entities;
std::vector<Entity*> SceneDB::entitiesToInstantiate;
std::vector<Entity*> SceneDB::entitiesToDestroy;
std::string SceneDB::currSceneName;
std::string SceneDB::pendingSceneName;
bool SceneDB::pendingScene = false;
int SceneDB::totalEntities = 0;

SceneDB::~SceneDB()
{
    for (int i = 0; i < entities.size(); i++)
    {
        delete entities[i];
    }
}

void SceneDB::RequestLoadNewScene(const std::string& sceneName)
{
    pendingScene = true;
    pendingSceneName = sceneName;
}

void SceneDB::LoadPendingScene()
{
    pendingScene = false;

    // reset camera and zoom if applicable                                                                         
    Renderer::RendererResetDefaults();

    std::vector<Entity*> dontDestroy;
    // delete entities that arent dontDestroy
    for (int i = 0; i < entities.size(); i++)
    {
        if (entities[i]->destroyOnLoad)
        {
            delete entities[i];
        }
        else
        {
            dontDestroy.push_back(entities[i]);
        }
    }

    entities.clear();
    entitiesToInstantiate.clear(); // NOTE: if a do not destroy entity is instantiate and scene is reloaded same frame, itll be destroyed
    entitiesToDestroy.clear();

    // add dont destroy entities back into entities list
    for (Entity* entity : dontDestroy)
    {
        entities.push_back(entity);
    }

    LoadScene(pendingSceneName);
    
    // run start on entities that werent in dontDestroy vector
    for (int i = dontDestroy.size(); i < entities.size(); i++)
    {
        entities[i]->Start();
    }
}

void SceneDB::LoadScene(const std::string& sceneName)
{
    // load scene
    currSceneName = sceneName;

	std::string scenePath = "resources/scenes/" + sceneName + ".scene";
	if (!std::filesystem::exists(scenePath))
	{
		std::cout << "error: scene " << sceneName << ".scene is missing";
		exit(0);
	}

    LoadEntitiesInScene(sceneName);

}

void SceneDB::LoadEntitiesInScene(const std::string& sceneName)
{
    lua_State* luaState = LuaStateManager::GetLuaState();

    std::string scenePath = "resources/scenes/" + sceneName + ".scene";

    rapidjson::Document configDocument;
    EngineUtils::ReadJsonFile(scenePath, configDocument);

    // Ensure the document contains an array named "actors"
    if (!configDocument.HasMember("actors") || !configDocument["actors"].IsArray())
    {
        std::cout << "error: no actors found in scene " << sceneName;
        exit(0);
    }

    // Get the actors array
    const rapidjson::Value& actors = configDocument["actors"];

    // Loop through each actor in the JSON array
    for (rapidjson::SizeType actorIndex = 0; actorIndex < actors.Size(); actorIndex++)
    {
        const rapidjson::Value& actor = actors[actorIndex];

        // Set base values
        std::string name = "";
        std::unordered_map<std::string, Component*> componentMap;

        if (actor.HasMember("template"))
        {
            std::string templateName = actor["template"].GetString();
            Template* entityTemplate = TemplateDB::FindEntity(templateName);
            if (entityTemplate != nullptr)
            {
                // Extract values from the JSON
                name = entityTemplate->entityName;
            }
            else
            {
                std::cout << "error: template " << templateName << " is missing";
                exit(0);
            }

            for (auto component : entityTemplate->components)
            {
                luabridge::LuaRef instanceTable = luabridge::newTable(luaState);
                instanceTable["key"] = component.first;

                luabridge::LuaRef parentTable = *(component.second->luaRef);
                ComponentDB::EstablishInheritance(instanceTable, parentTable);

                std::shared_ptr<luabridge::LuaRef> instanceTablePtr = std::make_shared<luabridge::LuaRef>(instanceTable);
                componentMap[component.first] = new Component(instanceTablePtr, component.second->type, component.second->hasStart, component.second->hasUpdate, component.second->hasLateUpdate);
            }
        }

        // Extract values from the JSON
        // Override template if applicable
        name = actor.HasMember("name") ? actor["name"].GetString() : name;

        // Loop through each component in the JSON array
        if (actor.HasMember("components"))
        {
            const rapidjson::Value& components = actor["components"];

            // Loop through each component in the JSON array
            for (rapidjson::Value::ConstMemberIterator itr = components.MemberBegin(); itr != components.MemberEnd(); ++itr) {
                // Extract the component's name (e.g., "first_component", "second_component")
                std::string componentName = itr->name.GetString();

                // Extract the component's type (e.g., the object containing the "type" field)
                const rapidjson::Value& component = itr->value;

                // Check if the component has a "type" field and extract it
                if (component.HasMember("type")) 
                {
                    std::string componentType = component["type"].GetString();

                    if (ComponentDB::components.find(componentType) != ComponentDB::components.end())
                    {
                        luabridge::LuaRef instanceTable = luabridge::newTable(luaState);
                        instanceTable["key"] = componentName;

                        // establish inheritance from default component type
                        Component* parentComponent = ComponentDB::components[componentType];
                        luabridge::LuaRef parentTable = *(parentComponent->luaRef);
                        ComponentDB::EstablishInheritance(instanceTable, parentTable);
                          
                        // inject property overrides
                        for (auto propItr = component.MemberBegin(); propItr != component.MemberEnd(); ++propItr) {
                            std::string propName = propItr->name.GetString();

                            if (propName != "type") 
                            { // Exclude "type" field itself
                                if (propItr->value.IsString()) 
                                {
                                    instanceTable[propName] = propItr->value.GetString();
                                }
                                else if (propItr->value.IsInt())
                                {
                                    instanceTable[propName] = propItr->value.GetInt();
                                }
                                else if (propItr->value.IsDouble()) 
                                {
                                    instanceTable[propName] = propItr->value.GetDouble();
                                }
                                else if (propItr->value.IsBool()) 
                                {
                                    instanceTable[propName] = propItr->value.GetBool();
                                }
                                else if (propItr->value.IsArray()) 
                                {
                                    // Create a Lua table for arrays
                                    luabridge::LuaRef luaArray = luabridge::newTable(luaState);
                                    int index = 1; // Lua uses 1-based indexing

                                    // Iterate over array elements
                                    for (auto& arrayElem : propItr->value.GetArray()) 
                                    {
                                        if (arrayElem.IsString()) 
                                        {
                                            luaArray[index++] = arrayElem.GetString();
                                        }
                                        else if (arrayElem.IsInt()) 
                                        {
                                            luaArray[index++] = arrayElem.GetInt();
                                        }
                                        else if (arrayElem.IsDouble()) 
                                        {
                                            luaArray[index++] = arrayElem.GetDouble();
                                        }
                                        else if (arrayElem.IsBool()) 
                                        {
                                            luaArray[index++] = arrayElem.GetBool();
                                        }
                                    }

                                    instanceTable[propName] = luaArray;
                                }
                                else
                                {
                                    std::cout << "error: could not override " << propName << " because type is not supported!";
                                    exit(0);
                                }
                            }
                        }

                        std::shared_ptr<luabridge::LuaRef> instanceTablePtr = std::make_shared<luabridge::LuaRef>(instanceTable);
                        componentMap[componentName] = new Component(instanceTablePtr, componentType, parentComponent->hasStart, parentComponent->hasUpdate, parentComponent->hasLateUpdate);
                    }
                    else
                    {
                        std::cout << "error: failed to locate component " << componentName;
                        exit(0);
                    }
                }
                else {
                    std::cout << "error: component " << componentName << " is missing a type" << std::endl;
                    exit(0);
                }
            }
        }

        // Create the Entity object
        Entity* entity = new Entity(
            name, componentMap 
        );

        // Add the entity to the entities vector
        entities.push_back(entity);

        entity->entityID = totalEntities;
        totalEntities++;
    }
}

void SceneDB::Start()
{
    for (Entity* entity : entities)
    {
        entity->Start();
    }
}

void SceneDB::Update()
{
    std::vector<Entity*> newEntities = entitiesToInstantiate;

    // clear array of new entities since we may want to instantiate more entities in these start functions
    entitiesToInstantiate.clear();

    // add new entities
    for (Entity* entity : newEntities)
    {
        entities.push_back(entity);

        entity->entityID = totalEntities;
        totalEntities++;

        entity->Start();
    }

    for (Entity* entity : entities)
    {
        if (!entity->wasDestroyed)
        {
            entity->Update();
        }
    }
}

void SceneDB::LateUpdate()
{
    for (Entity* entity : entities)
    {
        if (!entity->wasDestroyed)
        {
            entity->LateUpdate();
        }
    }

    // handle clean up for entities we want to destroy
    for (auto& entity : entitiesToDestroy)
    {
        int indexOfEntityInList = -1;
        // remove from entities vector
        for (int i = 0; i < entities.size(); i++)
        {
            if (entity == entities[i])
            {
                indexOfEntityInList = i;
            }
        }

        if (indexOfEntityInList != -1)
        {
            entities.erase(entities.begin() + indexOfEntityInList);
        }

        delete(entity);
    }

    entitiesToDestroy.clear();
}

Entity* SceneDB::Find(const std::string& name)
{
    for (auto entity : entities)
    {
        if (!entity->wasDestroyed && entity->entityName == name)
        {   
            return entity;
        }
    }

    // entities that are planned to be added but not yet to entities list
    for (auto entity : entitiesToInstantiate)
    {
        if (!entity->wasDestroyed && entity->entityName == name)
        {
            return entity;
        }
    }

    return luabridge::LuaRef(LuaStateManager::GetLuaState());
}

luabridge::LuaRef SceneDB::FindAll(const std::string& name)
{
    luabridge::LuaRef entityTable = luabridge::newTable(LuaStateManager::GetLuaState());
    int index = 1;

    for(auto entity : entities)
    {
        if (!entity->wasDestroyed && entity->entityName == name)
        {
            entityTable[index] = entity;
            index++;
        }
    }

    // entities that are planned to be added but not yet to entities list
    for (auto entity : entitiesToInstantiate)
    {
        if (!entity->wasDestroyed && entity->entityName == name)
        {
            entityTable[index] = entity;
            index++;
        }
    }

    return entityTable;
}

Entity* SceneDB::Instantiate(const std::string& entityTemplateName)
{
    lua_State* luaState = LuaStateManager::GetLuaState();
    std::string entityName = "";
    std::unordered_map<std::string, Component*> componentMap;

    Template* entityTemplate = TemplateDB::FindEntity(entityTemplateName);
    if (entityTemplate != nullptr)
    {
        // Extract values from the JSON
        entityName = entityTemplate->entityName;
    }
    else
    {
        std::cout << "error: template " << entityTemplateName << " is missing";
        exit(0);
    }

    for (auto component : entityTemplate->components)
    {
        luabridge::LuaRef instanceTable = luabridge::newTable(luaState);
        instanceTable["key"] = component.first;

        luabridge::LuaRef parentTable = *(component.second->luaRef);
        ComponentDB::EstablishInheritance(instanceTable, parentTable);

        std::shared_ptr<luabridge::LuaRef> instanceTablePtr = std::make_shared<luabridge::LuaRef>(instanceTable);
        componentMap[component.first] = new Component(instanceTablePtr, component.second->type, component.second->hasStart, component.second->hasUpdate, component.second->hasLateUpdate);
    }


    // Create the Entity object
    Entity* entity = new Entity(
        entityName, componentMap
    );

    entitiesToInstantiate.push_back(entity);
    return entity;
}

void SceneDB::Destroy(Entity* entity)
{
    if (entity != luabridge::LuaRef(LuaStateManager::GetLuaState()))
    {
        if (entity->wasDestroyed)
        {
            std::cout << "\033[31m" << "error: attempting to destroy an entity that does not exist" << "\033[0m" << "\n";
            return;
        }

        entitiesToDestroy.push_back(entity);
        entity->wasDestroyed = true;

        // turn off all components
        // NOTE: may not be necessary bc of wasDestroyed variable
        for (auto component : entity->components)
        {
            (*component.second->luaRef)["enabled"] = false; 
        }

        // remove from added entities vector (if applicable)
        int indexOfEntityInList = -1;
        // remove from entities vector
        for (int i = 0; i < entitiesToInstantiate.size(); i++)
        {
            if (entity == entitiesToInstantiate[i])
            {
                indexOfEntityInList = i;
            }
        }

        if (indexOfEntityInList != -1)
        {
            entitiesToInstantiate.erase(entitiesToInstantiate.begin() + indexOfEntityInList);
        }
    }

}

void SceneDB::DontDestroy(Entity* entity)
{
    entity->destroyOnLoad = false;
}

uint64_t SceneDB::GetNumberOfEntitiesInScene()
{
    return entities.size();
}

Entity* SceneDB::GetEntityAtIndex(int index)
{
    if (index >= entities.size() || index < 0)
    {
        return NULL;
    }

    return entities[index];
}






