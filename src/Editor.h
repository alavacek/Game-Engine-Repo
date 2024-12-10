#ifndef EDITOR_H
#define EDITOR_H

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

	void RenderEditor();
	void RenderSceneHierarchy();
	void RenderInspector(Entity* entity);

	void Simulate();
	void StopSimulation();

	~Editor();
private:
	Engine* currentSimulation = nullptr;
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_GLContext glContext;

	bool simulating = false;

};

#endif
