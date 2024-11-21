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
	for (const auto& componentKey : componentsKeysAlphabeticalOrder)
	{
		Component* component = components[componentKey];
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

	for (const auto& componentKey : componentsKeysAlphabeticalOrder)
	{
		Component* component = components[componentKey];
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

		luabridge::LuaRef instanceTable = ComponentDB::CreateInstanceTable(componentKey, typeName);
		Component* parentComponent = ComponentDB::components[typeName];

		std::shared_ptr<luabridge::LuaRef> instanceTablePtr = std::make_shared<luabridge::LuaRef>(instanceTable);
		Component* addedComponent = new Component(instanceTablePtr, typeName, parentComponent->hasStart, parentComponent->hasUpdate, parentComponent->hasLateUpdate);

		 // add ref for components table
		 components[componentKey] = addedComponent;
		 keysOfNewlyAddedComponents.push_back(componentKey);

		 return instanceTable;
	}
	else
	{
		std::cout << "error: failed to add component of type " << typeName;
		exit(0);
	}
}

void Entity::RemoveComponent(const std::string& typeName)
{
	// Set to nil by default
	luabridge::LuaRef componentLuaRef = luabridge::LuaRef(LuaStateManager::GetLuaState());

	for (const auto& componentKey : componentsKeysAlphabeticalOrder)
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

	// incase added to keysOfNewlyAddComponents, should now remove
	for (const auto& componentKey : keysOfNewlyAddedComponents)
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
		for (const auto& keyOfComponent : keysAddedComponents)
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
	for (const auto& keyOfComponent : keysOfComponentsToRemove)
	{
		Component* componentToRemove = components[keyOfComponent];

		if (components[keyOfComponent]->wasInstantiated)
		{
			// remove from alphabetical order vector
			int indexToRemove = IndexOfComponentInAlphabeticalVector(keyOfComponent);

			// If binary search failed and returned -1
			if (indexToRemove < 0)
			{
				std::cout << "ERROR: Trying to remove a key that does not exist!";
				exit(0);
			}

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
	//for (const auto& pair : components) 
	//{
	//	delete pair.second;
	//}
}


// OLD C++ HARD CODED COMPONENTS

void SpriteRenderer::ChangeSprite(const std::string& viewImageName_in, glm::dvec2 pivot)
{
	viewImageName = viewImageName_in;

	if (!viewImageName.empty()) {
		viewImage = ImageDB::LoadImage(viewImageName);
	}

	// Calculate default pivot offset if not provided
	// Later is used when enity's sprite is different from template
	if (viewImage && (viewPivotOffset.x == -1 || viewPivotOffset.y == -1) || (useDefaultPivotX || useDefaultPivotY))
	{
		int w = 0, h = 0;
		SDL_QueryTexture(viewImage, nullptr, nullptr, &w, &h);

		if (viewPivotOffset.x == -1 || useDefaultPivotX)
		{
			viewPivotOffset.x = w * 0.5;
		}

		if (viewPivotOffset.y == -1 || useDefaultPivotY)
		{
			viewPivotOffset.y = h * 0.5;
		}
	}
}

void SpriteRenderer::RenderEntity(Entity* entity, SDL_Rect* cameraRect, int pixelsPerUnit, bool drawCollision)
{

}

bool SpriteRenderer::IsEntityInView(SDL_Rect* entityDestinationRect)
{
	if ((entityDestinationRect->x + entityDestinationRect->w < 0) ||
		(entityDestinationRect->y + entityDestinationRect->h < 0) || 
		(entityDestinationRect->x > (Renderer::GetResolution().x / Renderer::GetZoomFactor())) || 
		(entityDestinationRect->y > (Renderer::GetResolution().y / Renderer::GetZoomFactor())))
	{
		return false;
	}
	
	return true;
}
