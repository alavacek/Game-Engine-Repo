#ifndef ERRORHANDLING_H
#define ERRORHANDLING_H

#include <filesystem>
#include <string>

#include "DebugDB.h"
#include "lua.hpp"
#include "LuaBridge.h"

class ErrorHandling
{
public:
	static void ReportError(const std::string& entityName, const luabridge::LuaException& e)
	{
		std::string errorMessage = e.what();

		// Normalize file paths across platforms
		std::replace(errorMessage.begin(), errorMessage.end(), '\\', '/');

		if (entityName != "")
		{
			// Display (with color codes)
			std::cout << "\033[31m" << entityName << " : " << errorMessage << "\033[0m" << "\n";
		}
		else
		{
			// Display (with color codes)
			std::cout << "\033[31m" << errorMessage << "\033[0m" << "\n";
		}

		DebugDB::AddStatement(DebugType::LogError, entityName, "", errorMessage);

	}

	static void ReportString(const std::string& error)
	{
		// Display (with color codes)
		std::cout << "\033[31m" << error << "\033[0m" << "\n";
		DebugDB::AddStatement(DebugType::LogError, "", "", error);
	}
};

#endif
