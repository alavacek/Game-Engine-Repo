#include "DebugDB.h"

std::vector<DebugLog*> DebugDB::debugStatements;
bool DebugDB::debugStatementsUpdated = false;
bool DebugDB::saveDebugStatements = false; // false by default, but if editor instance, will be set to true for debug panel to be rendered

void DebugDB::Init()
{    
    lua_State* luaState = LuaStateManager::GetLuaState();

    // Debug Namesapce
    luabridge::getGlobalNamespace(luaState)
        .beginNamespace("Debug")
        .addFunction("Log", &DebugDB::CppDebugLog)
        .addFunction("LogError", &DebugDB::CppDebugLogError)
        .endNamespace();
}

void DebugDB::CppDebugLog(const std::string& message)
{
    std::cout << message << std::endl;

    if (saveDebugStatements)
    {
        AddStatement(DebugType::Log, "", "", message);
    }
}

void DebugDB::CppDebugLogError(const std::string& message)
{
    std::cerr << message << std::endl;

    if (saveDebugStatements)
    {
        AddStatement(DebugType::LogError, "", "", message);
    }
}

void DebugDB::AddStatement(DebugType type, std::string entityName, std::string componentType, std::string message)
{
    if (saveDebugStatements)
    {
        debugStatementsUpdated = true;

        if (debugStatements.size() == debugStatements.capacity())
        {
            debugStatements.reserve(debugStatements.size() * 2);
        }

        DebugLog* log = new DebugLog(type, "", "", message);
        debugStatements.push_back(log);
    }
}

void DebugDB::Reset()
{
    for (DebugLog* statement : debugStatements)
    {
        delete(statement);
    }

    debugStatements.clear();

    // want the logs to stay after the simulation ends
    // debugStatementsUpdated = true;
}