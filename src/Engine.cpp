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

	// Entities class inside of Lua
	luabridge::getGlobalNamespace(LuaStateManager::GetLuaState())
		.beginClass<Entity>("Entity")
		.addFunction("GetName", &Entity::GetName)
		.addFunction("GetID", &Entity::GetID)
		.addFunction("GetComponentByKey", &Entity::GetComponentByKey)
		.addFunction("GetComponent", &Entity::GetComponent)
		.addFunction("GetComponents", &Entity::GetComponents)
		.endClass();

	// Entities that exist
	luabridge::getGlobalNamespace(LuaStateManager::GetLuaState())
		.beginNamespace("Entity")
		.addFunction("Find", &SceneDB::Find)
		.addFunction("FindAll", &SceneDB::FindAll)
		.endNamespace();

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

void Engine::Start()
{
	// Lua Loading
	LuaStateManager::Initialize();
	lua_State* luaState = LuaStateManager::GetLuaState();

	// set up renderer
	Renderer::RendererInit();
	renderer = Renderer::GetRenderer();

	ReadResources();

	std::string game_start_message = configDocument["game_start_message"].GetString();

	// set up audio
	AudioDB::InitAudio();

	// load intro screen images
	bool hasIntroImages = ImageDB::LoadIntroImages(configDocument);

	if (hasIntroImages)
	{
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

		TextDB::LoadIntroText(configDocument);
		AudioDB::LoadIntroAudio(configDocument);
	}
	else
	{
		state == INPROGRESS;
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

void Engine::Input()
{
	SDL_Event e;
	while (Helper::SDL_PollEvent498(&e)) // empty/consume all the events in the event queue
	{
		if (e.type == SDL_QUIT)
		{
			exit(1);
		}

		if (state == INTRO)
		{
			if ((e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) ||
				(e.type == SDL_KEYDOWN && (e.key.keysym.scancode == SDL_SCANCODE_SPACE || e.key.keysym.scancode == SDL_SCANCODE_RETURN)))
			{
				// Left-click to advance the image
				ImageDB::ProgressToNextIntroImage();
				TextDB::ProgressToNextIntroText();

				if (ImageDB::FinishedWithIntro() && TextDB::FinishedWithIntro())
				{
					state = INPROGRESS;
					AudioDB::EndIntroAudio();

					AudioDB::LoadGameplayAudio(configDocument);
				}
			}
		}
		else
		{
			Input::ProcessEvent(e);

		}

	}
}

void Engine::Update()
{
	if (isRunning)
	{
		currScene->Update();

		// Late Update
		Input::LateUpdate();
		currScene->LateUpdate();

		// sort based on y positon or based on render order
		std::sort(currScene->entityRenderOrder.begin(), currScene->entityRenderOrder.end(), Entity::CompareEntities);
	}
}

void Engine::Render()
{
	if (isRunning)
	{
		// TODO: May need to move this to begining of game loop
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);

		if (state == INTRO)
		{
			ImageDB::RenderCurrentIntroImage();
			TextDB::DrawCurrentIntroText(Renderer::GetResolution().y);
		}
		else if (state == INPROGRESS)
		{
			glm::vec2 centerPos = glm::vec2(0,0)/* = currScene->GetPlayerEntity() != nullptr ? currScene->GetPlayerEntity()->transform->position : glm::vec2(0, 0)*/;
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
		}
		else if (state == WON)
		{

		}
		else if (state == LOST)
		{

		}

		Helper::SDL_RenderPresent498(renderer);
	}
}

void Engine::EndGame()
{
	delete currScene;
}

Engine::~Engine()
{
	if (currScene != nullptr)
	{
		delete currScene;
	}
}

