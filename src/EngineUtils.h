#ifndef ENGINEUTILS_H
#define ENGINEUTILS_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdio.h>


#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>

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

	static void RemoveEntityFromJson(const std::string& jsonFilePath, const std::string& entityName) 
	{
		// Open the JSON file for reading
		FILE* file = nullptr;
		errno_t err = fopen_s(&file, jsonFilePath.c_str(), "rb");
		if (err != 0 || !file) {
			printf("Failed to open JSON file: %s\n", jsonFilePath.c_str());
			return;
		}

		// Read the JSON file into a buffer
		char readBuffer[65536];
		rapidjson::FileReadStream readStream(file, readBuffer, sizeof(readBuffer));
		rapidjson::Document document;
		document.ParseStream(readStream);
		fclose(file);

		// Check if the document contains an "entities" array
		if (!document.IsObject() || !document.HasMember("entities") || !document["entities"].IsArray()) 
		{
			printf("Invalid JSON format or missing 'entities' array.\n");
			return;
		}

		// Access the "entities" array
		rapidjson::Value& entities = document["entities"];
		for (rapidjson::SizeType i = 0; i < entities.Size(); ++i) 
		{
			const rapidjson::Value& entity = entities[i];

			// Check if the entity has a "name" field matching the one to remove
			if (entity.HasMember("name") && entity["name"].IsString() &&
				entityName == entity["name"].GetString()) 
			{
				// Remove the entity from the array
				entities.Erase(entities.Begin() + i);
				break;
			}
		}

		// Write the updated JSON back to the file
		err = fopen_s(&file, jsonFilePath.c_str(), "wb");
		if (err != 0 || !file) {
			printf("Failed to open JSON file for writing: %s\n", jsonFilePath.c_str());
			return;
		}

		rapidjson::FileWriteStream writeStream(file, readBuffer, sizeof(readBuffer));
		rapidjson::Writer<rapidjson::FileWriteStream> writer(writeStream);
		document.Accept(writer);
		fclose(file);

		printf("Entity '%s' removed from JSON file: %s\n", entityName.c_str(), jsonFilePath.c_str());
	}

	static void RemoveComponentFromJson(const std::string& jsonFilePath, const std::string& entityName, const std::string& componentKey)
	{
		// Open the JSON file for reading
		FILE* file = nullptr;
		errno_t err = fopen_s(&file, jsonFilePath.c_str(), "rb");
		if (err != 0 || !file) {
			printf("Failed to open JSON file: %s\n", jsonFilePath.c_str());
			return;
		}

		// Read the JSON file into a buffer
		char readBuffer[65536];
		rapidjson::FileReadStream readStream(file, readBuffer, sizeof(readBuffer));
		rapidjson::Document document;
		document.ParseStream(readStream);
		fclose(file);

		// Check if the document contains an "entities" array
		if (!document.IsObject() || !document.HasMember("entities") || !document["entities"].IsArray())
		{
			printf("Invalid JSON format or missing 'entities' array.\n");
			return;
		}

		// Access the "entities" array
		rapidjson::Value& entities = document["entities"];
		for (rapidjson::SizeType i = 0; i < entities.Size(); ++i)
		{
			rapidjson::Value& entity = entities[i];

			// Check if the entity has a "name" field matching the one to modify
			if (entity.HasMember("name") && entity["name"].IsString() &&
				entityName == entity["name"].GetString())
			{
				// Check if the entity has a "components" object
				if (entity.HasMember("components") && entity["components"].IsObject())
				{
					rapidjson::Value& components = entity["components"];

					// Check if the component to be removed exists
					if (components.HasMember(componentKey.c_str()))
					{
						// Remove the component
						components.RemoveMember(componentKey.c_str());
						printf("Component '%s' removed from entity '%s'.\n", componentKey.c_str(), entityName.c_str());
					}
					else
					{
						printf("Component '%s' not found in entity '%s'.\n", componentKey.c_str(), entityName.c_str());
					}
				}
				else
				{
					printf("Entity '%s' does not have a 'components' object.\n", entityName.c_str());
				}
				break;
			}
		}

		// Write the updated JSON back to the file
		err = fopen_s(&file, jsonFilePath.c_str(), "wb");
		if (err != 0 || !file) {
			printf("Failed to open JSON file for writing: %s\n", jsonFilePath.c_str());
			return;
		}

		rapidjson::FileWriteStream writeStream(file, readBuffer, sizeof(readBuffer));
		rapidjson::Writer<rapidjson::FileWriteStream> writer(writeStream);
		document.Accept(writer);
		fclose(file);

		printf("JSON file '%s' updated successfully.\n", jsonFilePath.c_str());
	}

	static void AddTemplateEntityToJson(const std::string& jsonFilePath, const std::string& templateName, const std::string& entityName)
	{
		// Open the JSON file for reading
		FILE* file = nullptr;
		errno_t err = fopen_s(&file, jsonFilePath.c_str(), "rb");
		if (err != 0 || !file) {
			printf("Failed to open JSON file: %s\n", jsonFilePath.c_str());
			return;
		}

		// Read the JSON file into a buffer
		char readBuffer[65536];
		rapidjson::FileReadStream readStream(file, readBuffer, sizeof(readBuffer));
		rapidjson::Document document;
		document.ParseStream(readStream);
		fclose(file);

		// Check if the document contains an "entities" array
		if (!document.IsObject() || !document.HasMember("entities") || !document["entities"].IsArray()) {
			printf("Invalid JSON format or missing 'entities' array.\n");
			return;
		}

		// Access the "entities" array
		rapidjson::Value& entities = document["entities"];

		// Create the new template entity
		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
		rapidjson::Value newEntity(rapidjson::kObjectType);

		// Add the "template" field
		newEntity.AddMember("template", rapidjson::Value(templateName.c_str(), allocator).Move(), allocator);

		// Add the "name" field
		newEntity.AddMember("name", rapidjson::Value(entityName.c_str(), allocator).Move(), allocator);

		// Add the new entity to the entities array
		entities.PushBack(newEntity, allocator);

		// Write the updated JSON back to the file
		err = fopen_s(&file, jsonFilePath.c_str(), "wb");
		if (err != 0 || !file) {
			printf("Failed to open JSON file for writing: %s\n", jsonFilePath.c_str());
			return;
		}

		rapidjson::FileWriteStream writeStream(file, readBuffer, sizeof(readBuffer));
		rapidjson::Writer<rapidjson::FileWriteStream> writer(writeStream);
		document.Accept(writer);
		fclose(file);

		printf("Template entity '%s' with name '%s' added to JSON file: %s\n", templateName.c_str(), entityName.c_str(), jsonFilePath.c_str());
	}

	static void AddComponentToEntityInJson(const std::string& jsonFilePath, const std::string& entityName, const std::string& componentKey, const std::string& componentType)
	{
		// Open the JSON file for reading
		FILE* file = nullptr;
		errno_t err = fopen_s(&file, jsonFilePath.c_str(), "rb");
		if (err != 0 || !file) {
			printf("Failed to open JSON file: %s\n", jsonFilePath.c_str());
			return;
		}

		// Read the JSON file into a buffer
		char readBuffer[65536];
		rapidjson::FileReadStream readStream(file, readBuffer, sizeof(readBuffer));
		rapidjson::Document document;
		document.ParseStream(readStream);
		fclose(file);

		// Check if the document contains an "entities" array
		if (!document.IsObject() || !document.HasMember("entities") || !document["entities"].IsArray()) {
			printf("Invalid JSON format or missing 'entities' array.\n");
			return;
		}

		// Access the "entities" array
		rapidjson::Value& entities = document["entities"];

		for (rapidjson::SizeType i = 0; i < entities.Size(); ++i) {
			rapidjson::Value& entity = entities[i];

			// Check if the entity matches the given name
			if (entity.HasMember("name") && entity["name"].IsString() && entityName == entity["name"].GetString()) {
				// Check if "components" field exists, if not create it
				if (!entity.HasMember("components") || !entity["components"].IsObject()) {
					entity.AddMember("components", rapidjson::Value(rapidjson::kObjectType), document.GetAllocator());
				}

				// Access the "components" field
				rapidjson::Value& components = entity["components"];

				// Check if the component already exists; if not, add it
				if (!components.HasMember(componentKey.c_str())) {
					rapidjson::Value componentObject(rapidjson::kObjectType); // Create an empty object for the component
					componentObject.AddMember(
						"type",
						rapidjson::Value(componentType.c_str(), document.GetAllocator()).Move(),
						document.GetAllocator()
					);

					// Add the component to the "components" field
					components.AddMember(
						rapidjson::Value(componentKey.c_str(), document.GetAllocator()).Move(),
						componentObject,
						document.GetAllocator()
					);
				}

				break; // Stop searching after modifying the matching entity
			}
		}

		// Write the updated JSON back to the file
		err = fopen_s(&file, jsonFilePath.c_str(), "wb");
		if (err != 0 || !file) {
			printf("Failed to open JSON file for writing: %s\n", jsonFilePath.c_str());
			return;
		}

		rapidjson::FileWriteStream writeStream(file, readBuffer, sizeof(readBuffer));
		rapidjson::Writer<rapidjson::FileWriteStream> writer(writeStream);
		document.Accept(writer);
		fclose(file);

		printf("Component '%s' added to entity '%s' in JSON file: %s\n", componentKey.c_str(), entityName.c_str(), jsonFilePath.c_str());
	}

	static bool isNumber(const std::string& s)
	{
		std::string::const_iterator it = s.begin();
		while (it != s.end() && std::isdigit(*it)) ++it;
		return !s.empty() && it == s.end();
	}

};

#endif
