#include "TemplateDB.h"

// Definition of the static member
std::unordered_map<std::string, Template*> TemplateDB::templates;

Template* TemplateDB::FindEntity(std::string templateName)
{
	return templates.find(templateName) != templates.end() ? templates[templateName] : nullptr;
}

void TemplateDB::LoadTemplates()
{
    lua_State* luaState = LuaStateManager::GetLuaState();

    // template config
    rapidjson::Document templateDocument;
    std::string templatesPath = "resources/actor_templates/";

    if (std::filesystem::exists(templatesPath) && std::filesystem::is_directory(templatesPath)) 
    {
        for (const auto& entry : std::filesystem::directory_iterator(templatesPath)) 
        {
            if (entry.is_regular_file() && entry.path().extension() == ".template") 
            {
                // Pass the full path to the ReadJsonFile function
                std::string fullPath = entry.path().string();
                EngineUtils::ReadJsonFile(fullPath, templateDocument);

                // Extract values from the JSON
                std::string name = templateDocument.HasMember("name") ? templateDocument["name"].GetString() : "";
                std::string templateName = entry.path().stem().string();

                std::unordered_map<std::string, Component*> componentMap;

                // Loop through each component in the JSON array
                if (templateDocument.HasMember("components"))
                {
                    const rapidjson::Value& components = templateDocument["components"];

                    // Loop through each component in the JSON array
                    for (rapidjson::Value::ConstMemberIterator itr = components.MemberBegin(); itr != components.MemberEnd(); ++itr) 
                    {
                        // Extract the component's name (e.g., "first_component", "second_component")
                        std::string componentName = itr->name.GetString();

                        // Extract the component's type (e.g., the object containing the "type" field)
                        const rapidjson::Value& component = itr->value;

                        // Check if the component has a "type" field and extract it
                        if (component.HasMember("type")) {
                            std::string componentType = component["type"].GetString();

                            if (ComponentDB::components.find(componentType) != ComponentDB::components.end())
                            {
                                luabridge::LuaRef instanceTable = luabridge::newTable(luaState);
                                instanceTable["key"] = componentName;

                                Component* parentComponent = ComponentDB::components[componentType];
                                luabridge::LuaRef parentTable = *(parentComponent->luaRef);
                                ComponentDB::EstablishInheritance(instanceTable, parentTable);

                                // inject property ovverrides
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
                Template* newEntityTemplate = new Template(
                    templateName, name, componentMap
                );

                templates[templateName] = newEntityTemplate;
            }
        }
    }
}