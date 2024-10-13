#ifndef SCENEDB_H
#define SCENEDB_H

#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "EngineUtils.h"
#include "Entity.h"
#include "glm/glm.hpp"
#include "Template.h"
#include "TemplateDB.h"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"

class SceneDB
{
public:
	~SceneDB();

	void LoadScene(const std::string& sceneName);
	void ParseEntityDialogue(Entity* entity);

	Entity* GetPlayerEntity();

	uint64_t GetNumberOfEntitiesInScene();
	Entity* GetEntityAtIndex(int index);

	uint64_t GetSceneMaxHeight();
	uint64_t GetSceneMaxWidth();

	void ChangeEntityPosition(Entity* entity, glm::vec2 newPosition);
	int IndexOfEntityAtPosition(Entity* entity);
	glm::ivec2 HashPositionToBucket(glm::vec2 pos);

	std::unordered_map<uint64_t, std::unordered_map<uint64_t, std::vector<Entity*>>> locationOfEntitiesInScene;
	std::vector<Entity*> entityRenderOrder;

private:
	std::string currSceneName;
	std::vector<Entity*> entities;

	Entity* playerEnity;
	int totalEntities = 0;
	int maxHeight = 0;
	int maxWidth = 0;
};

#endif

