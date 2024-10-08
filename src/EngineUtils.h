#ifndef ENGINEUTILS_H
#define ENGINEUTILS_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdio.h>

#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"

class EngineUtils
{
public:
	static void ReadJsonFile(const std::string& path, rapidjson::Document& out_document)
	{
		FILE* file_pointer = nullptr;
#ifdef _WIN32
		fopen_s(&file_pointer, path.c_str(), "rb");
#else
		file_pointer = fopen(path.c_str(), "rb");
#endif
		char buffer[65536];
		rapidjson::FileReadStream stream(file_pointer, buffer, sizeof(buffer));
		out_document.ParseStream(stream);
		std::fclose(file_pointer);

		if (out_document.HasParseError()) {
			rapidjson::ParseErrorCode errorCode = out_document.GetParseError();
			std::cout << "error parsing json at [" << path << "]" << std::endl;
			exit(0);
		}
	}

	static std::string ObtainWordAfterPhrase(const std::string& input, const std::string& phrase)
	{
		// Find the pos of phrase in the string
		size_t pos = input.find(phrase);

		// If phrase is not found, return an empty string
		if (pos == std::string::npos)
		{
			return "";
		}

		// Find the starting pos of the next word (skip spaces after the phrase)
		pos += phrase.length();
		while (pos < input.size() && std::isspace(input[pos]))
		{
			++pos;
		}

		// If we're at the end of the string, return an empty string
		if (pos == input.size())
		{
			return "";
		}

		// Find the end position of the word (until a space or the end of the string)
		size_t endPos = pos;
		while (endPos < input.size() && !std::isspace(input[endPos]))
		{
			++endPos;
		}

		// Extract and return the word
		return input.substr(pos, endPos - pos);
	}
};

#endif
