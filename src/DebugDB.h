#ifndef DEBUGDB_H
#define DEBUGDB_H

#include <string>
#include <unordered_map>
#include <vector>

#include "lua.hpp"
#include "LuaBridge.h"
#include "LuaStateManager.h"

enum DebugType
{
	Log,
	LogError
};

struct DebugLog
{
	DebugLog(DebugType typeIn, std::string entityNameIn, std::string componentTypeIn, std::string messageIn)
		: type(typeIn), entityName(entityNameIn), componentType(componentTypeIn), message(messageIn) {};

	DebugType type;
	std::string entityName;
	std::string componentType;
	std::string message;
};

class DebugDB
{
public:
	static void Init();

	static void CppDebugLog(const std::string& message);
	static void CppDebugLogError(const std::string& message);

	static void AddStatement(DebugType type, std::string entityName, std::string componentType, std::string message);

	// EDITOR ONLY
	static void Reset(); // reset between simulations

	// just returning debugStatements every frame is super expensive, 
	//instead i will cache them off inside of the editor and mark debugStatementsUpdated dirty when the editor versions is outdated
	static bool DebugStatementsUpdate() { return debugStatementsUpdated; }
	static void MarkDebugStatementsDirty() { debugStatementsUpdated = true; }
	static std::vector<DebugLog*> GetDebugStatementsAndMarkClean() { debugStatementsUpdated = false; return debugStatements; }

	static bool saveDebugStatements;

private:
	static std::vector<DebugLog*> debugStatements;
	static bool debugStatementsUpdated;
};

#endif
