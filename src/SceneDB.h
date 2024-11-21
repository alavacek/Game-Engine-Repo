#ifndef SCENEDB_H
#define SCENEDB_H

#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "b2WorldDB.h"
#include "box2d/box2d.h"
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

	static void RequestLoadNewScene(const std::string& sceneName);
	static void LoadPendingScene();

	static void LoadScene(const std::string& sceneName);
	static void LoadEntitiesInScene(const std::string& sceneName);
	static std::string GetCurrentSceneName() { return currSceneName; }

	static uint64_t GetNumberOfEntitiesInScene();
	static Entity* GetEntityAtIndex(int index);

	static void Start();
	static void Update();
	static void LateUpdate();

	static Entity* Find(const std::string& name);
	static luabridge::LuaRef FindAll(const std::string& name);
	static Entity* Instantiate(const std::string& entityTemplateName);
	static void Destroy(Entity* entity);

	static void DontDestroy(Entity* entity);

	static bool pendingScene;

private:
	static std::string currSceneName;
	
	static std::string pendingSceneName;

	static std::vector<Entity*> entities;
	static std::vector<Entity*> entitiesToInstantiate;
	static std::vector<Entity*> entitiesToDestroy;


	 
	static int totalEntities; // NOTE: this is total enemies to exist, not total in scene
};

#endif

