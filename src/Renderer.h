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
	static void RendererResetDefaults();

	static SDL_Renderer* GetRenderer() { return renderer;  }
	static glm::ivec2 GetResolution() { return resolution; }

	static float GetZoomFactor() { return zoomFactor; }
	static void SetZoomFactor(float zoomFactorIn) { zoomFactor = zoomFactorIn; }

	static glm::vec2 GetCameraPosition() { return cameraPosition; }
	static void SetCameraPosition(float x, float y) { cameraPosition = glm::vec2(x, y); }
	static float GetCameraPositionX() { return cameraPosition.x; }
	static float GetCameraPositionY() { return cameraPosition.y; }

	static int GetPixelsPerUnit() { return pixelsPerUnit; }

private:
	static SDL_Window* window;
	static SDL_Renderer* renderer;

	static glm::ivec2 resolution;
	static glm::vec2 cameraPosition;

	static float originalZoomFactor;
	static float zoomFactor;

	static int pixelsPerUnit;

};
#endif

