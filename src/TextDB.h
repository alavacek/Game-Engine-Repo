#ifndef TEXTDB_H
#define TEXTDB_H

#include <iostream>
#include <queue>
#include <string>
#include <tuple>
#include <vector>

#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include "Renderer.h"
#include "SDL.h"
#include "SDL_ttf.h"

struct TextRenderRequest
{
	TextRenderRequest(SDL_Texture* drawTextureIn, SDL_Rect drawRectIn)
	{
		drawTexture = drawTextureIn;
		drawRect = drawRectIn;
	}

	SDL_Texture* drawTexture;
	SDL_Rect drawRect;
};

using TextKey = std::tuple<std::string, uint32_t, SDL_Color>;

struct TextKeyHash {
	std::size_t operator()(const TextKey& key) const {
		// Hash combining text content, font size, and font color
		auto h1 = std::hash<std::string>{}(std::get<0>(key));
		auto h2 = std::hash<uint32_t>{}(std::get<1>(key));
		auto h3 = std::hash<int>{}(std::get<2>(key).r) ^ std::hash<int>{}(std::get<2>(key).g) ^
			std::hash<int>{}(std::get<2>(key).b) ^ std::hash<int>{}(std::get<2>(key).a);
		return h1 ^ h2 ^ h3;
	}
};

class TextDB
{
public:
	static void TextDB_Init(const std::string& fontName);
	
	static void DrawText(const std::string& textContent, float x, float y, 
		const std::string& fontName, float fontSize, float r, float g, float b, float a);
	static void RenderText();
private:
	static TTF_Font* FindFont(const std::string& fontName, int fontSize);
	static std::unordered_map<TextKey, SDL_Texture*, TextKeyHash> cachedTextures;
	static std::unordered_map<std::string, std::unordered_map<int, TTF_Font*>> cachedFonts;
	static std::queue<TextRenderRequest> renderRequests;
};

#endif