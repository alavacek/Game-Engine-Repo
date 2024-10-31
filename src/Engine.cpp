#include "Engine.h"

int Engine::pixelsPerUnit = 100;

void Engine::GameLoop()
{
	Start();

	while (isRunning)
	{
		Input();

		Update();

		Render();

		SDL_Delay(16); //time for each frame
	}
}

void Engine::Start()
{
	// Lua Loading
	LuaStateManager::Initialize();
	lua_State* luaState = LuaStateManager::GetLuaState();

	// set up renderer
	Renderer::RendererInit();
	renderer = Renderer::GetRenderer();

	// set up Input
	Input::Init();

	LuaClassAndNamespaceSetup();

	ReadResources();

	// set up audio
	AudioDB::InitAudio();

	// set up text database
	if (configDocument.HasMember("font") && configDocument["font"].IsString())
	{
		TextDB::TextDB_Init(configDocument["font"].GetString());
	}
	else
	{
		std::cout << "error: text render failed. No font configured";
		exit(0);
	}

	// set up camera
	glm::ivec2 resolution = Renderer::GetResolution();
	double zoomFactor = Renderer::GetZoomFactor();

	cameraRect.w = static_cast<int>(std::round(resolution.x / zoomFactor));
	cameraRect.h = static_cast<int>(std::round(resolution.y / zoomFactor));

	glm::vec2 centerPos = glm::vec2(0,0)/* = currScene->GetPlayerEntity() != nullptr ? currScene->GetPlayerEntity()->transform->position : glm::vec2(0, 0)*/;

	cameraRect.x = static_cast<int>(std::round((centerPos.x * pixelsPerUnit) - (cameraRect.w / 2)));
	cameraRect.y = static_cast<int>(std::round((centerPos.y * pixelsPerUnit) - (cameraRect.h / 2)));


	// Entity Start
	currScene->Start();
}

void Engine::ReadResources()
{
	// check for resource folder
	if (!std::filesystem::exists("resources"))
	{
		std::cout << "error: resources/ missing";
		exit(0);
	}
	else if (!std::filesystem::exists("resources/game.config"))
	{
		std::cout << "error: resources/game.config missing";
		exit(0);
	}

	configPath = "resources/game.config";
	EngineUtils::ReadJsonFile(configPath, configDocument);

	// rendering config
	std::string renderingConfig = "resources/rendering.config";

	// Determine what components exist in resources/component_types
	ComponentDB::LoadComponents();

	// template config
	TemplateDB::LoadTemplates();

	// scene config
	if (configDocument.HasMember("initial_scene") && configDocument["initial_scene"].IsString())
	{
		std::string sceneName = configDocument["initial_scene"].GetString();

		currScene = new SceneDB();
		currScene->LoadScene(sceneName);
	}
	else
	{
		// no initial scene specified
		std::cout << "error: initial_scene unspecified";
		exit(0);
	}
}

void Engine::LuaClassAndNamespaceSetup()
{
	lua_State* luaState = LuaStateManager::GetLuaState();
	// Entities Class inside of Lua
	luabridge::getGlobalNamespace(luaState)
		.beginClass<Entity>("Entity")
		.addFunction("GetName", &Entity::GetName)
		.addFunction("GetID", &Entity::GetID)
		.addFunction("GetComponentByKey", &Entity::GetComponentByKey)
		.addFunction("GetComponent", &Entity::GetComponent)
		.addFunction("GetComponents", &Entity::GetComponents)
		.endClass();

	// Entities Namespace inside of Lua
	luabridge::getGlobalNamespace(luaState)
		.beginNamespace("Entity")
		.addFunction("Find", &SceneDB::Find)
		.addFunction("FindAll", &SceneDB::FindAll)
		.endNamespace();

	// Application Namespace inside of Lua
	luabridge::getGlobalNamespace(luaState)
		.beginNamespace("Application")
		.addFunction("GetFrame", &Helper::GetFrameNumber)
		.addFunction("Quit", &Engine::Quit)
		.addFunction("Sleep", &Engine::Sleep)
		.addFunction("OpenURL", &Engine::OpenURL)
		.endNamespace();

	// Vec2 Class
	luabridge::getGlobalNamespace(luaState)
		.beginClass<glm::vec2>("vec2")
		.addProperty("x", &glm::vec2::x)
		.addProperty("y", &glm::vec2::y)
		.endClass();

	// Input Name Space inside of Lua
	luabridge::getGlobalNamespace(luaState)
		.beginNamespace("Input")
		.addFunction("GetKey", &Input::GetKey)
		.addFunction("GetKeyDown", &Input::GetKeyDown)
		.addFunction("GetKeyUp", &Input::GetKeyUp)
		.addFunction("GetMousePosition", &Input::GetMousePosition)
		.addFunction("GetMouseButton", &Input::GetMouseButton)
		.addFunction("GetMouseButtonDown", &Input::GetMouseButtonDown)
		.addFunction("GetMouseButtonUp", &Input::GetMouseButtonUp)
		.addFunction("GetMouseScrollDelta", &Input::GetMouseScrollDelta)
		.endNamespace();
}


void Engine::Input()
{
	SDL_Event e;
	while (Helper::SDL_PollEvent498(&e)) // empty/consume all the events in the event queue
	{
		if (e.type == SDL_QUIT)
		{
			exit(1);
		}

		Input::ProcessEvent(e);

	}
}

void Engine::Update()
{
	if (isRunning)
	{
		currScene->Update();

		// Late Update
		currScene->LateUpdate();
		Input::LateUpdate();

		// sort based on y position or based on render order
		std::sort(currScene->entityRenderOrder.begin(), currScene->entityRenderOrder.end(), Entity::CompareEntities);
	}
}

void Engine::Render()
{
	if (isRunning)
	{
		// TODO: May need to move this to beginning of game loop
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);

		glm::vec2 centerPos = glm::vec2(0, 0)/* = currScene->GetPlayerEntity() != nullptr ? currScene->GetPlayerEntity()->transform->position : glm::vec2(0, 0)*/;
		glm::ivec2 resolution = Renderer::GetResolution();

		double zoomFactor = Renderer::GetZoomFactor();

		cameraRect.x = glm::mix(cameraRect.x, static_cast<int>(std::round((centerPos.x * pixelsPerUnit) - (cameraRect.w / 2))), Renderer::GetCameraEaseFactor());
		cameraRect.y = glm::mix(cameraRect.y, static_cast<int>(std::round((centerPos.y * pixelsPerUnit) - (cameraRect.h / 2))), Renderer::GetCameraEaseFactor());

		// render visible map
		SDL_RenderSetScale(renderer, zoomFactor, zoomFactor);

		for (Entity* entity : currScene->entityRenderOrder)
		{
			//entity->->RenderEntity(entity, &c//eraRect, pixelsPerUnit, entity->velocity != glm::vec2(0,0), debugShowCollisions);
		}

		// possibly move to update?
		if (pendingScene != "")
		{
			delete currScene; // TODO: iss this appropriate or should I just load new scene?

			currScene = new SceneDB();
			currScene->LoadScene(pendingScene);
			pendingScene = "";

			Render();
		}

		Helper::SDL_RenderPresent498(renderer);
	}
}

void Engine::EndGame()
{
	delete currScene;
}

// Lua Functions
void Engine::Quit()
{
	exit(0);
}

void Engine::Sleep(int milliseconds)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void Engine::OpenURL(std::string url)
{
#if defined(_WIN32) || defined(_WIN64) // Windows
	std::string command = "start " + url;

#elif defined(__APPLE__) // OSX
	std::string command = "open " + url;

#elif defined(__linux__) // Linus
	std::string command = "xdg-open " + url;

#else
	std::cout << "Platform not supported\n";
	exit(0);

#endif
	std::system(command.c_str());
	
}

Engine::~Engine()
{
	if (currScene != nullptr)
	{
		delete currScene;
	}
}

