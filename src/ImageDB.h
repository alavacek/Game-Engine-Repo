#ifndef IMAGEDB_H
#define IMAGEDB_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include "Renderer.h"
#include "SDL.h"
#include "SDL_image.h"
#include "SDLHelper.h"

class ImageDB
{
public:
	static bool LoadIntroImages(const rapidjson::Document& configDocument); //return false if none
	static void ProgressToNextIntroImage();
	static void RenderCurrentIntroImage();
	static bool FinishedWithIntro() { return finishedIntro; }

	static SDL_Texture* GetCurrentIntroTexture() { return introTextures[currIntroImage]; }

	static SDL_Texture* LoadImage(const std::string& imageName);

private:
	//TODO: could keep track of frames since used and get rid of images that havent been used for several frames
	static std::unordered_map<std::string, SDL_Texture*> cachedTextures;

	static std::vector<SDL_Texture*> introTextures;
	static uint16_t currIntroImage;
	static bool finishedIntro;
};
#endif
