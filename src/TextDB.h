#ifndef TEXTDB_H
#define TEXTDB_H

#include <iostream>
#include <string>
#include <vector>

#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include "Renderer.h"
#include "SDL.h"
#include "SDL_ttf.h"

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
	static std::vector<std::string> introTexts; // TODO: maybe should move to heap?
	static TTF_Font* font;
	static SDL_Color white;

	static uint16_t currIntroText;
	static bool finishedIntro;
};

#endif