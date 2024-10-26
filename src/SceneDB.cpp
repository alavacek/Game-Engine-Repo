#include "SceneDB.h"

SceneDB::~SceneDB()
{
    for (int i = 0; i < entities.size(); i++)
    {
        delete entities[i];
    }
}

void SceneDB::LoadScene(const std::string& sceneName)
{
	currSceneName = sceneName + ".scene";

	std::string scenePath = "resources/scenes/" + sceneName + ".scene";
	if (!std::filesystem::exists(scenePath))
	{
		std::cout << "error: scene " << sceneName << " is missing";
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

    // Clear the entities vector in case it's been used before
    entityRenderOrder.clear();

    // Loop through each actor in the JSON array
    for (rapidjson::SizeType actorIndex = 0; actorIndex < actors.Size(); actorIndex++)
    {
        const rapidjson::Value& actor = actors[actorIndex];

        // Set base values
        std::string name = "";

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
        }

        // Extract values from the JSON
        // Override template if applicable
        name = actor.HasMember("name") ? actor["name"].GetString() : name;
        std::unordered_map<std::string, std::shared_ptr<luabridge::LuaRef>> componentMap;

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
                if (component.HasMember("type")) {
                    std::string componentType = component["type"].GetString();

                    if (ComponentDB::componentFiles.find(componentType) != ComponentDB::componentFiles.end())
                    {
                        luabridge::LuaRef instanceTable = luabridge::newTable(luaState);
                        instanceTable["key"] = componentName;

                        luabridge::LuaRef parentTable = *(ComponentDB::componentFiles[componentType]);
                        ComponentDB::EstablishInheritance(instanceTable, parentTable);
                          
                        std::shared_ptr<luabridge::LuaRef> instanceTablePtr = std::make_shared<luabridge::LuaRef>(instanceTable);
                        componentMap[componentName] = instanceTablePtr;
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
        entityRenderOrder.push_back(entity);
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

}

uint64_t SceneDB::GetNumberOfEntitiesInScene()
{
    return entityRenderOrder.size();
}

Entity* SceneDB::GetEntityAtIndex(int index)
{
    if (index >= entities.size() || index < 0)
    {
        return NULL;
    }

    return entities[index];
}

uint64_t SceneDB::GetSceneMaxHeight()
{
    return maxHeight;
}

uint64_t SceneDB::GetSceneMaxWidth()
{
    return maxWidth;
}





