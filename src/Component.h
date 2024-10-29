#ifndef COMPONENT_H
#define COMPONENT_H

#include <string>
#include "lua.hpp"
#include "LuaBridge.h"

class Component
{
public:
	std::shared_ptr<luabridge::LuaRef> luaRef;
	std::string type;

	Component(const std::shared_ptr<luabridge::LuaRef>& luaRef, const std::string type)
		: luaRef(luaRef), type(type) {}
};

#endif

