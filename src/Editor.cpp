#include "Editor.h"

void Editor::Init()
{
    // setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    window = Helper::SDL_CreateWindow498("ImGui Example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_RESIZABLE);
    renderer = Helper::SDL_CreateRenderer498(window, 0, 0);


    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable keyboard controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

    // theme
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    // Start the Dear ImGui frame
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
}

void Editor::EditorLoop()
{
    // Main loop
    bool running = true;

    while (running) 
    {
        SDL_Event event;
        while (Helper::SDL_PollEvent498(&event)) 
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
            ImGui_ImplSDL2_ProcessEvent(&event);
        }

        // Start the ImGui frame
        ImGui::NewFrame();

        // properties window
        RenderEditor();

        // send ImGui context to SDL Renderer context
        ImGui::Render();
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);

        // show all renders now
        Helper::SDL_RenderPresent498(renderer);

        // clear the renders 
        SDL_RenderClear(renderer);

        // if a simulation is running, run its logic
        if (simulating)
        {
            currentSimulation->Frame();
        }
    }
}

void Editor::EditorCleanup()
{
    // Cleanup
    ImGui_ImplSDL2_Shutdown();
    SDL_Quit();
}

void Editor::RenderEditor()
{
    ImGui::Begin("Editor");

    if (ImGui::Button("Play")) 
    {
        // When the button is pressed, call PlayFunction
        Simulate();
    }
    

    //// Scene Hierarchy
    //ImGui::Begin("Scene Hierarchy");
    //// Add hierarchy content here
    //ImGui::End();

    //// Inspector Panel
    //ImGui::Begin("Inspector");
    //// Add inspector content here
    //ImGui::End();

    //// Asset Browser
    //ImGui::Begin("Asset Browser");
    //// Add asset browser content here
    //ImGui::End();

    //// Debug Console
    //ImGui::Begin("Debug Console");
    //// Add debug logs here
    //ImGui::End();

    ImGui::End();

    
}

void Editor::RenderSceneHierarchy() 
{
    //for (auto& entity : scene->GetEntities()) 
    //{
    //    if (ImGui::Selectable(entity->GetName().c_str(), selectedEntity == entity)) {
    //        selectedEntity = entity; // Update selected entity
    //    }
    //}
}

void Editor::RenderInspector(Entity* entity)
{
    if (!entity) return;

    // TODO
}

void Editor::Simulate()
{
    // Simulation already open
    if (simulating)
    {
        currentSimulation->EndGame();
        delete(currentSimulation);
    }

    currentSimulation = new Engine();
    currentSimulation->Start();
    simulating = true;
}

void Editor::StopSimulation()
{
    if (simulating)
    {
        currentSimulation->EndGame();
        delete(currentSimulation);
        simulating = false;
    }

}

Editor::~Editor()
{

    if (currentSimulation)
    {
        currentSimulation->Quit();
        delete(currentSimulation);
    }
}



