#ifndef ENGINE_H
#define ENGINE_H

#include <algorithm>
#include <cmath>
#include <cstdlib> 
#include <filesystem>
#include <iostream>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <vector>

#include "AudioDB.h"
#include "ComponentDB.h"
#include "EngineUtils.h"
#include "Entity.h"
#include "glm/glm.hpp"
#include "ImageDB.h"
#include "Input.h"
#include "lua.hpp"
#include "LuaBridge.h"
#include "LuaStateManager.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include "Renderer.h"
#include "SceneDB.h"
#include "SDL2/SDL.h"
#include "SDLHelper.h"
#include "TemplateDB.h"
#include "TextDB.h"

class Engine
{
public:
	void GameLoop();
	void ReadResources();
	void LuaClassAndNamespaceSetup();

	void Start();

	void Input();

	void Update();

	void Render();

	void EndGame();

	static void Quit();
	static void Sleep(int milliseconds);
	static void OpenURL(std::string url);

	~Engine();

private:
	SDL_Rect cameraRect;
	std::vector<std::pair<uint64_t, std::string>> entityDialoguesToPrint;

	std::string pendingScene;

	std::string configPath;
	rapidjson::Document configDocument;

	SceneDB* currScene;
	SDL_Renderer* renderer;

	static int pixelsPerUnit;
	int frameSinceDamageTaken;

	int framesOfDamageIndicator = 30;
	int framesOfAttackIndicator = 30;

	bool isRunning = true;	
	bool inHealthCooldown = false;
	bool debugShowCollisions = true;
};

#endif

