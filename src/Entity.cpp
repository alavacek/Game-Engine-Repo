#include "Entity.h"

void Entity::Start()
{
	for (const auto& componentKey : componentsKeysAlphabeticalOrder)
	{
		if (wasDestroyed)
		{
			return;
		}

		Component* component = components[componentKey];

		if (component->hasStart)
		{
			component->Start();
		}

		components[componentKey]->wasInstantiated = true;
	}

	// PostLifeCycleFunctionComponentCleanUp();

}

void Entity::Update()
{
	PreLifeCycleFunctionComponentCleanUp();

	for (const auto& componentKey : componentsKeysAlphabeticalOrder)
	{
		Component* component = components[componentKey];
		
		if (component->hasUpdate)
		{
			component->Update();
		}
	}

}

void Entity::LateUpdate()
{
	for (const auto& componentKey : componentsKeysAlphabeticalOrder)
	{
		if (wasDestroyed)
		{
			return;
		}

		Component* component = components[componentKey];
		
		if (component->hasLateUpdate)
		{
			component->LateUpdate();
		}
	}

	// added components get added to componentsKeysAlphabeticalOrder and removed components get removed
	PostLifeCycleFunctionComponentCleanUp();
}

void Entity::OnDestroy()
{
	for (const auto& componentKey : componentsKeysAlphabeticalOrder)
	{
		Component* component = components[componentKey];

		component->OnDestroy();
	}
}

luabridge::LuaRef Entity::GetComponentByKey(const std::string& key)
{
	if (components.find(key) != components.end() && !components.find(key)->second->wasRemoved)
	{
		return *(components[key]->luaRef);
	}
	else
	{
		return luabridge::LuaRef(LuaStateManager::GetLuaState());
	}
}

luabridge::LuaRef Entity::GetComponent(const std::string& typeName)
{
	std::vector<std::string>::iterator existingComponentsIt = componentsKeysAlphabeticalOrder.begin();
	std::vector<std::string>::iterator newlyAddedComponentsIt = keysOfNewlyAddedComponents.begin();
	while (existingComponentsIt != componentsKeysAlphabeticalOrder.end() || newlyAddedComponentsIt != keysOfNewlyAddedComponents.end())
	{
		Component* component;
		if (existingComponentsIt == componentsKeysAlphabeticalOrder.end())
		{
			component = components[*newlyAddedComponentsIt];
			newlyAddedComponentsIt++;
		}
		else if (newlyAddedComponentsIt == keysOfNewlyAddedComponents.end())
		{
			component = components[*existingComponentsIt];
			existingComponentsIt++;
		}
		else if (*existingComponentsIt <= *newlyAddedComponentsIt)
		{
			component = components[*existingComponentsIt];
			existingComponentsIt++;
		}
		else
		{
			component = components[*newlyAddedComponentsIt];
			newlyAddedComponentsIt++;
		}

		if (component->type == typeName && !component->wasRemoved)
		{
			return *(component->luaRef);
		}
	}

	return luabridge::LuaRef(LuaStateManager::GetLuaState());
}

luabridge::LuaRef Entity::GetComponents(const std::string& typeName)
{
	luabridge::LuaRef componentsTable = luabridge::newTable(LuaStateManager::GetLuaState());
	int index = 1;

	std::vector<std::string>::iterator existingComponentsIt = componentsKeysAlphabeticalOrder.begin();
	std::vector<std::string>::iterator newlyAddedComponentsIt = keysOfNewlyAddedComponents.begin();
	while (existingComponentsIt != componentsKeysAlphabeticalOrder.end() || newlyAddedComponentsIt != keysOfNewlyAddedComponents.end())
	{
		Component* component;
		if (existingComponentsIt == componentsKeysAlphabeticalOrder.end())
		{
			component = components[*newlyAddedComponentsIt];
			newlyAddedComponentsIt++;
		}
		else if (newlyAddedComponentsIt == keysOfNewlyAddedComponents.end())
		{
			component = components[*existingComponentsIt];
			existingComponentsIt++;
		}
		else if (*existingComponentsIt <= *newlyAddedComponentsIt)
		{
			component = components[*existingComponentsIt];
			existingComponentsIt++;
		}
		else
		{
			component = components[*newlyAddedComponentsIt];
			newlyAddedComponentsIt++;
		}

		if (component->type == typeName && !component->wasRemoved)
		{
			componentsTable[index] = (*(component->luaRef));
			index++;
		}
	}

	return componentsTable;
}

luabridge::LuaRef Entity::AddComponent(const std::string& typeName)
{
	if (ComponentDB::components.find(typeName) != ComponentDB::components.end())
	{
		std::string componentKey = "r" + std::to_string(ComponentDB::numRuntimeAddedComponents);
		ComponentDB::numRuntimeAddedComponents++;

		luabridge::LuaRef instanceTable = ComponentDB::CreateInstanceTable(componentKey, typeName);
		Component* parentComponent = ComponentDB::components[typeName];

		std::shared_ptr<luabridge::LuaRef> instanceTablePtr = std::make_shared<luabridge::LuaRef>(instanceTable);
		Component* addedComponent = new Component(instanceTablePtr, typeName, parentComponent->hasStart, parentComponent->hasUpdate, parentComponent->hasLateUpdate);
		addedComponent->owningEntityName = entityName;

		 // add ref for components table
		 components[componentKey] = addedComponent;
		 keysOfNewlyAddedComponents.push_back(componentKey);

		 // Can get expensive
		 std::sort(keysOfNewlyAddedComponents.begin(), keysOfNewlyAddedComponents.end());

		 return instanceTable;
	}
	else
	{
		std::string message = "failed to add component of type " + typeName;

		DebugDB::AddStatement(DebugType::LogError, entityName, "", message);
		//std::cout << message;
		//exit(0);
	}
}

void Entity::RemoveComponent(const std::string& typeName)
{
	// Set to nil by default
	luabridge::LuaRef componentLuaRef = luabridge::LuaRef(LuaStateManager::GetLuaState());

	for (const std::string& componentKey : componentsKeysAlphabeticalOrder)
	{
		Component* component = components[componentKey];
		// found component of type, no need to run logic if already planned to be removed
		if (component->type == typeName && !component->wasRemoved)
		{
			// NOTE: this logic will remove all components of type
			componentLuaRef = *(component->luaRef);
			
			// Set isEnabled to false
			luabridge::LuaRef isEnabled = (componentLuaRef)["enabled"];
			if (isEnabled.isBool() && isEnabled)
			{
				(componentLuaRef)["enabled"] = false;

				component->wasRemoved = true;

				keysOfComponentsToRemove.push_back(componentKey);

				// destroy right here to stop simulating
				if (component->type == "Rigidbody")
				{
					component->OnDestroy();
				}
			}
		}
	}

	// incase added to keysOfNewlyAddComponents, should now remove
	for (const std::string& componentKey : keysOfNewlyAddedComponents)
	{
		Component* component = components[componentKey];
		// found component of type, no need to run logic if already planned to be removed
		if (component->type == typeName && !component->wasRemoved)
		{
			// NOTE: this logic will remove all components of type
			componentLuaRef = *(component->luaRef);

			// Set isEnabled to false
			luabridge::LuaRef isEnabled = (componentLuaRef)["enabled"];
			if (isEnabled.isBool() && isEnabled)
			{
				(componentLuaRef)["enabled"] = false;

				component->wasRemoved = true;

				keysOfComponentsToRemove.push_back(componentKey);
			}
		}
	}
}

void Entity::RemoveComponentByKey(const std::string& key)
{
	// Set to nil by default
	luabridge::LuaRef componentLuaRef = luabridge::LuaRef(LuaStateManager::GetLuaState());

	for (const std::string& componentKey : componentsKeysAlphabeticalOrder)
	{
		Component* component = components[componentKey];
		// found component of type, no need to run logic if already planned to be removed
		if (componentKey == key && !component->wasRemoved)
		{
			// NOTE: this logic will remove all components of type
			componentLuaRef = *(component->luaRef);

			// Set isEnabled to false
			luabridge::LuaRef isEnabled = (componentLuaRef)["enabled"];
			if (isEnabled.isBool() && isEnabled)
			{
				(componentLuaRef)["enabled"] = false;

				component->wasRemoved = true;

				keysOfComponentsToRemove.push_back(componentKey);

				// destroy right here to stop simulating
				if (component->type == "Rigidbody")
				{
					component->OnDestroy();
				}
			}
		}
	}

	// incase added to keysOfNewlyAddComponents, should now remove
	for (const std::string& componentKey : keysOfNewlyAddedComponents)
	{
		Component* component = components[componentKey];
		// found component of type, no need to run logic if already planned to be removed
		if (componentKey == key && !component->wasRemoved)
		{
			// NOTE: this logic will remove all components of type
			componentLuaRef = *(component->luaRef);

			// Set isEnabled to false
			luabridge::LuaRef isEnabled = (componentLuaRef)["enabled"];
			if (isEnabled.isBool() && isEnabled)
			{
				(componentLuaRef)["enabled"] = false;

				component->wasRemoved = true;

				keysOfComponentsToRemove.push_back(componentKey);
			}
		}
	}
}


// WARNING
// THIS IS ONLY TO BE CALLED FROM THE EDITOR WHEN WE ARE NOT SIMULATING
void Entity::RemoveComponentByKeyOutOfSimulation(const std::string& key)
{
	Component* componentToRemove = components[key];

	// remove from alphabetical order vector
	int indexToRemove = IndexOfComponentInAlphabeticalVector(key);

	// If binary search failed and returned -1
	if (indexToRemove < 0)
	{
		//std::cout << "ERROR: Trying to remove a key that does not exist!";
		//exit(0);

		std::string message = "Trying to remove a key that does not exist!";

		DebugDB::AddStatement(DebugType::LogError, "", "", message);

		return;
	}

	componentsKeysAlphabeticalOrder.erase(componentsKeysAlphabeticalOrder.begin() + indexToRemove);

	// remove from components map
	components.erase(key);

	// remove component from existence
	delete(componentToRemove);
}

int Entity::IndexOfComponentInAlphabeticalVector(const std::string& key)
{
	// Binary search hurraw
	int l = 0;
	int r = componentsKeysAlphabeticalOrder.size() - 1;

	// Loop to implement Binary Search 
	while (l <= r) {

		// Calculatiing mid 
		int m = l + (r - l) / 2;

		// Some random value assigned 
		// as 0 belongs to index 
		int res = -1000;

		if (key == (componentsKeysAlphabeticalOrder[m]))
			res = 0;

		// Check if key is present at mid 
		if (res == 0)
			return m;

		// If key greater, ignore left half 
		if (key > (componentsKeysAlphabeticalOrder[m]))
			l = m + 1;

		// If key is smaller, ignore right half 
		else
			r = m - 1;
	}

	return -1;
}

void Entity::PreLifeCycleFunctionComponentCleanUp()
{
	if (keysOfNewlyAddedComponents.size() > 0)
	{
		// make copy of keysOfNewlyAddedComponents and clear incase start functions add more components
		std::vector<std::string> keysAddedComponents = keysOfNewlyAddedComponents;

		keysOfNewlyAddedComponents.clear();

		// sort for starts to be ran in correct order
		std::sort(keysAddedComponents.begin(), keysAddedComponents.end());

		// Add Components
		for (const std::string& keyOfComponent : keysAddedComponents)
		{
			// only if added component hasnt already been removed
			if (components.find(keyOfComponent) != components.end())
			{
				componentsKeysAlphabeticalOrder.push_back(keyOfComponent);

				(*components[keyOfComponent]->luaRef)["entity"] = this;

				if (components[keyOfComponent]->hasStart)
				{
					components[keyOfComponent]->Start();
				}

				components[keyOfComponent]->wasInstantiated = true;
			}
		}

		// TODO: explore if this is too expensive, should maybe insert at specific point instead
		std::sort(componentsKeysAlphabeticalOrder.begin(), componentsKeysAlphabeticalOrder.end());
	}
}

void Entity::PostLifeCycleFunctionComponentCleanUp()
{
	// Remove components
	for (const std::string& keyOfComponent : keysOfComponentsToRemove)
	{
		Component* componentToRemove = components[keyOfComponent];

		if (componentToRemove->wasInstantiated)
		{
			// remove from alphabetical order vector
			int indexToRemove = IndexOfComponentInAlphabeticalVector(keyOfComponent);

			// If binary search failed and returned -1
			if (indexToRemove < 0)
			{
				//std::cout << "ERROR: Trying to remove a key that does not exist!";
				//exit(0);

				std::string message = "Trying to remove a key that does not exist!";

				DebugDB::AddStatement(DebugType::LogError, "", "", message);

				continue;
			}

			componentToRemove->OnDestroy();

			componentsKeysAlphabeticalOrder.erase(componentsKeysAlphabeticalOrder.begin() + indexToRemove);
		}

		// remove from components map
		components.erase(keyOfComponent);
		
		// remove component from existence
		delete(componentToRemove);
	}

	keysOfComponentsToRemove.clear();

}

Entity::~Entity()
{
	for (const auto& pair : components) 
	{
		delete(pair.second);
	}
}