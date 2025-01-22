#include "SceneDB.h"

std::vector<Entity*> SceneDB::entities;
std::vector<Entity*> SceneDB::entitiesToInstantiate;
std::vector<Entity*> SceneDB::entitiesToDestroy;

std::string SceneDB::currSceneName;
std::string SceneDB::pendingSceneName;
std::string SceneDB::currScenePath;
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
            entities[i]->OnDestroy();

            delete entities[i];
        }
        else
        {
            dontDestroy.push_back(entities[i]);
        }
    }

    // TODO: should I destroy b2 world? what if we load up menu scene with no simulations?

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

	currScenePath = "resources/scenes/" + sceneName + ".scene";
	if (!std::filesystem::exists(currScenePath))
	{
		std::cout << "error: scene " << sceneName << ".scene is missing";
		exit(0);
	}

    LoadEntitiesInScene(sceneName);

}

void SceneDB::LoadEntitiesInScene(const std::string& sceneName)
{
    lua_State* luaState = LuaStateManager::GetLuaState();

    currScenePath = "resources/scenes/" + sceneName + ".scene";

    rapidjson::Document configDocument;
    EngineUtils::ReadJsonFile(currScenePath, configDocument);

    // Ensure the document contains an array named "actors"
    if (!configDocument.HasMember("entities") || !configDocument["entities"].IsArray())
    {
        std::cout << "error: no actors found in scene " << sceneName;
        exit(0);
    }

    // Get the actors array
    const rapidjson::Value& entitiesJson = configDocument["entities"];

    // Loop through each actor in the JSON array
    for (rapidjson::SizeType entityIndex = 0; entityIndex < entitiesJson.Size(); entityIndex++)
    {
        const rapidjson::Value& entityJson = entitiesJson[entityIndex];

        // Set base values
        std::string name = "";
        std::unordered_map<std::string, Component*> componentMap;

        if (entityJson.HasMember("template"))
        {
            std::string templateName = entityJson["template"].GetString();
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
                luabridge::LuaRef instanceTable = ComponentDB::CreateInstanceTableFromTemplate(component.first, component.second->type, *(component.second->luaRef));

                std::shared_ptr<luabridge::LuaRef> instanceTablePtr = std::make_shared<luabridge::LuaRef>(instanceTable);
                componentMap[component.first] = new Component(instanceTablePtr, component.second->type, component.second->hasStart, component.second->hasUpdate, component.second->hasLateUpdate);
            }
        }

        // Extract values from the JSON
        // Override template if applicable
        name = entityJson.HasMember("name") ? entityJson["name"].GetString() : name;

        // Loop through each component in the JSON array
        if (entityJson.HasMember("components"))
        {
            const rapidjson::Value& components = entityJson["components"];

            // Loop through each component in the JSON array
            for (rapidjson::Value::ConstMemberIterator itr = components.MemberBegin(); itr != components.MemberEnd(); ++itr) {
                // Extract the component's name (e.g., "first_component", "second_component")
                std::string componentName = itr->name.GetString();

                // Extract the component's type (e.g., the object containing the "type" field)
                const rapidjson::Value& component = itr->value;

                componentMap[componentName] = ComponentDB::LoadComponentInstance(component, componentName);
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
        // Incase entity was somehow already destroy
        if (entity)
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

            entity->OnDestroy();

            delete(entity);
        }
        else
        {
            ErrorHandling::ReportString("Attempted to destroy entity that was already destroyed");
        }
        
    }

    entitiesToDestroy.clear();

    // Handle Events that were published this frame
    Event::UpdateSubscriptions();

    // step through physics if b2WorldInstance exists
    if (b2WorldDB::b2WorldInstance != nullptr)
    {
        b2WorldDB::b2WorldInstance->Step(1.0f / 60.0f, 8, 3);
    }
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

    
    // create empty entity
    if (entityTemplateName == "")
    {
        entityName = "Entity";
    }
    else
    {
        Template* entityTemplate = TemplateDB::FindEntity(entityTemplateName);
        if (entityTemplate != nullptr)
        {
            // Extract values from the JSON
            entityName = entityTemplate->entityName;
        }
        else
        {
            std::cout << "error: template " << entityTemplateName << " is missing";
            DebugDB::AddStatement(DebugType::LogError, "", "", "template " + entityTemplateName + " is missing");
            return nullptr;
            //exit(0);
        }

        for (auto component : entityTemplate->components)
        {
            luabridge::LuaRef instanceTable = ComponentDB::CreateInstanceTableFromTemplate(component.first, component.second->type, *(component.second->luaRef));

            std::shared_ptr<luabridge::LuaRef> instanceTablePtr = std::make_shared<luabridge::LuaRef>(instanceTable);
            componentMap[component.first] = new Component(instanceTablePtr, component.second->type, component.second->hasStart, component.second->hasUpdate, component.second->hasLateUpdate);
        }
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

            DebugDB::AddStatement(DebugType::LogError, "", "", "attempting to destroy an entity that does not exist");
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

// WARNING
// THIS IS ONLY TO BE CALLED FROM THE EDITOR WHEN WE ARE NOT SIMULATING
void SceneDB::RemoveEntityOutOfSimulation(Entity* entity)
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
        totalEntities--;
    }

    delete(entity);

}

void SceneDB::DontDestroy(Entity* entity)
{
    entity->destroyOnLoad = false;
}

void SceneDB::Reset()
{
    for (int i = 0; i < entities.size(); i++)
    {
        delete entities[i];
    }

    entities.clear();
    entitiesToInstantiate.clear();
    entitiesToDestroy.clear();

    // reset physics
    delete(b2WorldDB::b2WorldInstance);
    b2WorldDB::b2WorldInstance = nullptr;
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






