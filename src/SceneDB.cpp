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
    entityRenderOrder.clear();

    int numberOfColliders = 0;
    int numberOfTriggers = 0;

    // Loop through each actor in the JSON array
    for (rapidjson::SizeType i = 0; i < actors.Size(); i++)
    {
        const rapidjson::Value& actor = actors[i];

        // Set base values
        std::string name = "";
        float vel_x = 0;
        float vel_y = 0;
        std::string nearbyDialogue = "";
        std::string contactDialogue = "";

        Transform* transform = nullptr;
        SpriteRenderer* spriteRenderer = nullptr;
        Collider* collider = nullptr;
        TriggerCollider* triggerCollider = nullptr;

        if (actor.HasMember("template"))
        {
            std::string templateName = actor["template"].GetString();
            Template* entityTemplate = TemplateDB::FindEntity(templateName);
            if (entityTemplate != nullptr)
            {
                // Extract values from the JSON
                name = entityTemplate->entityName;
                vel_x = entityTemplate->velocity.x;
                vel_y = entityTemplate->velocity.y;
                nearbyDialogue = entityTemplate->nearbyDialogue;
                contactDialogue = entityTemplate->contactDialogue;

                transform = new Transform(entityTemplate->transform->position, entityTemplate->transform->scale, entityTemplate->transform->rotationDegrees);

                spriteRenderer = new SpriteRenderer(entityTemplate->spriteRenderer->viewImageName, entityTemplate->spriteRenderer->viewPivotOffset, entityTemplate->spriteRenderer->renderOrder);
                spriteRenderer->viewImageBack = entityTemplate->spriteRenderer->viewImageBack;
                spriteRenderer->viewImageDamage = entityTemplate->spriteRenderer->viewImageDamage;
                spriteRenderer->viewImageAttack = entityTemplate->spriteRenderer->viewImageAttack;
                // determine if new pivots should be calculated or not
                spriteRenderer->useDefaultPivotX = entityTemplate->spriteRenderer->useDefaultPivotX;
                spriteRenderer->useDefaultPivotY = entityTemplate->spriteRenderer->useDefaultPivotY;

                collider = entityTemplate->collider ? new Collider(entityTemplate->collider->colliderWidth, entityTemplate->collider->colliderHeight) : nullptr;
                triggerCollider = entityTemplate->triggerCollider ? new TriggerCollider(entityTemplate->triggerCollider->colliderWidth, entityTemplate->triggerCollider->colliderHeight) : nullptr;
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
        vel_x = actor.HasMember("vel_x") ? actor["vel_x"].GetFloat() : vel_x;
        vel_y = actor.HasMember("vel_y") ? actor["vel_y"].GetFloat() : vel_y;
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

            transform = new Transform(glm::vec2(transformPositionX, transformPositionY), glm::vec2(transformScaleX, transformScaleY), transformRotationDegrees);
        }

        // Sprite Renderer updates
        if (spriteRenderer != nullptr)
        {
            spriteRenderer->viewImageName = actor.HasMember("view_image") ? actor["view_image"].GetString() : spriteRenderer->viewImageName;

            std::string viewImageBackName = actor.HasMember("view_image_back") ? actor["view_image_back"].GetString() : "";
            if (viewImageBackName != "")
            {
                spriteRenderer->viewImageBack = ImageDB::LoadImage(viewImageBackName);
            }

            std::string viewImageDamageName = actor.HasMember("view_image_damage") ? actor["view_image_damage"].GetString() : "";
            if (viewImageDamageName != "")
            {
                spriteRenderer->viewImageDamage = ImageDB::LoadImage(viewImageDamageName);
            }

            std::string viewImageAttackName = actor.HasMember("view_image_attack") ? actor["view_image_attack"].GetString() : "";
            if (viewImageAttackName != "")
            {
                spriteRenderer->viewImageAttack = ImageDB::LoadImage(viewImageAttackName);
            }

            spriteRenderer->viewPivotOffset.x = actor.HasMember("view_pivot_offset_x") ? actor["view_pivot_offset_x"].GetDouble() : spriteRenderer->viewPivotOffset.x;
            spriteRenderer->viewPivotOffset.y = actor.HasMember("view_pivot_offset_y") ? actor["view_pivot_offset_y"].GetDouble() : spriteRenderer->viewPivotOffset.y;

            if (actor.HasMember("render_order"))
            {
                spriteRenderer->renderOrder = actor["render_order"].GetInt();
            }

            spriteRenderer->movementBounce = actor.HasMember("movement_bounce_enabled") ? actor["movement_bounce_enabled"].GetBool() : spriteRenderer->movementBounce;

            spriteRenderer->ChangeSprite(spriteRenderer->viewImageName, spriteRenderer->viewPivotOffset);
        }
        else
        {
            std::string viewImageName = actor.HasMember("view_image") ? actor["view_image"].GetString() : "";
            std::string viewImageBackName = actor.HasMember("view_image_back") ? actor["view_image_back"].GetString() : "";
            std::string viewImageDamageName = actor.HasMember("view_image_damage") ? actor["view_image_damage"].GetString() : "";
            std::string viewImageAttackName = actor.HasMember("view_image_attack") ? actor["view_image_attack"].GetString() : "";

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

            bool movementBounce = actor.HasMember("movement_bounce_enabled") ? actor["movement_bounce_enabled"].GetBool() : false;

            spriteRenderer = new SpriteRenderer(viewImageName, glm::dvec2(viewPivotOffsetX, viewPivotOffsetY), renderOrder, viewImageBackName, 
                movementBounce, viewImageDamageName, viewImageAttackName);
        }

        // Collider updates
        if (collider != nullptr)
        {
            collider->colliderWidth = actor.HasMember("box_collider_width") ? actor["box_collider_width"].GetFloat() : collider->colliderWidth;
            collider->colliderHeight = actor.HasMember("box_collider_height") ? actor["box_collider_height"].GetFloat() : collider->colliderHeight;
        }
        else
        {

            float colliderWidth = actor.HasMember("box_collider_width") ? actor["box_collider_width"].GetFloat() : 0;
            float colliderHeight = actor.HasMember("box_collider_height") ? actor["box_collider_height"].GetFloat() : 0;

            if (colliderWidth != 0 && colliderHeight != 0)
            {
                collider = new Collider(colliderWidth, colliderHeight);
            }
        }

        // Trigger Collider updates
        if (triggerCollider != nullptr)
        {
            triggerCollider->colliderWidth = actor.HasMember("box_trigger_width") ? actor["box_trigger_width"].GetFloat() : triggerCollider->colliderWidth;
            triggerCollider->colliderHeight = actor.HasMember("box_trigger_height") ? actor["box_trigger_height"].GetFloat() : triggerCollider->colliderHeight;
        }
        else
        {

            float triggerColliderWidth = actor.HasMember("box_trigger_width") ? actor["box_trigger_width"].GetFloat() : 0;
            float triggerColliderHeight = actor.HasMember("box_trigger_height") ? actor["box_trigger_height"].GetFloat() : 0;

            if (triggerColliderWidth != 0 && triggerColliderHeight != 0)
            {
                triggerCollider = new TriggerCollider(triggerColliderWidth, triggerColliderHeight);
            }
        }
        
        // Create the Entity object
        Entity* entity = new Entity(
            name, glm::vec2(vel_x, vel_y), nearbyDialogue, contactDialogue, transform, spriteRenderer, collider, triggerCollider
        );

        // Add the entity to the entities vector
        entityRenderOrder.push_back(entity);
        entities.push_back(entity);

        if (name == "player")
        {
            playerEnity = entity;
        }

        entity->entityID = totalEntities;
        totalEntities++;

        ParseEntityDialogue(entity);

        // Running average of collider size
        if (collider != nullptr && collider->colliderWidth != 0 && collider->colliderHeight != 0)
        {
            collisionsSpatialMapSize = ((collisionsSpatialMapSize * (numberOfColliders * 2)) + (collider->colliderWidth + collider->colliderHeight)) / ((numberOfColliders * 2) + 2);
            numberOfColliders++;
        }

        // Running average of trigger collider size
        if (triggerCollider != nullptr && triggerCollider->colliderWidth != 0 && triggerCollider->colliderHeight != 0)
        {
            triggersSpatialMapSize = ((triggersSpatialMapSize * (numberOfTriggers * 2)) + (triggerCollider->colliderWidth + triggerCollider->colliderHeight)) / ((numberOfTriggers * 2) + 2);
            numberOfTriggers++;
        }

    }

    // Fill spatial maps
    for (auto entity : entities)
    {
        Transform* transform = entity->transform;
        Collider* collider = entity->collider;
        TriggerCollider* triggerCollider = entity->triggerCollider;
        glm::vec2 position(transform->position.x, transform->position.y);

        // fill spatial map
        if (collider != nullptr)
        {
            float colliderHalfWidth = (entity->collider->colliderWidth / 2) * entity->transform->scale.x;
            float colliderHalfHeight = (entity->collider->colliderHeight / 2) * entity->transform->scale.y;

            glm::ivec2 upperLeftHashedPosition = HashPositionToBucket(glm::vec2(position.x - colliderHalfWidth, position.y - colliderHalfHeight));
            glm::ivec2 lowerRightHashedPosition = HashPositionToBucket(glm::vec2(position.x + colliderHalfWidth, position.y + colliderHalfHeight));

            for (int y = upperLeftHashedPosition.y; y <= lowerRightHashedPosition.y; y = y + collisionsSpatialMapSize)
            {
                for (int x = upperLeftHashedPosition.x; x <= lowerRightHashedPosition.x; x = x + collisionsSpatialMapSize)
                {
                    glm::ivec2 hashedPosition(x, y);

                    // Check if the position already exists in the map
                    collisionsOfEntitiesInScene[hashedPosition.x][hashedPosition.y].push_back(entity);

                    if (transform->position.x > maxWidth)
                    {
                        maxWidth = transform->position.x;
                    }

                    if (transform->position.y > maxHeight)
                    {
                        maxHeight = transform->position.y;
                    }
                }
            }
        }

        // fill spatial map
        if (triggerCollider != nullptr)
        {
            float colliderHalfWidth = (entity->triggerCollider->colliderWidth / 2) * entity->transform->scale.x;
            float colliderHalfHeight = (entity->triggerCollider->colliderHeight / 2) * entity->transform->scale.y;

            glm::ivec2 upperLeftHashedPosition = HashTriggerPositionToBucket(glm::vec2(position.x - colliderHalfWidth, position.y - colliderHalfHeight));
            glm::ivec2 lowerRightHashedPosition = HashTriggerPositionToBucket(glm::vec2(position.x + colliderHalfWidth, position.y + colliderHalfHeight));

            for (int y = upperLeftHashedPosition.y; y <= lowerRightHashedPosition.y; y = y + triggersSpatialMapSize)
            {
                for (int x = upperLeftHashedPosition.x; x <= lowerRightHashedPosition.x; x = x + triggersSpatialMapSize)
                {
                    glm::vec2 hashedPosition(x, y);

                    // Check if the position already exists in the map
                    triggersOfEntitiesInScene[hashedPosition.x][hashedPosition.y].push_back(entity);

                    if (transform->position.x > maxWidth)
                    {
                        maxWidth = transform->position.x;
                    }

                    if (transform->position.y > maxHeight)
                    {
                        maxHeight = transform->position.y;
                    }
                }
            }
        }
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

bool SceneDB::CanMoveEntityToPosition(Entity* entity, glm::vec2 proposedPosition)
{
    if (entity->collider != nullptr && entity->collider->colliderWidth != 0 && entity->collider->colliderHeight != 0)
    {
        float colliderHalfWidth = (entity->collider->colliderWidth / 2) * entity->transform->scale.x;
        float colliderHalfHeight = (entity->collider->colliderHeight / 2) * entity->transform->scale.y;

        glm::vec2 proposedUpperLeftPosition = glm::vec2(proposedPosition.x - colliderHalfWidth, proposedPosition.y - colliderHalfHeight);
        glm::vec2 proposedLowerRightPosition = glm::vec2(glm::vec2(proposedPosition.x + colliderHalfWidth, proposedPosition.y + colliderHalfHeight));

        glm::ivec2 proposedUpperLeftHashedPosition = HashPositionToBucket(proposedUpperLeftPosition);
        glm::ivec2 proposedLowerRightHashedPosition = HashPositionToBucket(proposedLowerRightPosition);

        for (int y = proposedUpperLeftHashedPosition.y - collisionsSpatialMapSize; y <= proposedLowerRightHashedPosition.y + collisionsSpatialMapSize; y = y + collisionsSpatialMapSize)
        {
            for (int x = proposedUpperLeftHashedPosition.x - collisionsSpatialMapSize; x <= proposedLowerRightHashedPosition.x + collisionsSpatialMapSize; x = x + collisionsSpatialMapSize)
            {
                for (int i = 0; i < collisionsOfEntitiesInScene[x][y].size(); i++)
                {
                    Entity* otherEntity = collisionsOfEntitiesInScene[x][y][i];
                    // does the current entity block?
                    if (otherEntity != entity && otherEntity->collider != nullptr
                        && otherEntity->collider->colliderWidth != 0 && otherEntity->collider->colliderHeight != 0)
                    {
                        float otherColliderHalfWidth = (otherEntity->collider->colliderWidth / 2) * otherEntity->transform->scale.x;
                        float otherColliderHalfHeight = (otherEntity->collider->colliderHeight / 2) * otherEntity->transform->scale.y;

                        glm::vec2 otherUpperLeftPosition = glm::vec2(otherEntity->transform->position.x - otherColliderHalfWidth, otherEntity->transform->position.y - otherColliderHalfHeight);
                        glm::vec2 otherLowerRightPosition = glm::vec2(otherEntity->transform->position.x + otherColliderHalfWidth, otherEntity->transform->position.y + otherColliderHalfHeight);

                        if (proposedLowerRightPosition.x > otherUpperLeftPosition.x && proposedUpperLeftPosition.x < otherLowerRightPosition.x)
                        {
                            if (proposedLowerRightPosition.y > otherUpperLeftPosition.y && proposedUpperLeftPosition.y < otherLowerRightPosition.y)
                            {
                                entity->collider->collidingEntitiesThisFrame.insert(otherEntity);
                            }
                        }
                    }
                }
            }
        }

        if (entity->collider->collidingEntitiesThisFrame.size() > 0)
        {
            return false;
        }
    }

    return true;
}

bool SceneDB::DetermineEntityTriggerCollisions(Entity* entity)
{
    if (entity->triggerCollider != nullptr && entity->triggerCollider->colliderWidth != 0 && entity->triggerCollider->colliderHeight != 0)
    {
        float colliderHalfWidth = (entity->triggerCollider->colliderWidth / 2) * entity->transform->scale.x;
        float colliderHalfHeight = (entity->triggerCollider->colliderHeight / 2) * entity->transform->scale.y;

        glm::vec2 proposedUpperLeftPosition = glm::vec2(entity->transform->position.x - colliderHalfWidth, entity->transform->position.y - colliderHalfHeight);
        glm::vec2 proposedLowerRightPosition = glm::vec2(glm::vec2(entity->transform->position.x + colliderHalfWidth, entity->transform->position.y + colliderHalfHeight));

        glm::ivec2 proposedUpperLeftHashedPosition = HashTriggerPositionToBucket(proposedUpperLeftPosition);
        glm::ivec2 proposedLowerRightHashedPosition = HashTriggerPositionToBucket(proposedLowerRightPosition);

        for (int y = proposedUpperLeftHashedPosition.y - triggersSpatialMapSize; y <= proposedLowerRightHashedPosition.y + triggersSpatialMapSize; y = y + triggersSpatialMapSize)
        {
            for (int x = proposedUpperLeftHashedPosition.x - triggersSpatialMapSize; x <= proposedLowerRightHashedPosition.x + triggersSpatialMapSize; x = x + triggersSpatialMapSize)
            {
                for (int i = 0; i < triggersOfEntitiesInScene[x][y].size(); i++)
                {
                    Entity* otherEntity = triggersOfEntitiesInScene[x][y][i];
                    // does the current entity block?
                    if (otherEntity != entity && otherEntity->collider != nullptr
                        && otherEntity->collider->colliderWidth != 0 && otherEntity->collider->colliderHeight != 0)
                    {
                        float otherColliderHalfWidth = (otherEntity->triggerCollider->colliderWidth / 2) * otherEntity->transform->scale.x;
                        float otherColliderHalfHeight = (otherEntity->triggerCollider->colliderHeight / 2) * otherEntity->transform->scale.y;

                        glm::vec2 otherUpperLeftPosition = glm::vec2(otherEntity->transform->position.x - otherColliderHalfWidth, otherEntity->transform->position.y - otherColliderHalfHeight);
                        glm::vec2 otherLowerRightPosition = glm::vec2(otherEntity->transform->position.x + otherColliderHalfWidth, otherEntity->transform->position.y + otherColliderHalfHeight);

                        if (proposedLowerRightPosition.x > otherUpperLeftPosition.x && proposedUpperLeftPosition.x < otherLowerRightPosition.x)
                        {
                            if (proposedLowerRightPosition.y > otherUpperLeftPosition.y && proposedUpperLeftPosition.y < otherLowerRightPosition.y)
                            {
                                entity->triggerCollider->triggeringEntitiesThisFrame.insert(otherEntity);
                            }
                        }
                    }
                }
            }
        }

        if (entity->triggerCollider->triggeringEntitiesThisFrame.size() > 0)
        {
            return false;
        }

        return true;
    }
    // No triggers if trigger collider is invalid
    else
    {
        return false;
    }
    
}

void SceneDB::ChangeEntityPosition(Entity* entity, glm::vec2 newPosition)
{    
    // additional collision logic
    if (entity->collider != nullptr && entity->collider->colliderWidth != 0 && entity->collider->colliderHeight != 0)
    {
        // NOTE: this implementation will NOT allow for dynamic collider changes!
        float colliderHalfWidth = (entity->collider->colliderWidth / 2) * entity->transform->scale.x;
        float colliderHalfHeight = (entity->collider->colliderHeight / 2) * entity->transform->scale.y;
        glm::vec2 entityPosition = glm::vec2(entity->transform->position.x, entity->transform->position.y);

        // current position info
        glm::ivec2 oldUpperLeftHashedPosition = HashPositionToBucket(glm::vec2(entityPosition.x - colliderHalfWidth, entityPosition.y - colliderHalfHeight));
        glm::ivec2 oldLowerRightHashedPosition = HashPositionToBucket(glm::vec2(entityPosition.x + colliderHalfWidth, entityPosition.y + colliderHalfHeight));

        // new position info
        glm::ivec2 newUpperLeftHashedPosition = HashPositionToBucket(glm::vec2(newPosition.x - colliderHalfWidth, newPosition.y - colliderHalfHeight));
        glm::ivec2 newLowerRightHashedPosition = HashPositionToBucket(glm::vec2(newPosition.x + colliderHalfWidth, newPosition.y + colliderHalfHeight));

        if (oldUpperLeftHashedPosition != newUpperLeftHashedPosition && oldLowerRightHashedPosition != newLowerRightHashedPosition)
        {
            for (int y = oldUpperLeftHashedPosition.y; y <= oldLowerRightHashedPosition.y; y = y + collisionsSpatialMapSize) {
                for (int x = oldUpperLeftHashedPosition.x; x <= oldLowerRightHashedPosition.x; x = x + collisionsSpatialMapSize) {
                    // Only remove from the old area if it's not part of the new area
                    if (x < newUpperLeftHashedPosition.x || x > newLowerRightHashedPosition.x || y < newUpperLeftHashedPosition.y || y > newLowerRightHashedPosition.y)
                    {
                        int entityIndexInLocationVec = IndexOfEntityAtCollisionPosition(entity, glm::ivec2(x, y));

                        // remove from current position
                        collisionsOfEntitiesInScene[x][y].erase(collisionsOfEntitiesInScene[x][y].begin() + entityIndexInLocationVec);
                    }
                }
            }

            // Add entity to new buckets it now occupies
            for (int y = newUpperLeftHashedPosition.y; y <= newLowerRightHashedPosition.y;  y = y + collisionsSpatialMapSize) {
                for (int x = newUpperLeftHashedPosition.x; x <= newLowerRightHashedPosition.x; x = x + collisionsSpatialMapSize) {
                    // Only add to the new area if it's not part of the old area
                    if (x < oldUpperLeftHashedPosition.x || x > oldLowerRightHashedPosition.x || y < oldUpperLeftHashedPosition.y || y > oldLowerRightHashedPosition.y)
                    {
                        // add to new bucket
                        auto& newBucketVec = collisionsOfEntitiesInScene[x][y];
                        auto insertionPoint = std::lower_bound(newBucketVec.begin(), newBucketVec.end(), entity,
                            [](Entity* a, Entity* b) { return a->entityID < b->entityID; });

                        // Insert the entity at the correct position in the new bucket
                        newBucketVec.insert(insertionPoint, entity);
                    }
                }
            }
        }
    }

    // additional trigger logic
    if (entity->triggerCollider != nullptr && entity->triggerCollider->colliderWidth != 0 && entity->triggerCollider->colliderHeight != 0)
    {
        // NOTE: this implementation will NOT allow for dynamic collider changes!
        float colliderHalfWidth = (entity->triggerCollider->colliderWidth / 2) * entity->transform->scale.x;
        float colliderHalfHeight = (entity->triggerCollider->colliderHeight / 2) * entity->transform->scale.y;
        glm::vec2 entityPosition = glm::vec2(entity->transform->position.x, entity->transform->position.y);

        // current position info
        glm::ivec2 oldUpperLeftHashedPosition = HashTriggerPositionToBucket(glm::vec2(entityPosition.x - colliderHalfWidth, entityPosition.y - colliderHalfHeight));
        glm::ivec2 oldLowerRightHashedPosition = HashTriggerPositionToBucket(glm::vec2(entityPosition.x + colliderHalfWidth, entityPosition.y + colliderHalfHeight));

        // new position info
        glm::ivec2 newUpperLeftHashedPosition = HashTriggerPositionToBucket(glm::vec2(newPosition.x - colliderHalfWidth, newPosition.y - colliderHalfHeight));
        glm::ivec2 newLowerRightHashedPosition = HashTriggerPositionToBucket(glm::vec2(newPosition.x + colliderHalfWidth, newPosition.y + colliderHalfHeight));

        if (oldUpperLeftHashedPosition != newUpperLeftHashedPosition && oldLowerRightHashedPosition != newLowerRightHashedPosition)
        {
            for (int y = oldUpperLeftHashedPosition.y; y <= oldLowerRightHashedPosition.y; y = y + triggersSpatialMapSize) {
                for (int x = oldUpperLeftHashedPosition.x; x <= oldLowerRightHashedPosition.x; x = x + triggersSpatialMapSize) {
                    // Only remove from the old area if it's not part of the new area
                    if (x < newUpperLeftHashedPosition.x || x > newLowerRightHashedPosition.x || y < newUpperLeftHashedPosition.y || y > newLowerRightHashedPosition.y)
                    {
                        int entityIndexInLocationVec = IndexOfEntityAtTriggernPosition(entity, glm::ivec2(x, y));

                        // remove from current position
                        triggersOfEntitiesInScene[x][y].erase(triggersOfEntitiesInScene[x][y].begin() + entityIndexInLocationVec);
                    }
                }
            }

            // Add entity to new buckets it now occupies
            for (int y = newUpperLeftHashedPosition.y; y <= newLowerRightHashedPosition.y; y = y + triggersSpatialMapSize) {
                for (int x = newUpperLeftHashedPosition.x; x <= newLowerRightHashedPosition.x; x = x + triggersSpatialMapSize) {
                    // Only add to the new area if it's not part of the old area
                    if (x < oldUpperLeftHashedPosition.x || x > oldLowerRightHashedPosition.x || y < oldUpperLeftHashedPosition.y || y > oldLowerRightHashedPosition.y)
                    {
                        // add to new bucket
                        auto& newBucketVec = triggersOfEntitiesInScene[x][y];
                        auto insertionPoint = std::lower_bound(newBucketVec.begin(), newBucketVec.end(), entity,
                            [](Entity* a, Entity* b) { return a->entityID < b->entityID; });

                        // Insert the entity at the correct position in the new bucket
                        newBucketVec.insert(insertionPoint, entity);
                    }
                }
            }
        }
    }

    // update actual position
    entity->transform->position = newPosition;
    return;
}

int SceneDB::IndexOfEntityAtCollisionPosition(Entity* entity, glm::ivec2 hashedPosition)
{
    // binary search bc should be sorted by entityID
    int low = 0;
    int high = collisionsOfEntitiesInScene[hashedPosition.x][hashedPosition.y].size() - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;

        if (collisionsOfEntitiesInScene[hashedPosition.x][hashedPosition.y][mid]->entityID == entity->entityID)
            return mid;

        // If x greater, ignore left half
        if (collisionsOfEntitiesInScene[hashedPosition.x][hashedPosition.y][mid]->entityID < entity->entityID)
            low = mid + 1;

        // If x is smaller, ignore right half
        else
            high = mid - 1;
    }

    //we messed up if we get here
    exit(0);
}

int SceneDB::IndexOfEntityAtTriggernPosition(Entity* entity, glm::ivec2 hashedPosition)
{
    // binary search bc should be sorted by entityID
    int low = 0;
    int high = triggersOfEntitiesInScene[hashedPosition.x][hashedPosition.y].size() - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;

        if (triggersOfEntitiesInScene[hashedPosition.x][hashedPosition.y][mid]->entityID == entity->entityID)
            return mid;

        // If x greater, ignore left half
        if (triggersOfEntitiesInScene[hashedPosition.x][hashedPosition.y][mid]->entityID < entity->entityID)
            low = mid + 1;

        // If x is smaller, ignore right half
        else
            high = mid - 1;
    }

    //we messed up if we get here
    exit(0);
}

glm::ivec2 SceneDB::HashPositionToBucket(glm::vec2 pos)
{
    return glm::ivec2(static_cast<int>((std::floor(pos.x / collisionsSpatialMapSize)) * collisionsSpatialMapSize), static_cast<int>((std::floor(pos.y / collisionsSpatialMapSize)) * collisionsSpatialMapSize));
}

glm::ivec2 SceneDB::HashTriggerPositionToBucket(glm::vec2 pos)
{
    return glm::ivec2(static_cast<int>((std::floor(pos.x / triggersSpatialMapSize)) * triggersSpatialMapSize), static_cast<int>((std::floor(pos.y / triggersSpatialMapSize)) * triggersSpatialMapSize));
}



