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
    entities.clear();
    entityRenderOrder.clear();


    // Loop through each actor in the JSON array
    for (rapidjson::SizeType i = 0; i < actors.Size(); i++)
    {
        const rapidjson::Value& actor = actors[i];

        // Set base values
        std::string name = "";
        char view = '?';
        int vel_x = 0;
        int vel_y = 0;
        bool blocking = false;
        std::string nearbyDialogue = "";
        std::string contactDialogue = "";

        Transform* transform = nullptr;
        SpriteRenderer* spriteRenderer = nullptr;

        if (actor.HasMember("template"))
        {
            std::string templateName = actor["template"].GetString();
            Template* entityTemplate = TemplateDB::FindEntity(templateName);
            if (entityTemplate != nullptr)
            {
                // Extract values from the JSON
                name = entityTemplate->entityName;
                view = entityTemplate->view;
                vel_x = entityTemplate->velocity.x;
                vel_y = entityTemplate->velocity.y;
                blocking = entityTemplate->blocking;
                nearbyDialogue = entityTemplate->nearbyDialogue;
                contactDialogue = entityTemplate->contactDialogue;

                transform = new Transform(entityTemplate->transform->position, entityTemplate->transform->scale, entityTemplate->transform->rotationDegrees);
                spriteRenderer = new SpriteRenderer(entityTemplate->spriteRenderer->viewImageName, entityTemplate->spriteRenderer->viewPivotOffset, entityTemplate->spriteRenderer->renderOrder);

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
        view = actor.HasMember("view") ? actor["view"].GetString()[0] : view;
        vel_x = actor.HasMember("vel_x") ? actor["vel_x"].GetInt() : vel_x;
        vel_y = actor.HasMember("vel_y") ? actor["vel_y"].GetInt() : vel_y;
        blocking = actor.HasMember("blocking") ? actor["blocking"].GetBool() : blocking;
        nearbyDialogue = actor.HasMember("nearby_dialogue") ? actor["nearby_dialogue"].GetString() : nearbyDialogue;
        contactDialogue = actor.HasMember("contact_dialogue") ? actor["contact_dialogue"].GetString() : contactDialogue;

        // Transform updates
        if (transform != nullptr)
        {
            transform->position.x = actor.HasMember("transform_position_x") ? actor["transform_position_x"].GetInt() : transform->position.x;
            transform->position.y = actor.HasMember("transform_position_y") ? actor["transform_position_y"].GetInt() : transform->position.y;

            transform->scale.x = actor.HasMember("transform_scale_x") ? actor["transform_scale_x"].GetFloat() : transform->scale.x;
            transform->scale.y = actor.HasMember("transform_scale_y") ? actor["transform_scale_y"].GetFloat() : transform->scale.y;

            transform->rotationDegrees = actor.HasMember("transform_rotation_degrees") ? actor["transform_rotation_degrees"].GetDouble() : transform->rotationDegrees;
        }
        else
        {
            int transformPositionX = actor.HasMember("transform_position_x") ? actor["transform_position_x"].GetInt() : 0;
            int transformPositionY = actor.HasMember("transform_position_y") ? actor["transform_position_y"].GetInt() : 0;

            int transformScaleX = actor.HasMember("transform_scale_x") ? actor["transform_scale_x"].GetFloat() : 1.0;
            int transformScaleY = actor.HasMember("transform_scale_y") ? actor["transform_scale_y"].GetFloat() : 1.0;

            int transformRotationDegrees = actor.HasMember("transform_rotation_degrees") ? actor["transform_rotation_degrees"].GetDouble() : 0.0;

            transform = new Transform(glm::ivec2(transformPositionX, transformPositionY), glm::vec2(transformScaleX, transformScaleY), transformRotationDegrees);
        }

        // Sprite Renderer updates
        if (spriteRenderer != nullptr)
        {
            spriteRenderer->viewImageName = actor.HasMember("view_image") ? actor["view_image"].GetString() : spriteRenderer->viewImageName;

            spriteRenderer->viewPivotOffset.x = actor.HasMember("view_pivot_offset_x") ? actor["view_pivot_offset_x"].GetDouble() : spriteRenderer->viewPivotOffset.x;
            spriteRenderer->viewPivotOffset.y = actor.HasMember("view_pivot_offset_y") ? actor["view_pivot_offset_y"].GetDouble() : spriteRenderer->viewPivotOffset.y;

            if (actor.HasMember("render_order"))
            {
                spriteRenderer->renderOrder = actor["render_order"].GetInt();
            }

            spriteRenderer->ChangeSprite(spriteRenderer->viewImageName, spriteRenderer->viewPivotOffset);
        }
        else
        {
            std::string viewImageName = actor.HasMember("view_image") ? actor["view_image"].GetString() : "";

            double viewPivotOffsetX = actor.HasMember("view_pivot_offset_x") ? actor["view_pivot_offset_x"].GetDouble() : -1.0;
            double viewPivotOffsetY = actor.HasMember("view_pivot_offset_y") ? actor["view_pivot_offset_y"].GetDouble() : -1.0;

            std::optional<int> renderOrder;
            if (actor.HasMember("render_order"))
            {
                renderOrder = actor["render_order"].GetInt();
            }
            else
            {
                renderOrder = std::nullopt;
            }

            spriteRenderer = new SpriteRenderer(viewImageName, glm::dvec2(viewPivotOffsetX, viewPivotOffsetY), renderOrder);
        }
        

        // Create the Entity object
        Entity* entity = new Entity(
            name, view, glm::ivec2(vel_x, vel_y), blocking, nearbyDialogue, contactDialogue, transform, spriteRenderer
        );

        // Add the entity to the entities vector
        entities.push_back(entity);
        entityRenderOrder.push_back(entity);

        if (name == "player")
        {
            playerEnity = entity;
        }

        entity->entityID = totalEntities;
        totalEntities++;

        glm::ivec2 position(transform->position.x, transform->position.y);

        // Check if the position already exists in the map
        locationOfEntitiesInScene[transform->position.x][transform->position.y].push_back(entity);

        if (transform->position.x > maxWidth)
        {
            maxWidth = transform->position.x;
        }

        if (transform->position.y > maxHeight)
        {
            maxHeight = transform->position.y;
        }

        ParseEntityDialogue(entity);
    }
}

void SceneDB::ParseEntityDialogue(Entity* entity)
{
    // set up entity dialogue type

    //nearby dialogue
    if (entity->nearbyDialogue != "")
    {
        //handle dialogue command type
        if (entity->nearbyDialogue.find("[health down]") != std::string::npos)
        {
            entity->nearbyDialogueType = HEALTHDOWN;
        }
        else if (entity->nearbyDialogue.find("[score up]") != std::string::npos)
        {
            entity->nearbyDialogueType = SCOREUP;
        }
        else if (entity->nearbyDialogue.find("[you win]") != std::string::npos)
        {
            entity->nearbyDialogueType = YOUWIN;
        }
        else if (entity->nearbyDialogue.find("[game over]") != std::string::npos)
        {
            entity->nearbyDialogueType = GAMEOVER;
        }

        // is scene change requested when this dialogue prints
        entity->nearbySceneToLoad = EngineUtils::ObtainWordAfterPhrase(entity->nearbyDialogue, "proceed to ");
    }

    //contact dialogue
    if (entity->contactDialogue != "")
    {
        //handle dialogue command type
        if (entity->contactDialogue.find("[health down]") != std::string::npos)
        {
            entity->contactDialogueType = HEALTHDOWN;
        }
        else if (entity->contactDialogue.find("[score up]") != std::string::npos)
        {
            entity->contactDialogueType = SCOREUP;
        }
        else if (entity->contactDialogue.find("[you win]") != std::string::npos)
        {
            entity->contactDialogueType = YOUWIN;
        }
        else if (entity->contactDialogue.find("[game over]") != std::string::npos)
        {
            entity->contactDialogueType = GAMEOVER;
        }

        // is scene change requested when this dialogue prints
        entity->contactSceneToLoad = EngineUtils::ObtainWordAfterPhrase(entity->contactDialogue, "proceed to ");
    }
}

Entity* SceneDB::GetPlayerEntity() {
    return playerEnity;
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

uint64_t SceneDB::GetSceneMaxHeight()
{
    return maxHeight;
}

uint64_t SceneDB::GetSceneMaxWidth()
{
    return maxWidth;
}

void SceneDB::ChangeEntityPosition(Entity* entity, glm::ivec2 newPosition)
{
    
    int entityIndexInLocationVec = IndexOfEntityAtPosition(entity);

    // remove from current position
    locationOfEntitiesInScene[entity->transform->position.x][entity->transform->position.y].erase(locationOfEntitiesInScene[entity->transform->position.x][entity->transform->position.y].begin() + entityIndexInLocationVec);

    // update actual position
    entity->transform->position = newPosition;

    // add to new bucket
    auto insertionPoint = std::lower_bound(locationOfEntitiesInScene[entity->transform->position.x][entity->transform->position.y].begin(), locationOfEntitiesInScene[entity->transform->position.x][entity->transform->position.y].end(), entity,
        [](Entity* a, Entity* b) { return a->entityID < b->entityID; });

    // Insert the entity at the correct position
    locationOfEntitiesInScene[entity->transform->position.x][entity->transform->position.y].insert(insertionPoint, entity);
}

int SceneDB::IndexOfEntityAtPosition(Entity* entity)
{
    // binary search bc should be sorted by entityID
    int low = 0;
    int high = locationOfEntitiesInScene[entity->transform->position.x][entity->transform->position.y].size() - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;

        if (locationOfEntitiesInScene[entity->transform->position.x][entity->transform->position.y][mid]->entityID == entity->entityID)
            return mid;

        // If x greater, ignore left half
        if (locationOfEntitiesInScene[entity->transform->position.x][entity->transform->position.y][mid]->entityID < entity->entityID)
            low = mid + 1;

        // If x is smaller, ignore right half
        else
            high = mid - 1;
    }

    //we messed up if we get here
    exit(0);

}



