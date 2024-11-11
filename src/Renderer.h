#ifndef RENDERER_H
#define RENDERER_H

#include <iostream>
#include <string>
#include <queue>

#include "EngineUtils.h"
#include "glm/glm.hpp"
#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include "SDL.h"
#include "SDL2/SDL.h"
#include "SDLHelper.h"


class Renderer
{
public:
	static void RendererInit();

	static SDL_Renderer* GetRenderer() { return renderer;  }
	static glm::ivec2 GetResolution() { return resolution; }
	static double GetZoomFactor() { return zoomFactor; }
	static float GetCameraEaseFactor() { return cameraEaseFactor; }

	static glm::vec2 GetCameraPosition() { return glm::vec2(0, 0); }
	static glm::vec2 GetCameraDimensions() { return glm::vec2(100, 100); }

private:
	static SDL_Window* window;
	static SDL_Renderer* renderer;
	static glm::ivec2 resolution;
	static double zoomFactor;
	static float cameraEaseFactor;
};
#endif

