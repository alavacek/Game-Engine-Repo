#include "Entity.h"

bool Entity::CompareEntities(const Entity* a, const Entity* b)
{
	// As a last resort, compare by entityID (to maintain consistent order for equal y positions)
	return a->entityID < b->entityID;
}

void Entity::Start()
{
	for (const auto& componentKey : componentsKeysAlphabeticalOrder)
	{
		std::shared_ptr<luabridge::LuaRef> luaRefPtr = components[componentKey]->luaRef; // Get the LuaRef pointer
		luabridge::LuaRef luaRef = *luaRefPtr;

		if (luaRefPtr && luaRefPtr->isTable())
		{
			try
			{
				luabridge::LuaRef onStartFunc = (luaRef)["OnStart"];
				if (onStartFunc.isFunction())
				{
					onStartFunc(luaRef);
				}
			}
			catch (const luabridge::LuaException& e)
			{
				ComponentDB::ReportError(entityName, e);
			}

		}
	}
}

void Entity::Update()
{
	for (const auto& componentKey : componentsKeysAlphabeticalOrder)
	{
		std::shared_ptr<luabridge::LuaRef> luaRefPtr = components[componentKey]->luaRef; // Get the LuaRef pointer
		luabridge::LuaRef luaRef = *luaRefPtr;

		if (luaRefPtr && luaRefPtr->isTable())
		{
			try
			{
				luabridge::LuaRef onStartFunc = (luaRef)["OnUpdate"];
				if (onStartFunc.isFunction())
				{
					onStartFunc(luaRef);
				}
			}
			catch (const luabridge::LuaException& e)
			{
				ComponentDB::ReportError(entityName, e);
			}
		}
	}

}

void Entity::LateUpdate()
{
	for (const auto& componentKey : componentsKeysAlphabeticalOrder)
	{
		std::shared_ptr<luabridge::LuaRef> luaRefPtr = components[componentKey]->luaRef; // Get the LuaRef pointer
		luabridge::LuaRef luaRef = *luaRefPtr;

		if (luaRefPtr && luaRefPtr->isTable())
		{
			try
			{
				luabridge::LuaRef onStartFunc = (luaRef)["OnLateUpdate"];
				if (onStartFunc.isFunction())
				{
					onStartFunc(luaRef);
				}
			}
			catch (const luabridge::LuaException& e)
			{
				ComponentDB::ReportError(entityName, e);
			}
		}
	}
}

luabridge::LuaRef Entity::GetComponentByKey(const std::string& key)
{
	if (components.find(key) != components.end())
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
		if (component->type == typeName)
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
		if (component->type == typeName)
		{
			componentsTable[index] = (*(component->luaRef));
			index++;
		}
	}

	return componentsTable;
}

Entity::~Entity()
{
	for (const auto& pair : components) 
	{
		delete pair.second;
	}
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
