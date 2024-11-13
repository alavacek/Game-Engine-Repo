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

	// Entity Start
	SceneDB::Start();
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

		SceneDB::LoadScene(sceneName);
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
		// Load Pending scene if requested
		if (SceneDB::pendingScene)
		{
			SceneDB::LoadPendingScene();
		}

		SceneDB::Update();

		// Late Update
		SceneDB::LateUpdate();
		Input::LateUpdate();;
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

		Helper::SDL_RenderPresent498(renderer);
	}
}

void Engine::EndGame()
{
	
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

}

