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

	void Simulate();
	void StopSimulation();

	~Editor();
private:
	Engine* engine = nullptr;
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	Entity* selectedEntity = nullptr;

	SDL_GLContext glContext;
	ImVec2 windowSize = ImVec2(1280, 720);

	std::string debugLogs = "";
	bool simulating = false;

};

#endif
