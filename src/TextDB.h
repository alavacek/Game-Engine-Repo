#ifndef TEXTDB_H
#define TEXTDB_H

#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include "Renderer.h"
#include "SDL.h"
#include "SDL_ttf.h"

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
	static void LoadIntroText(const rapidjson::Document& configDocument);
	static void ProgressToNextIntroText();
	static void DrawCurrentIntroText(uint32_t windowHeight);
	static bool FinishedWithIntro() { return finishedIntro; }
	
	static void DrawText(const std::string& textContent, uint32_t fontSize, SDL_Color fontColor, uint32_t x, uint32_t y);
private:
	static std::unordered_map<TextKey, SDL_Texture*, TextKeyHash> cachedTextures;
	static std::vector<std::string> introTexts;
	static TTF_Font* font;
	static SDL_Color white;

	static uint16_t currIntroText;
	static bool finishedIntro;
};

#endif