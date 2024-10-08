#ifndef RENDERER_H
#define RENDERER_H

#include <iostream>
#include <string>

#include "EngineUtils.h"
#include "glm/glm.hpp"
#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include "SDL2/SDL.h"
#include "SDLHelper.h"

class Renderer
{
public:
	static void RendererInit();

	static SDL_Renderer* GetRenderer() { return renderer;  }
	static glm::ivec2 GetResolution() { return resolution; }
	static glm::ivec2 GetCameraOffset() { return cameraOffset; }
	static double GetZoomFactor() { return zoomFactor; }

private:
	static SDL_Window* window;
	static SDL_Renderer* renderer;
	static glm::ivec2 resolution;
	static glm::ivec2 cameraOffset;
	static double zoomFactor;
};
#endif

