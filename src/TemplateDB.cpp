#include "TemplateDB.h"

// Definition of the static member
std::unordered_map<std::string, Template*> TemplateDB::templates;

Template* TemplateDB::FindEntity(std::string templateName)
{
	return templates.find(templateName) != templates.end() ? templates[templateName] : nullptr;
}

void TemplateDB::LoadTemplates()
{
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

                        componentMap[componentName] = ComponentDB::LoadComponentInstance(component, componentName);
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

void TemplateDB::Reset()
{
    for (auto temp : templates)
    {
        delete(temp.second);
    }

    templates.clear();

}