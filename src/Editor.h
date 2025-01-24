#ifndef EDITOR_H
#define EDITOR_H

#include "DebugDB.h"
#include "Engine.h"
#include "EngineUtils.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "Renderer.h"
#include "SDL2/SDL.h"
#include "SDLHelper.h"

enum class AssetCategory {
	Scenes,
	Templates,
	Components
};

class Editor
{
public:
	void Init();
	void EditorLoop();
	void EditorCleanup();

	void RenderEditor(bool resetDefaults = false);
	void RenderSceneHierarchy();
	void RenderInspector();
	void RenderDebug();
	void RenderAddEntity();
	void RenderAddComponent();
	void RenderAssets();
	
	void ResetEditor();

	void Simulate();
	void StopSimulation();
	void AddTemplateTypeToCurrentScene(const std::string& templateName);

	~Editor();
private:
	Engine* engine = nullptr;
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;

	Entity* selectedEntity = nullptr;

	std::pair<std::string, Component*> selectedComponent = std::make_pair("", nullptr);
	std::string loadedScene = "";
	std::string entityToAddTemplateName = "";
	std::string componentToAdd = "";

	AssetCategory currentCategory = AssetCategory::Scenes;
	std::vector<std::string> displayedAssets;
	std::string selectedAsset = "";

	// Asset directories for each category
	const std::string sceneDir = "resources/scenes/";
	const std::string templateDir = "resources/templates/";
	const std::string componentDir = "resources/components_types/";

	SDL_GLContext glContext;
	ImVec2 windowSize = ImVec2(1280, 720);

	std::string debugLogs = "";

	bool simulating = false;
	bool showAddEntityWindow = false;
	bool showAddComponentWindow = false;

};

#endif
