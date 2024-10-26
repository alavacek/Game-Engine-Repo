#include "ComponentDB.h"

std::unordered_map<std::string, std::shared_ptr<luabridge::LuaRef>> ComponentDB::componentFiles;

void ComponentDB::LoadComponents()
{
	std::string componentDirectoryPath = "resources/component_types";
	lua_State* luaState = LuaStateManager::GetLuaState();

	if (std::filesystem::exists(componentDirectoryPath))
	{
		for (const auto& entry : std::filesystem::directory_iterator(componentDirectoryPath)) {
			if (entry.is_regular_file() && entry.path().extension() == ".lua") 
			{
				std::string fileNameWithoutExtension = entry.path().stem().string();			

				std::string filePath = entry.path().string();
				std::replace(filePath.begin(), filePath.end(), '\\', '/');

				if (luaL_dofile(luaState, filePath.c_str()) != LUA_OK)
				{
					std::cout << "problem with lua file " << fileNameWithoutExtension;
					exit(0);
				}

				luabridge::LuaRef luaRef = luabridge::getGlobal(luaState, fileNameWithoutExtension.c_str());
				std::shared_ptr<luabridge::LuaRef> luaRefPtr = std::make_shared<luabridge::LuaRef>(luaRef);

				componentFiles[fileNameWithoutExtension] = luaRefPtr;
			}
		}
	}

	luabridge::getGlobalNamespace(luaState)
		.beginNamespace("Debug")
		.addFunction("Log", &ComponentDB::CppDebugLog)
		.addFunction("LogError", &ComponentDB::CppDebugLogError)
		.endNamespace();
}

void ComponentDB::EstablishInheritance(luabridge::LuaRef& instanceTable, luabridge::LuaRef& parentTable)
{
	lua_State* luaState = LuaStateManager::GetLuaState();

	// create metatable to establish inheritance
	luabridge::LuaRef newMetatable = luabridge::newTable(luaState);
	newMetatable["__index"] = parentTable;

	// must use raw lua C-API to perform a "setmetable"
	instanceTable.push(luaState);
	newMetatable.push(luaState);
	lua_setmetatable(luaState, -2);
	lua_pop(luaState, 1);
}

void ComponentDB::CppDebugLog(const std::string& message)
{
	std::cout << message << std::endl;
}

void ComponentDB::CppDebugLogError(const std::string& message)
{
	std::cerr << message << std::endl;
}