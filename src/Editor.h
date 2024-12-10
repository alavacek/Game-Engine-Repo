#ifndef EDITOR_H
#define EDITOR_H

#include "Engine.h"
#include "EngineUtils.h"
#include "Renderer.h"
#include "SDL2/SDL.h"
#include "SDLHelper.h"

class Editor
{
public:
	void RenderEditor();
	void RenderSceneHierarchy();
	void RenderInspector(Entity* entity);
};

#endif
