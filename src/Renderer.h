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

struct RenderRequest
{
	RenderRequest(SDL_Texture* drawTextureIn, SDL_Rect drawRectIn)
	{
		drawTexture = drawTextureIn;
		drawRect = drawRectIn;
	}

	SDL_Texture* drawTexture;
	SDL_Rect drawRect;

};

class Renderer
{
public:
	static void RendererInit();

	static SDL_Renderer* GetRenderer() { return renderer;  }
	static glm::ivec2 GetResolution() { return resolution; }
	static double GetZoomFactor() { return zoomFactor; }
	static float GetCameraEaseFactor() { return cameraEaseFactor; }

	static void RequestRender(SDL_Texture* drawTexture, SDL_Rect drawRect);
	static void RenderRequests();

private:
	static std::queue<RenderRequest> renderRequests;
	static SDL_Window* window;
	static SDL_Renderer* renderer;
	static glm::ivec2 resolution;
	static double zoomFactor;
	static float cameraEaseFactor;
};
#endif

