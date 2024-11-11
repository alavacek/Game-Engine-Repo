#ifndef COMPONENTDB_H
#define COMPONENTDB_H

#include <filesystem>
#include <unordered_map>
#include <string>

#include "Component.h"
#include "lua.hpp"
#include "LuaBridge.h"
#include "LuaStateManager.h"

class ComponentDB
{
public:
	static void LoadComponents();
	static void EstablishInheritance(luabridge::LuaRef& instanceTable, luabridge::LuaRef& parentTable);

	static void CppDebugLog(const std::string& message);
	static void CppDebugLogError(const std::string& message);

	static void ReportError(const std::string& entityName, const luabridge::LuaException& e);

	static std::unordered_map<std::string, Component*> components;

	static int numRuntimeAddedComponents;

	~ComponentDB();
};

#endif
