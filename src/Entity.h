#ifndef ENTITY_H
#define ENTITY_H

#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Component.h"
#include "ComponentDB.h"
#include "DebugDB.h"
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
	void RemoveComponentByKey(const std::string& key);
	void RemoveComponentByKeyOutOfSimulation(const std::string& key);

	// EDITOR ONLY
	// okay so what are the odds 2 billion adds will happen and overflow
	unsigned int componentCounter = 0;

	~Entity();
private:
	int IndexOfComponentInAlphabeticalVector(const std::string& key);
	void PreLifeCycleFunctionComponentCleanUp();
	void PostLifeCycleFunctionComponentCleanUp();

	std::vector<std::string> keysOfComponentsToRemove;
	std::vector<std::string> keysOfNewlyAddedComponents;


};

#endif