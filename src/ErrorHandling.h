#ifndef ERRORHANDLING_H
#define ERRORHANDLING_H

#include <filesystem>
#include <string>

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

		// Display (with color codes)
		std::cout << "\033[31m" << entityName << " : " << errorMessage << "\033[0m" << "\n";
	}

	static void ReportString(const std::string& error)
	{
		// Display (with color codes)
		std::cout << "\033[31m" << error << "\033[0m" << "\n";
	}
};

#endif
