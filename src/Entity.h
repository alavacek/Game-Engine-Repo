#ifndef ENTITY_H
#define ENTITY_H

#include <optional>
#include <string>
#include <vector>

#include "Component.h"
#include "glm/glm.hpp"
#include "ImageDB.h"
#include "SDLHelper.h"

enum DialogueType
{
	NONE,
	HEALTHDOWN,
	SCOREUP,
	YOUWIN,
	GAMEOVER,
};

class Transform;
class SpriteRenderer;
class Collider;


class Entity
{
public:
	static bool CompareEntities(const Entity* a, const Entity* b);

	std::string entityName;
	
	Transform* transform;
	SpriteRenderer* spriteRenderer;
	Collider* collider;

	glm::vec2 velocity;
	std::string nearbyDialogue;
	std::string contactDialogue;

	bool hasIncreasedScore = false;
	bool hasTriggeredNearbyDialogue = false;
	DialogueType nearbyDialogueType = NONE;
	DialogueType contactDialogueType = NONE;
	std::string nearbySceneToLoad; // will be set to "" if not relevant
	std::string contactSceneToLoad; // will be set to "" if not relevant

	std::vector<Component> components;

	int entityID;

	Entity(std::string entityName, glm::vec2 initialVelocity,
		std::string nearbyDialogue, std::string contactDialogue,
		Transform* transformIn, SpriteRenderer* spriteIn, Collider* colliderIn)
		: entityName(entityName), transform(transformIn),
		spriteRenderer(spriteIn), collider(colliderIn), velocity(initialVelocity),
		nearbyDialogue(nearbyDialogue), contactDialogue(contactDialogue) {}

	Entity() {}

	~Entity();
private:
	
};

class Transform
{
public:
	glm::vec2 position;
	glm::vec2 scale;

	float rotationDegrees;

	Transform(glm::ivec2 pos = { 0, 0 }, glm::ivec2 scl = { 1.0, 1.0 }, float rotation = 0.0f)
		: position(pos), scale(scl), rotationDegrees(rotation) {}
};

class SpriteRenderer
{
public:
	glm::dvec2 viewPivotOffset;
	SDL_Texture* viewImage;
	SDL_Texture* viewImageBack;
	std::string viewImageName;
	std::optional<int> renderOrder;

	bool movementBounce;
	bool flipSpriteVertically = false;
	bool showBackImage = false;

	// Updated constructor to handle view_image and pivot offset logic
	SpriteRenderer(const std::string& viewImageName_in = "", glm::dvec2 pivot = { -1, -1 }, std::optional<int> renderOrderIn = std::nullopt, const std::string& viewImageBackName = "", bool movementBounceIn = false)
		: viewImageName(viewImageName_in), viewPivotOffset(pivot), viewImage(nullptr), renderOrder(renderOrderIn), movementBounce(movementBounceIn)
	{
		if (!viewImageName.empty()) {
			viewImage = ImageDB::LoadImage(viewImageName);
		}

		if (!viewImageBackName.empty())
		{
			viewImageBack = ImageDB::LoadImage(viewImageBackName);
		}

		// Calculate default pivot offset if not provided
		if (viewImage && (viewPivotOffset.x == -1 || viewPivotOffset.y == -1))
		{
			int w = 0, h = 0;
			SDL_QueryTexture(viewImage, nullptr, nullptr, &w, &h);

			if (viewPivotOffset.x == -1)
			{
				viewPivotOffset.x = w * 0.5;
			}

			if (viewPivotOffset.y == -1)
			{
				viewPivotOffset.y = h * 0.5;
			}
		}
	}

	void ChangeSprite(const std::string& viewImageName_in = "", glm::dvec2 pivot = { -1, -1 });

	void RenderEntity(Entity* entity, SDL_Rect* cameraRect, int pixelsPerUnit, bool bounce = false);
};

class Collider
{
public:
	float colliderWidth;
	float colliderHeight;

	Collider(float width, float height)
		: colliderWidth(width), colliderHeight(height) {}
};

#endif