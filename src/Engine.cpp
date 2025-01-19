#include "Engine.h"

void Engine::GameLoop()
{
	Start();

	while (isRunning)
	{
		Frame();
	}
}

void Engine::Frame()
{
	// input is handled inside of editor 
	if (!editorInstance)
	{
		Input();
	}

	Update();

	Render();

	if (!editorInstance)
	{
		SDL_Delay(16); //time for each frame
	}
}

void Engine::Start()
{
	// editor should read resources earlier on in order to fill hierarchy
	if (!editorInstance)
	{
		ReadResources();
	}

	isRunning = true;

	// set up renderer
	Renderer::RendererInit();
	renderer = Renderer::GetRenderer();

	// set up Input
	Input::Init();

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

	window = Renderer::GetWindow();

	// Entity Start
	SceneDB::Start();

	// let editor know its 
	DebugDB::MarkDebugStatementsDirty();
}

void Engine::ReadResources()
{
	// Lua Loading
	LuaStateManager::Initialize();
	lua_State* luaState = LuaStateManager::GetLuaState();

	LuaClassAndNamespaceSetup();

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

	DebugDB::Init();

	// template config
	TemplateDB::LoadTemplates();

	// scene config
	if (configDocument.HasMember("initial_scene") && configDocument["initial_scene"].IsString())
	{
		std::string sceneName = configDocument["initial_scene"].GetString();

		SceneDB::LoadScene(sceneName);
	}
	else
	{
		// no initial scene specified
		std::cout << "error: initial_scene unspecified";
		exit(0);
	}

	if (editorInstance)
	{
		DebugDB::saveDebugStatements = true;
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
		.addFunction("AddComponent", &Entity::AddComponent)
		.addFunction("RemoveComponent", &Entity::RemoveComponent)
		.endClass();

	// Entities Namespace inside of Lua
	luabridge::getGlobalNamespace(luaState)
		.beginNamespace("Entity")
		.addFunction("Find", &SceneDB::Find)
		.addFunction("FindAll", &SceneDB::FindAll)
		.addFunction("Instantiate", &SceneDB::Instantiate)
		.addFunction("Destroy", &SceneDB::Destroy)
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

	// Text Scripting Name Space inside of Lua
	luabridge::getGlobalNamespace(luaState)
		.beginNamespace("Text")
		.addFunction("Draw", &TextDB::DrawText)
		.endNamespace();

	// Audio Scripting Name Space inside of Lua
	luabridge::getGlobalNamespace(luaState)
		.beginNamespace("Audio")
		.addFunction("Play", &AudioDB::PlayChannel)
		.addFunction("Halt", &AudioDB::HaltChannel)
		.addFunction("SetVolume", &AudioDB::SetVolume)
		.endNamespace();

	// Image Scripting Name Space inside of Lua
	luabridge::getGlobalNamespace(luaState)
		.beginNamespace("Image")
		.addFunction("Draw", &ImageDB::Draw)
		.addFunction("DrawEx", &ImageDB::DrawEx)
		.addFunction("DrawUI", &ImageDB::DrawUI)
		.addFunction("DrawUIEx", &ImageDB::DrawUIEx)
		.addFunction("DrawPixel", &ImageDB::DrawPixel)
		.addFunction("DrawLine", &ImageDB::DrawLine)
		.endNamespace();

	// Camera Scripting
	luabridge::getGlobalNamespace(luaState)
		.beginNamespace("Camera")
		.addFunction("SetPosition", &Renderer::SetCameraPosition)
		.addFunction("GetPositionX", &Renderer::GetCameraPositionX)
		.addFunction("GetPositionY", &Renderer::GetCameraPositionY)
		.addFunction("SetZoom", &Renderer::SetZoomFactor)
		.addFunction("GetZoom", &Renderer::GetZoomFactor)
		.endNamespace();

	// Scene Scripting
	luabridge::getGlobalNamespace(luaState)
		.beginNamespace("Scene")
		.addFunction("Load", &SceneDB::RequestLoadNewScene)
		.addFunction("GetCurrent", &SceneDB::GetCurrentSceneName)
		.addFunction("DontDestroy", &SceneDB::DontDestroy)
		.endNamespace();

	// Vector Namespace
	// TODO: 2 different vec2 and Vector2, this is gross, fix it
	luabridge::getGlobalNamespace(luaState)
		.beginClass<b2Vec2>("Vector2")
		.addConstructor<void(*) (float, float)>()
		.addProperty("x", &b2Vec2::x)
		.addProperty("y", &b2Vec2::y)
		.addFunction("Normalize", &b2Vec2::Normalize)
		.addFunction("Length", &b2Vec2::Length)
		.addFunction("__add", &b2Vec2::operator_add)
		.addFunction("__sub", &b2Vec2::operator_sub)
		.addFunction("__mul", &b2Vec2::operator_mul)
		.addStaticFunction("Distance", static_cast<float (*)(const b2Vec2&, const b2Vec2&)>(&b2Distance))
		.addStaticFunction("Dot", static_cast<float (*)(const b2Vec2&, const b2Vec2&)>(&b2Dot))
		.addStaticFunction("Cross", static_cast<float (*)(const b2Vec2&, const b2Vec2&)>(&b2Cross))
		.endClass();

	// Collision Class inside of Lua
	luabridge::getGlobalNamespace(luaState)
		.beginClass<Collision2D>("collision")
		.addProperty("other", &Collision2D::other)
		.addProperty("point", &Collision2D::point)
		.addProperty("relative_velocity", &Collision2D::relativeVelocity)
		.addProperty("normal", &Collision2D::normal)
		.endClass();

	// Collision Class inside of Lua
	luabridge::getGlobalNamespace(luaState)
		.beginClass<HitResult>("HitResult")
		.addProperty("entity", &HitResult::entity)
		.addProperty("point", &HitResult::point)
		.addProperty("normal", &HitResult::normal)
		.addProperty("is_trigger", &HitResult::isTrigger)
		.endClass();

	luabridge::getGlobalNamespace(luaState)
		.beginNamespace("Physics")
		.addFunction("Raycast", &b2WorldDB::Raycast)
		.addFunction("RaycastAll", &b2WorldDB::RaycastAll)
		.endNamespace();

	luabridge::getGlobalNamespace(luaState)
		.beginNamespace("Event")
		.addFunction("Publish", &Event::Publish)
		.addFunction("Unsubscribe", &Event::Unsubscribe)
		.addFunction("Subscribe", &Event::Subscribe)
		.endNamespace();

}


void Engine::Input() // NOTE: THIS IS ONLY CALLED WHEN RUNNING WITHOUT AN EDITOR
{
	SDL_Event event;
	
	while (SDL_PollEvent(&event)) // empty/consume all the events in the event queue
	{
		if (SDL_GetWindowFromID(event.window.windowID) == window)
		{
			if ((event.type == SDL_WINDOWEVENT) && (event.window.event == SDL_WINDOWEVENT_CLOSE))
			{
				exit(1);
			}

			Input::ProcessEvent(event);
		}
	}
}

void Engine::Update()
{
	if (isRunning)
	{
		// Load Pending scene if requested
		if (SceneDB::pendingScene)
		{
			SceneDB::LoadPendingScene();
		}

		SceneDB::Update();

		// Late Update
		SceneDB::LateUpdate();

		Input::LateUpdate();
	}
}

void Engine::Render()
{
	if (isRunning)
	{
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);

		ImageDB::RenderImages();

		ImageDB::RenderUIImages();

		TextDB::RenderText();

		ImageDB::RenderPixels();

		ImageDB::RenderLines();

		SDL_RenderPresent(renderer);
	}
}

void Engine::EndGame()
{
	if (editorInstance)
	{
		isRunning = false;

		SceneDB::Reset();
		ImageDB::Reset();
		AudioDB::Reset();
		TextDB::Reset();
		TemplateDB::Reset();
		DebugDB::Reset();

		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
	}
	else
	{
		exit(1);
	}
}

// Lua Functions
void Engine::Quit()
{
	// TODO: how to handle if we are in an editorInstance?
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
	//std::cout << "Platform not supported\n";
	//exit(0);
	std::string message = "Platform not supported";

	DebugDB::AddStatement(DebugType::LogError, "", "", message);

#endif
	std::system(command.c_str());
	
}

Engine::~Engine()
{
	SDL_DestroyWindow(Renderer::GetWindow());
	SDL_DestroyRenderer(Renderer::GetRenderer());
}

