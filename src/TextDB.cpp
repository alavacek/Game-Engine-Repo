#include "TextDB.h"


std::vector<std::string> TextDB::introTexts;
TTF_Font* TextDB::font;
SDL_Color TextDB::white = { 255, 255, 255, 255 };

uint16_t TextDB::currIntroText = 0;
bool TextDB::finishedIntro = false;


void TextDB::TextDB_Init(const std::string& fontName)
{
    TTF_Init();

    std::string fontPath = "resources/fonts/" + fontName + ".ttf";
    font = TTF_OpenFont(fontPath.c_str(), 16);
    if (font == nullptr) 
    {
        std::cout << "error: font " << fontName << " missing";
        exit(0);
    }
}

void TextDB::LoadIntroText(const rapidjson::Document& configDocument)
{
    if (configDocument.HasMember("intro_text") && configDocument["intro_text"].IsArray()) {
        const rapidjson::Value& introTextArray = configDocument["intro_text"];
        for (rapidjson::SizeType i = 0; i < introTextArray.Size(); i++) {
            if (introTextArray[i].IsString()) {
                introTexts.push_back(introTextArray[i].GetString());
            }
        }
        finishedIntro = (introTexts.size() == 0);
    }
    else
    {
        finishedIntro = true;
    }
    
}

void TextDB::ProgressToNextIntroText()
{
    if (currIntroText < introTexts.size())
    {
        currIntroText++;

        if (currIntroText == introTexts.size())
        {
            finishedIntro = true;
        }
    }
}

void TextDB::DrawCurrentIntroText(uint32_t windowHeight)
{
    if (currIntroText < introTexts.size())
    {
        // draw texts  
        DrawText(introTexts[currIntroText], 16, white, 25, windowHeight - 50);
    }
    else if (finishedIntro && introTexts.size() > 0)
    {
        DrawText(introTexts[introTexts.size() - 1], 16, white, 25, windowHeight - 50);
    }
}

void TextDB::DrawText(const std::string& textContent, uint32_t fontSize, SDL_Color fontColor, uint32_t x, uint32_t y)
{
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, textContent.c_str(), fontColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(Renderer::GetRenderer(), textSurface);

    SDL_Rect textRect = { x, y, textSurface->w, textSurface->h };
    SDL_RenderCopy(Renderer::GetRenderer(), textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}