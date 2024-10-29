#ifndef SCENEDB_H
#define SCENEDB_H

#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "ComponentDB.h"
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
	void LoadEntitiesInScene(const std::string& sceneName);

	uint64_t GetNumberOfEntitiesInScene();
	Entity* GetEntityAtIndex(int index);

	uint64_t GetSceneMaxHeight();
	uint64_t GetSceneMaxWidth();

	void Start();
	void Update();
	void LateUpdate();

	static Entity* Find(const std::string& name);
	static luabridge::LuaRef FindAll(const std::string& name);

	std::vector<Entity*> entityRenderOrder;

private:
	std::string currSceneName;
	static std::vector<Entity*> entities;

	int totalEntities = 0;
	int maxHeight = 0;
	int maxWidth = 0;
	int collisionsSpatialMapSize;
	int triggersSpatialMapSize;
};

#endif

