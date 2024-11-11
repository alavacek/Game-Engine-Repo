#include "ComponentDB.h"

std::unordered_map<std::string, Component*> ComponentDB::components;
int ComponentDB::numRuntimeAddedComponents = 0;

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

				// all components should start as enabled, since these are templates for instances of components
				// we can just set these ones to enabled
				// NOTE: not sure this is actually doing something, had to try something else in instantiate function
				luaRef["enabled"] = true;

				std::shared_ptr<luabridge::LuaRef> luaRefPtr = std::make_shared<luabridge::LuaRef>(luaRef);
				
				bool hasStart = false;
				bool hasUpdate = false;
				bool hasLateUpdate = false;

				// has start
				luabridge::LuaRef onStartFunc = (luaRef)["OnStart"];
				if (onStartFunc.isFunction())
				{
					hasStart = true;
				}

				// has update
				luabridge::LuaRef onUpdateFunc = (luaRef)["OnUpdate"];
				if (onUpdateFunc.isFunction())
				{
					hasUpdate = true;
				}

				// has late update
				luabridge::LuaRef onLateUpdateFunc = (luaRef)["OnLateUpdate"];
				if (onLateUpdateFunc.isFunction())
				{
					hasLateUpdate = true;
				}

				Component* newComponentBase = new Component(luaRefPtr, fileNameWithoutExtension, hasStart, hasUpdate, hasLateUpdate);

				components[fileNameWithoutExtension] = newComponentBase;
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

void ComponentDB::ReportError(const std::string& entityName, const luabridge::LuaException& e)
{
	std::string errorMessage = e.what();

	// Normalize file paths across platforms
	std::replace(errorMessage.begin(), errorMessage.end(), '\\', '/');

	// Display (with color codes)
	std::cout << "\033[31m" << entityName << " : " << errorMessage << "\033[0m" << "\n";
}

ComponentDB::~ComponentDB()
{
	for (auto& component : components)
	{
		delete(component.second);
	}
}