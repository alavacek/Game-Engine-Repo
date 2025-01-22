#ifndef COMPONENTDB_H
#define COMPONENTDB_H

#include <filesystem>
#include <unordered_map>
#include <string>

#include "Component.h"
#include "lua.hpp"
#include "LuaBridge.h"
#include "LuaStateManager.h"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "Rigidbody.h"

class ComponentDB
{
public:
	static void LoadComponents();
	static void EstablishLuaInheritance(luabridge::LuaRef& instanceTable, luabridge::LuaRef& parentTable);

	static luabridge::LuaRef CreateInstanceTable(const std::string& componentName, const std::string& componentType);
	static luabridge::LuaRef CreateInstanceTableFromTemplate(const std::string& componentName, const std::string& componentType, luabridge::LuaRef templateTable);
	static Component* LoadComponentInstance(const rapidjson::Value& component, const std::string& componentName); // TODO: assess if passing string is expensive and necessary

	static std::unordered_map<std::string, Component*> components;
	static std::vector<std::string> componentsAlphabeticalOrder;

	static int numRuntimeAddedComponents;

	~ComponentDB();

private:
	static lua_State* luaState; // cache off

	
};

#endif
