#ifndef ENTITY_H
#define ENTITY_H

#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Component.h"
#include "ComponentDB.h"
#include "glm/glm.hpp"
#include "ImageDB.h"
#include "lua.hpp"
#include "LuaBridge.h"
#include "LuaStateManager.h"
#include "SDLHelper.h"

class Entity
{
public:
	std::unordered_map<std::string, Component*> components;
	std::vector<std::string> componentsKeysAlphabeticalOrder;

	std::string entityName;
	int entityID;

	bool wasDestroyed;
	bool destroyOnLoad;


	Entity(const std::string& entityName, const std::unordered_map<std::string, Component*>& components)
		: entityName(entityName), components(components) 
	{
		for (const auto& pair : components) 
		{
			componentsKeysAlphabeticalOrder.push_back(pair.first);

			(*pair.second->luaRef)["entity"] = this;
			pair.second->owningEntityName = entityName;
			
			// override to be true for now since logic in componentdb.h didnt seem to be doing anything
			(*pair.second->luaRef)["enabled"] = true;
		}

		// Sort the keys
		std::sort(componentsKeysAlphabeticalOrder.begin(), componentsKeysAlphabeticalOrder.end());

		wasDestroyed = false;
		destroyOnLoad = true;
	}

	void Start();
	void Update();
	void LateUpdate();
	void OnDestroy();

	std::string GetName() const { return entityName; }
	int GetID() const { return entityID;  }
	luabridge::LuaRef GetComponentByKey(const std::string& key);
	luabridge::LuaRef GetComponent(const std::string& typeName);
	luabridge::LuaRef GetComponents(const std::string& typeName);

	luabridge::LuaRef AddComponent(const std::string& typeName);
	void RemoveComponent(const std::string& typeName);

	~Entity();
private:
	int IndexOfComponentInAlphabeticalVector(const std::string& key);
	void PreLifeCycleFunctionComponentCleanUp();
	void PostLifeCycleFunctionComponentCleanUp();

	std::vector<std::string> keysOfComponentsToRemove;
	std::vector<std::string> keysOfNewlyAddedComponents;
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
	SDL_Texture* viewImageDamage;
	SDL_Texture* viewImageAttack;

	std::string viewImageName;
	std::optional<int> renderOrder;

	bool movementBounce;
	bool flipSpriteVertically = false;
	bool showBackImage = false;

	bool useDefaultPivotX;
	bool useDefaultPivotY;

	// Updated constructor to handle view_image and pivot offset logic
	SpriteRenderer(const std::string& viewImageName_in = "", glm::dvec2 pivot = { -1, -1 }, std::optional<int> renderOrderIn = std::nullopt, const std::string& viewImageBackName = "", 
		bool movementBounceIn = false, const std::string& viewImageDamageName = "", const std::string& viewImageAttackName = "")
		: viewImageName(viewImageName_in), viewPivotOffset(pivot), viewImage(nullptr), renderOrder(renderOrderIn), movementBounce(movementBounceIn)
	{
		if (!viewImageName.empty()) {
			viewImage = ImageDB::LoadImage(viewImageName);
		}

		if (!viewImageBackName.empty())
		{
			viewImageBack = ImageDB::LoadImage(viewImageBackName);
		}

		if (!viewImageDamageName.empty())
		{
			viewImageDamage = ImageDB::LoadImage(viewImageDamageName);
		}

		if (!viewImageAttackName.empty())
		{
			viewImageAttack = ImageDB::LoadImage(viewImageAttackName);
		}

		// used for templating 
		useDefaultPivotX = false;
		useDefaultPivotY = false;

		// Calculate default pivot offset if not provided
		if (viewImage && (viewPivotOffset.x == -1 || viewPivotOffset.y == -1))
		{
			int w = 0, h = 0;
			SDL_QueryTexture(viewImage, nullptr, nullptr, &w, &h);

			if (viewPivotOffset.x == -1)
			{
				viewPivotOffset.x = w * 0.5;
				useDefaultPivotX = true;
			}

			if (viewPivotOffset.y == -1)
			{
				viewPivotOffset.y = h * 0.5;
				useDefaultPivotY = true;
			}
		}
	}

	void ChangeSprite(const std::string& viewImageName_in = "", glm::dvec2 pivot = { -1, -1 });

	void RenderEntity(Entity* entity, SDL_Rect* cameraRect, int pixelsPerUnit, bool drawCollision);

	static bool IsEntityInView(SDL_Rect* entityDestinationRect);
};

#endif