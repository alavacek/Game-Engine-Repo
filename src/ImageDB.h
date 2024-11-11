#ifndef IMAGEDB_H
#define IMAGEDB_H

#include <iostream>
#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include "Renderer.h"
#include "SDL.h"
#include "SDL_image.h"
#include "SDLHelper.h"

struct ImageRenderRequest
{
	ImageRenderRequest(std::string imageNameIn, float xIn, float yIn, int rotationDegreesIn = 0, float scaleXIn = 1, float scaleYIn = 1,
		float pivotXIn = 0.5f, float pivotYIn = 0.5f, int rIn = 255, int gIn = 255, int bIn = 255, int aIn = 255, int sortingOrderIn = 0)
	{
		imageName = imageNameIn;
		x = xIn;
		y = yIn;
		rotationDegrees = rotationDegreesIn;
		scaleX = scaleXIn;
		scaleY = scaleYIn;
		pivotX = pivotXIn;
		pivotY = pivotYIn;
		r = rIn;
		g = gIn;
		b = bIn;
		a = aIn;
		sortingOrder = sortingOrderIn;
	}

	std::string imageName;
	float x;
	float y;
	int rotationDegrees;
	float scaleX;
	float scaleY;
	float pivotX;
	float pivotY;
	// TODO: i hate this, can i make sdl_color into a recognizable structure
	int r;
	int g;
	int b;
	int a;
	int sortingOrder;
};

struct ImageUIRenderRequest
{
	ImageUIRenderRequest(std::string imageNameIn, float xIn, float yIn, int rIn = 255, int gIn = 255, int bIn = 255, int aIn = 255, int sortingOrderIn = 0)
	{
		imageName = imageNameIn;
		x = xIn;
		y = yIn;
		r = rIn;
		g = gIn;
		b = bIn;
		a = aIn;
		sortingOrder = sortingOrderIn;
	}

	std::string imageName;
	float x;
	float y;
	// TODO: i hate this, can i make sdl_color into a recognizable structure
	int r;
	int g;
	int b;
	int a;
	int sortingOrder;
};

struct PixelRenderRequest
{
	PixelRenderRequest(float xIn, float yIn, int rIn = 255, int gIn = 255, int bIn = 255, int aIn = 255)
	{
		x = xIn;
		y = yIn;
		r = rIn;
		g = gIn;
		b = bIn;
		a = aIn;
	}

	float x;
	float y;
	int r;
	int g;
	int b;
	int a;
};

class ImageDB
{
public:
	static SDL_Texture* LoadImage(const std::string& imageName);
	static void RenderImages();
	static void RenderUIImages();
	static void RenderPixels();

	static void Draw(const std::string& imageName, float x, float y);
	static void DrawEx(const std::string& imageName, float x, float y,
		float rotationDegrees, float scaleX, float scaleY, float pivotX, float pivotY, float r, float g, float b, float a, float sortingOrder);

	static void DrawUI(const std::string& imageName, float x, float y);
	static void DrawUIEx(const std::string& imageName, float x, float y, float r, float g, float b, float a, float sortingOrder);

	static void DrawPixel(float x, float y, float r, float g, float b, float a);
	
private:
	static bool CompareImageRequests(const ImageRenderRequest& a, const ImageRenderRequest& b);
	static bool CompareUIImageRequests(const ImageUIRenderRequest& a, const ImageUIRenderRequest& b);

	//TODO: could keep track of frames since used and get rid of images that havent been used for several frames
	static std::unordered_map<std::string, SDL_Texture*> cachedTextures;
	static std::deque<ImageRenderRequest> renderRequests;
	static std::deque<ImageUIRenderRequest> renderUIRequests;
	static std::deque<PixelRenderRequest> renderPixelRequests;
};
#endif
