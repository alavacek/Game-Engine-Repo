#ifndef COMPONENT_H
#define COMPONENT_H

#include <string>

#include "ErrorHandling.h"
#include "lua.hpp"
#include "LuaBridge.h"


class Component
{
public:
	std::shared_ptr<luabridge::LuaRef> luaRef;

	std::string type;
	std::string owningEntityName;

	bool hasStart;
	bool hasUpdate;
	bool hasLateUpdate;

	bool enabled = true;
	bool wasRemoved = false;
	bool wasInstantiated = false;

	Component() : hasStart(false), hasUpdate(false), hasLateUpdate(false) {};

	Component(const std::shared_ptr<luabridge::LuaRef>& luaRef, const std::string type,
		bool hasStart, bool hasUpdate, bool hasLateUpdate)
		: luaRef(luaRef), type(type), hasStart(hasStart), hasUpdate(hasUpdate), hasLateUpdate(hasLateUpdate) {}

	// Copy Constructor
	// Not handling luaRef
	Component(const Component& other)
	{
		type = other.type;
		owningEntityName = other.owningEntityName;

		hasStart = other.hasStart;
		hasUpdate = other.hasUpdate;
		hasLateUpdate = other.hasLateUpdate;

		enabled = other.enabled;
		wasRemoved = other.wasRemoved;
		wasInstantiated = other.wasInstantiated;
	}

	virtual void Start();
	virtual void Update();
	virtual void LateUpdate();
	virtual void OnDestroy();

	// TODO: Deconstructor??
};

#endif

