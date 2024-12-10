#include "Editor.h"

void Editor::RenderEditor()
{
    //Engine engine;
    // engine.GameLoop();
    
    //// Start Docking Space
    //ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

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



