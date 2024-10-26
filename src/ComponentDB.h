#ifndef COMPONENTDB_H
#define COMPONENTDB_H

#include <filesystem>
#include <unordered_map>
#include <string>

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

	static std::unordered_map<std::string, std::shared_ptr<luabridge::LuaRef>> componentFiles;
};

#endif
