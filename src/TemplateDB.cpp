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
                int vel_x = templateDocument.HasMember("vel_x") ? templateDocument["vel_x"].GetFloat() : 0;
                int vel_y = templateDocument.HasMember("vel_y") ? templateDocument["vel_y"].GetFloat() : 0;
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
                std::string viewImageBackName = templateDocument.HasMember("view_image_back") ? templateDocument["view_image_back"].GetString() : "";

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

                bool movementBounce = templateDocument.HasMember("movement_bounce_enabled") ? templateDocument["movement_bounce_enabled"].GetBool() : false;

                SpriteRenderer* spriteRenderer = new SpriteRenderer(viewImageName, glm::dvec2(viewPivotOffsetX, viewPivotOffsetY), renderOrder, viewImageBackName, movementBounce);

                // Collider
                Collider* collider = nullptr;
                float colliderWidth = templateDocument.HasMember("box_collider_width") ? templateDocument["box_collider_width"].GetFloat() : 0;
                float colliderHeight = templateDocument.HasMember("box_collider_height") ? templateDocument["box_collider_height"].GetFloat() : 0;
                         
                if (colliderWidth != 0 && colliderHeight != 0)
                {
                    collider = new Collider(colliderWidth, colliderHeight);
                }

                // Trigger Collider
                TriggerCollider* triggerCollider = nullptr;
                float triggerColliderWidth = templateDocument.HasMember("box_trigger_width") ? templateDocument["box_trigger_width"].GetFloat() : 0;
                float triggerColliderHeight = templateDocument.HasMember("box_trigger_height") ? templateDocument["box_trigger_height"].GetFloat() : 0;

                if (triggerColliderWidth != 0 && triggerColliderHeight != 0)
                {
                    triggerCollider = new TriggerCollider(triggerColliderWidth, triggerColliderHeight);
                }

                // Create the Entity object
                Template* newEntityTemplate = new Template(
                    templateName, name, glm::vec2(vel_x, vel_y), nearbyDialogue, contactDialogue, transform, spriteRenderer, collider, triggerCollider
                );

                templates[templateName] = newEntityTemplate;
            }
        }
    }
}