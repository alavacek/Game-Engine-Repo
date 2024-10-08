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

    if (std::filesystem::exists(templatesPath) && std::filesystem::is_directory(templatesPath)) {
        for (const auto& entry : std::filesystem::directory_iterator(templatesPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".template") {
                // Pass the full path to the ReadJsonFile function
                std::string fullPath = entry.path().string();
                EngineUtils::ReadJsonFile(fullPath, templateDocument);

                // Extract values from the JSON
                std::string name = templateDocument.HasMember("name") ? templateDocument["name"].GetString() : "";
                char view = templateDocument.HasMember("view") ? templateDocument["view"].GetString()[0] : '?';
                int vel_x = templateDocument.HasMember("vel_x") ? templateDocument["vel_x"].GetInt() : 0;
                int vel_y = templateDocument.HasMember("vel_y") ? templateDocument["vel_y"].GetInt() : 0;
                bool blocking = templateDocument.HasMember("blocking") ? templateDocument["blocking"].GetBool() : false;
                std::string nearbyDialogue = templateDocument.HasMember("nearby_dialogue") ? templateDocument["nearby_dialogue"].GetString() : "";
                std::string contactDialogue = templateDocument.HasMember("contact_dialogue") ? templateDocument["contact_dialogue"].GetString() : "";

                std::string templateName = entry.path().stem().string();

                // Transform
                int transformPositionX = templateDocument.HasMember("transform_position_x") ? templateDocument["transform_position_x"].GetInt() : 0;
                int transformPositionY = templateDocument.HasMember("transform_position_y") ? templateDocument["transform_position_y"].GetInt() : 0;

                int transformScaleX = templateDocument.HasMember("transform_scale_x") ? templateDocument["transform_scale_x"].GetFloat() : 1.0;
                int transformScaleY = templateDocument.HasMember("transform_scale_y") ? templateDocument["transform_scale_y"].GetFloat() : 1.0;

                int transformRotationDegrees = templateDocument.HasMember("transform_rotation_degrees") ? templateDocument["transform_rotation_degrees"].GetDouble() : 0.0;

                Transform* transform = new Transform(glm::ivec2(transformPositionX, transformPositionY), glm::vec2(transformScaleX, transformScaleY), transformRotationDegrees);

                // Sprite Renderer
                std::string viewImageName = templateDocument.HasMember("view_image") ? templateDocument["view_image"].GetString() : "";

                double viewPivotOffsetX = templateDocument.HasMember("view_pivot_offset_x") ? templateDocument["view_pivot_offset_x"].GetDouble() : -1.0;
                double viewPivotOffsetY = templateDocument.HasMember("view_pivot_offset_y") ? templateDocument["view_pivot_offset_y"].GetDouble() : -1.0;

                std::optional<int> renderOrder;
                if (templateDocument.HasMember("render_order")) 
                {
                    renderOrder = templateDocument["render_order"].GetInt();
                }
                else 
                {
                    renderOrder = std::nullopt;
                }

                SpriteRenderer* spriteRenderer = new SpriteRenderer(viewImageName, glm::dvec2(viewPivotOffsetX, viewPivotOffsetY), renderOrder);

                // Create the Entity object
                Template* newEntityTemplate = new Template(
                    templateName, name, view, glm::ivec2(vel_x, vel_y), blocking, nearbyDialogue, contactDialogue, transform, spriteRenderer
                );

                templates[templateName] = newEntityTemplate;
            }
        }
    }
}