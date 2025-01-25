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

    loadedScene = engine->GetInitialSceneName();

    // Initial Render
    // Start the ImGui frame
    ImGui::NewFrame();

    // properties window
    RenderEditor(true);

    // send ImGui context to SDL Renderer context
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);

    // start with scene assets loaded
    currentCategory = AssetCategory::Scenes;
    displayedAssets = EngineUtils::GetFilesInDirectory(sceneDir, ".scene");

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
    ImGui::Begin("Asset Browser", nullptr, ImGuiWindowFlags_MenuBar);
    RenderAssets();
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
            if (currentCategory == AssetCategory::Templates)
            {
                selectedAsset = "";
            }

        }

        if (entity != nullptr && selectedEntity == entity)
        {
            if (ImGui::Button("Remove Entity"))
            {
                if (simulating)
                {
                    SceneDB::Destroy(entity);
                    selectedEntity = nullptr;
                }
                else // actually remove from json file
                {
                    EngineUtils::RemoveEntityFromJson(SceneDB::GetCurrentScenePath(), entity->entityName);

                    SceneDB::RemoveEntityOutOfSimulation(entity);
                    selectedEntity = nullptr;
                    ResetEditor();
                }

                selectedComponent = std::make_pair("", nullptr);
            }
        }
    }

    if (ImGui::Button("Add Entity"))
    {
        showAddEntityWindow = true;
    }

    if (showAddEntityWindow)
    {
        RenderAddEntity();
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
            // TODO: do i want additional logic here if we update a template mid simulation?
            selectedEntity->RemoveComponentByKey(selectedComponent.first);
        }
        else // actually remove from json file
        {
            if (Template* selectedTemplate = dynamic_cast<Template*>(selectedEntity))
            {
                std::string path = templateDir + selectedTemplate->templateName + ".template";
                EngineUtils::RemoveComponentFromTemplateJson(path, selectedComponent.first);
            }
            else
            {
                EngineUtils::RemoveComponentFromJson(SceneDB::GetCurrentScenePath(), selectedEntity->entityName, selectedComponent.first);
            }

            ResetEditor();
        }

        selectedComponent = std::make_pair("", nullptr);
    }

    if (ImGui::Button("Add Component"))
    {
        showAddComponentWindow = true;
    }

    if (showAddComponentWindow)
    {
        RenderAddComponent();
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

void Editor::RenderAddEntity()
{
    ImGui::Begin("Add Entity to Scene");

    for (std::string templateName : TemplateDB::GetBuiltInTemplates())
    {
        if (ImGui::Selectable(templateName.c_str(), entityToAddTemplateName == templateName))
        {
            entityToAddTemplateName = templateName;
        }
    }

    if (entityToAddTemplateName != "")
    {
        if (ImGui::Button("Add Entity"))
        {
            AddTemplateTypeToCurrentScene(entityToAddTemplateName);

            entityToAddTemplateName = "";
            showAddEntityWindow = false;
        }
    }
    
    if (ImGui::Button("Cancel"))
    {
        entityToAddTemplateName = "";
        showAddEntityWindow = false;
    }

    ImGui::End();
}

void Editor::RenderAddComponent()
{
    std::string windowTitle = "Add Component to " + selectedEntity->GetName();
    ImGui::Begin(windowTitle.c_str());

    for (std::string component : ComponentDB::componentsAlphabeticalOrder)
    {
        if (ImGui::Selectable(component.c_str(), componentToAdd == component))
        {
            componentToAdd = component;
        }
    }

    if (selectedEntity && componentToAdd != "")
    {
        if (ImGui::Button("Add Component"))
        {
            if (simulating)
            {
                // TODO: do i want additional logic here if we update a template mid simulation?
                selectedEntity->AddComponent(componentToAdd);
            }
            else
            {
                std::string componentKey = std::to_string(selectedEntity->componentCounter);

                if (Template * selectedTemplate = dynamic_cast<Template*>(selectedEntity))
                {
                    std::string path = templateDir + selectedTemplate->templateName + ".template";
                    EngineUtils::AddComponentToTemplateInJson(path, componentKey, componentToAdd);
                }
                else
                {
                    EngineUtils::AddComponentToEntityInJson(SceneDB::GetCurrentScenePath(), selectedEntity->GetName(), componentKey, componentToAdd);

                }

                ResetEditor();
            }

            showAddComponentWindow = false;
            componentToAdd = "";
        }
    }

    if (ImGui::Button("Cancel"))
    {

        showAddComponentWindow = false;
        componentToAdd = "";
    }

    ImGui::End();
}

void Editor::RenderAssets()
{
    if (ImGui::BeginMenuBar()) 
    {
        if (ImGui::MenuItem("Scenes", nullptr, currentCategory == AssetCategory::Scenes)) 
        {
            currentCategory = AssetCategory::Scenes;
            displayedAssets = EngineUtils::GetFilesInDirectory(sceneDir, ".scene");
            selectedAsset = ""; // reset
        }
        if (ImGui::MenuItem("Templates", nullptr, currentCategory == AssetCategory::Templates)) 
        {
            currentCategory = AssetCategory::Templates;
            displayedAssets = EngineUtils::GetFilesInDirectory(templateDir, ".template");
            selectedAsset = ""; // reset
        }
        if (ImGui::MenuItem("Components", nullptr, currentCategory == AssetCategory::Components)) 
        {
            currentCategory = AssetCategory::Components;
            displayedAssets = EngineUtils::GetFilesInDirectory(componentDir, ".lua");
            selectedAsset = ""; // reset
        }
        ImGui::EndMenuBar();
    }

    if (!displayedAssets.empty())
    {
        bool deleteAsset = false;

        for (const auto& asset : displayedAssets)
        {
            if (ImGui::Selectable(asset.c_str(), selectedAsset == asset)) 
            {
                selectedAsset = asset.c_str();
            }

            if (selectedAsset == asset)
            {               
                if (!simulating && currentCategory == AssetCategory::Scenes)
                {
                    if (ImGui::Button("Load Scene"))
                    {
                        loadedScene = selectedAsset;
                        selectedEntity = nullptr;
                        ResetEditor();
                    }
                }
                else if (currentCategory == AssetCategory::Templates)
                {
                    if (ImGui::Button("Add Template to Scene"))
                    {
                        AddTemplateTypeToCurrentScene(selectedAsset);
                    }
                    selectedEntity = TemplateDB::FindTemplate(selectedAsset);
                }
                else if (currentCategory == AssetCategory::Components)
                {
                    if (ImGui::Button("Open Asset"))
                    {
                        std::string assetPath = componentDir + selectedAsset + ".lua";
                        EngineUtils::OpenAsset(assetPath);
                    }
                }

                // Delete Asset
                if (!simulating)
                {
                    if (currentCategory == AssetCategory::Scenes)
                    {
                        // only want to ber able to delete 
                        if (selectedAsset != "" && displayedAssets.size() > 1)
                        { // Ensure an asset is selected
                            if (ImGui::Button("Delete Scene"))
                            {
                                deleteAsset = true;
                            }
                        }
                    }
                    else if (currentCategory == AssetCategory::Templates)
                    {
                        if (selectedAsset != "")
                        {
                            if (ImGui::Button("Delete Template"))
                            {
                                deleteAsset = true;
                            }
                        }
                    }
                    else if (currentCategory == AssetCategory::Components)
                    {
                        if (selectedAsset != "")
                        {
                            if (ImGui::Button("Delete Component"))
                            {
                                deleteAsset = true;
                            }
                        }
                    }
                }
            }
        }

        if (deleteAsset)
        {
            if (currentCategory == AssetCategory::Scenes)
            {
                if (selectedAsset == loadedScene)
                {
                    if (displayedAssets.size() > 0)
                    {
                        loadedScene = displayedAssets[0];
                    }
                }

                std::string assetPath = sceneDir + selectedAsset + ".scene";
                EngineUtils::DeleteAsset(assetPath);

                displayedAssets = EngineUtils::GetFilesInDirectory(sceneDir, ".scene");

            }
            else if (currentCategory == AssetCategory::Templates)
            {
                if (selectedEntity && selectedAsset == dynamic_cast<Template*>(selectedEntity)->templateName)
                {
                    selectedEntity = nullptr;
                }

                std::string assetPath = templateDir + selectedAsset + ".template";
                EngineUtils::DeleteAsset(assetPath);

                engine->ReloadTemplatesFiles();
                ResetEditor();

                displayedAssets = EngineUtils::GetFilesInDirectory(templateDir, ".template");
            }
            else if (currentCategory == AssetCategory::Components)
            {
                std::string assetPath = componentDir + selectedAsset + ".lua";
                EngineUtils::DeleteAsset(assetPath);

                engine->ReloadComponentsFiles();
                ResetEditor();

                displayedAssets = EngineUtils::GetFilesInDirectory(componentDir, ".lua");
            }

            selectedAsset = "";
        }
        
    }

    if (currentCategory == AssetCategory::Scenes)
    {
        if (ImGui::Button("Create New Scene")) 
        {
            std::string emptyScene = emptyAssetDir + "Empty.scene";
            std::string newSceneName = "NewScene.scene"; // TODO: Replace with logic for naming
            EngineUtils::CreateNewAsset(emptyScene, sceneDir, newSceneName);

            displayedAssets = EngineUtils::GetFilesInDirectory(sceneDir, ".scene");
        }
    }
    else if (currentCategory == AssetCategory::Templates) 
    {
        if (ImGui::Button("Create New Template")) 
        {
            std::string emptyTemplate = emptyAssetDir + "Empty.template";
            std::string newTemplateName = "NewTemplate.template"; // TODO: Replace with logic for naming
            EngineUtils::CreateNewAsset(emptyTemplate, templateDir, newTemplateName);

            engine->ReloadTemplatesFiles();
            ResetEditor();

            displayedAssets = EngineUtils::GetFilesInDirectory(templateDir, ".template");
        }
    }
    else if (currentCategory == AssetCategory::Components) 
    {
        if (ImGui::Button("Create New Component")) 
        {
            std::string emptyComponent = emptyAssetDir + "Empty.lua";
            std::string newComponentName = "NewComponent.lua"; // TODO: Replace with logic for naming
            EngineUtils::CreateNewAsset(emptyComponent, componentDir, newComponentName, true);

            engine->ReloadComponentsFiles();
            ResetEditor();

            displayedAssets = EngineUtils::GetFilesInDirectory(componentDir, ".lua");
        }
    }

}


void Editor::ResetEditor()
{
    std::string selectedEntityName = "";
    bool selectedEntityIsTemplate = false;

    // selected entity is a template
    if (Template* selectedTemplate = dynamic_cast<Template*>(selectedEntity))
    {
        selectedEntityIsTemplate = true;
        selectedEntityName = selectedTemplate->templateName;

        engine->ReloadTemplatesFiles();
    }
    // if reloading same scene and an entity is selected in hierarchy
    else if (SceneDB::GetCurrentSceneName() == loadedScene && selectedEntity)
    {
        selectedEntityName = selectedEntity->GetName();
    }

    // reload current scene
    engine->ReloadSceneFiles(loadedScene);

    // reset all of these because they rely on certain entities existing
    if (selectedEntityName != "")
    {
        if (selectedEntityIsTemplate)
        {
            selectedEntity = TemplateDB::FindTemplate(selectedEntityName);
        }
        else
        {
            selectedEntity = SceneDB::Find(selectedEntityName);
            if (selectedEntity == luabridge::LuaRef(LuaStateManager::GetLuaState()))
            {
                selectedEntity = nullptr;
            }
        }

    }
    else
    {
        selectedEntity = nullptr;
    }

    entityToAddTemplateName = "";
    componentToAdd = "";
    selectedComponent = std::make_pair("", nullptr);             
    showAddEntityWindow = false;
    showAddComponentWindow = false;

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

        //reload scene
        SceneDB::LoadScene(loadedScene);

        selectedEntity = nullptr;
        selectedComponent = std::make_pair("", nullptr);
    }

}

void Editor::AddTemplateTypeToCurrentScene(const std::string& templateName)
{
    if (simulating)
    {
        // add entity to simulating game
        SceneDB::Instantiate(templateName);
    }
    else
    {
        Template* temp = TemplateDB::FindTemplate(templateName);

        std::string entityName = temp->GetName();

        if (temp->instanceCountInScene > 0)
        {
            entityName += " (" + std::to_string(temp->instanceCountInScene) + ")";
        }

        EngineUtils::AddTemplateEntityToJson(SceneDB::GetCurrentScenePath(), templateName, entityName);
        ResetEditor();
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



