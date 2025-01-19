#include "Editor.h"

void Editor::Init()
{
    // setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    window = Helper::SDL_CreateWindow498("Editor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowSize.x, windowSize.y, SDL_WINDOW_RESIZABLE);
    renderer = Helper::SDL_CreateRenderer498(window, 0, 0);

    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable keyboard controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

    // theme
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    // Start the Dear ImGui frame
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();

    // create engine
    engine = new Engine();
    engine->ReadResources();

    // Initial Render
    // Start the ImGui frame
    ImGui::NewFrame();

    // properties window
    RenderEditor(true);

    // send ImGui context to SDL Renderer context
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);

    // show all renders now
    SDL_RenderPresent(renderer);

    // clear the renders 
    SDL_RenderClear(renderer);
}

void Editor::EditorLoop()
{
    // Main loop
    bool running = true;

    while (running) 
    {
        SDL_Event event;
        while (SDL_PollEvent(&event)) 
        {
            // process editor events
            if (SDL_GetWindowFromID(event.window.windowID) == window)
            {
                if ((event.type == SDL_WINDOWEVENT) && (event.window.event == SDL_WINDOWEVENT_CLOSE))
                {
                    running = false;
                }
                ImGui_ImplSDL2_ProcessEvent(&event);
            }
            else if (simulating && engine && SDL_GetWindowFromID(event.window.windowID) == engine->GetWindow())
            {
                if ((event.type == SDL_WINDOWEVENT) && (event.window.event == SDL_WINDOWEVENT_CLOSE))
                {
                    StopSimulation();
                    continue;
                }

                Input::ProcessEvent(event);
            }
        }

        // Start the ImGui frame
        ImGui::NewFrame();

        // properties window
        RenderEditor();

        // send ImGui context to SDL Renderer context
        ImGui::Render();
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);

        // show all renders now
        SDL_RenderPresent(renderer);

        // clear the renders 
        SDL_RenderClear(renderer);

        // if a simulation is running, run its logic
        if (simulating)
        {
            if (engine->GetIsRunning())
            {
                engine->Frame();
            }
            else
            {
                // stopped within engine, maybe from exit event
                StopSimulation();
            }
        }

        SDL_Delay(16); //time for each frame
    }
}

void Editor::EditorCleanup()
{
    // Cleanup
    ImGui_ImplSDL2_Shutdown();
    SDL_Quit();
}

void Editor::RenderEditor(bool resetDefaults)
{
    // Retrieve the main viewport size for fullscreen docking.
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    //ImGui::SetNextWindowViewport(viewport->ID);

    // Set up the main editor window as a dockspace.
    ImGuiWindowFlags mainWindowFlags = ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::Begin("Editor", nullptr, mainWindowFlags);

    // Enable Dockspace
    ImGuiID dockspaceID = ImGui::GetID("MyDockspace");
    ImGui::DockSpace(dockspaceID, ImVec2(0, 0));

    // Inspector Defaults
    // TODO
    if (resetDefaults)
    {
        ImGui::SetNextWindowCollapsed(false);
        
    }

    // Inspector Panel
    ImGui::Begin("Inspector");
    RenderInspector();
    ImGui::End();

    // Scene Hierarchy Panel
    ImGui::Begin("Scene Hierarchy");
    RenderSceneHierarchy();
    ImGui::End();


    // Scene Hierarchy Panel
    ImGui::Begin("Simulate");

    if (ImGui::Button("Play"))
    {
        // When the button is pressed, call PlayFunction
        Simulate();
    }

    ImGui::End();


    // Asset Browser Panel
    ImGui::Begin("Asset Browser");
    ImGui::Text("Asset Browser Content");
    ImGui::End();

    // Debug Console Panel
    ImGui::Begin("Debug Console");
    RenderDebug();
    ImGui::End();

    ImGui::End(); // End Editor Window
  
}

void Editor::RenderSceneHierarchy() 
{
    for (int i = 0; i < SceneDB::GetNumberOfEntitiesInScene(); i++)  
    {
        Entity* entity = SceneDB::GetEntityAtIndex(i);
        if (ImGui::Selectable(entity->GetName().c_str(), selectedEntity == entity)) 
        {
            // newly selected, reset selectedComponent
            if (selectedEntity != entity)
            {
                selectedComponent = std::make_pair("", nullptr);
            }

            // update selected entity
            selectedEntity = entity; 

        }

        if (entity != nullptr && selectedEntity == entity)
        {
            if (ImGui::Button("Remove Entity"))
            {
                if (simulating)
                {
                    SceneDB::Destroy(entity);
                }
                else // actually remove from json file
                {
                    // TODO HANDLE TEMPLATES
                    EngineUtils::RemoveEntityFromJson(SceneDB::GetCurrentScenePath(), entity->entityName);

                    SceneDB::RemoveEntityOutOfSimulation(entity);
                }

                selectedEntity = nullptr;
                selectedComponent = std::make_pair("", nullptr);
            }
        }
    }
}

void Editor::RenderInspector()
{
    if (!selectedEntity) return;
    ImGui::Text(selectedEntity->GetName().c_str());
    ImGui::Text("Components:");
    bool removeSelectedComponent = false;
    
    for (auto& component : selectedEntity->components)
    {
        std::string componentInfo = component.first + " : " + component.second->type;
        if (ImGui::Selectable(componentInfo.c_str(), selectedComponent == component))
        {
            selectedComponent = std::make_pair(component.first, component.second);
        }

        if (selectedComponent == component)
        {
            if (ImGui::Button("Remove Component"))
            {
                removeSelectedComponent = true;

            }
        }
    }

    if (removeSelectedComponent)
    {
        if (simulating)
        {
            SceneDB::Find(selectedComponent.second->owningEntityName)->RemoveComponentByKey(selectedComponent.first);
        }
        else // actually remove from json file
        {
            EngineUtils::RemoveComponentFromJson(SceneDB::GetCurrentScenePath(), selectedEntity->entityName, selectedComponent.first);
            selectedEntity->RemoveComponentByKeyOutOfSimulation(selectedComponent.first);
        }

        selectedComponent = std::make_pair("", nullptr);
    }

}

void Editor::RenderDebug()
{
    if (DebugDB::DebugStatementsUpdate())
    {
        std::vector<DebugLog*> debugStatements = DebugDB::GetDebugStatementsAndMarkClean();

        debugLogs = "";

        for (DebugLog* log : debugStatements)
        {
            switch (log->type) 
            {
                case (DebugType::Log):
                    debugLogs += log->message + "\n";
                    break;
                case  (DebugType::LogError):
                    debugLogs += "ERROR:" + log->message + "\n";
                    break;
                default:
                    debugLogs += log->message + "\n";
            }
        }
    }

    ImGui::Text(debugLogs.c_str());
}

void Editor::Simulate()
{
    // Simulation already open
    if (simulating)
    {
        StopSimulation();
    }

    engine->Start();
    simulating = true;
}

void Editor::StopSimulation()
{
    if (simulating)
    {
        engine->EndGame();
        simulating = false;
        engine->ReadResources();

        selectedEntity = nullptr;
        selectedComponent = std::make_pair("", nullptr);
    }

}

Editor::~Editor()
{

    if (engine)
    {
        engine->Quit();
        delete(engine);
    }
}



