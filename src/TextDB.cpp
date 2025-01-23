#include "TextDB.h"

std::unordered_map<TextKey, SDL_Texture*, TextKeyHash> TextDB::cachedTextures;
std::unordered_map<std::string, std::unordered_map<int, TTF_Font*>> TextDB::cachedFonts;
std::queue<TextRenderRequest> TextDB::renderRequests;

// SDL Color equality operator for hash tuple key
bool operator==(const SDL_Color& lhs, const SDL_Color& rhs) {
    return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
}

void TextDB::TextDB_Init(const std::string& fontName)
{
    TTF_Init();
}

void TextDB::DrawText(const std::string& textContent, float x, float y, const std::string& fontName, float fontSize, float r, float g, float b, float a)
{
    TTF_Font* font = FindFont(fontName, fontSize);
    SDL_Color fontColor = { static_cast<int>(r), static_cast<int>(g), static_cast<int>(b), static_cast<int>(a) };

    // TODO: TextKey prolly does not need to hold fontSize info since cachedFonts alr does?
    TextKey key = std::make_tuple(textContent, static_cast<int>(fontSize), fontColor);
    SDL_Texture* textTexture;

    // Does not exist yet
    if (cachedTextures.find(key) == cachedTextures.end())
    {
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, textContent.c_str(), fontColor);
        textTexture = SDL_CreateTextureFromSurface(Renderer::GetRenderer(), textSurface);

        cachedTextures[key] = textTexture;

        SDL_FreeSurface(textSurface);
    }
    else
    {
        textTexture = cachedTextures[key];
    }

    SDL_Rect textRect = { static_cast<int>(x), static_cast<int>(y), 0, 0 };
    SDL_QueryTexture(textTexture, nullptr, nullptr, &textRect.w, &textRect.h);

    TextRenderRequest request = TextRenderRequest(textTexture, textRect);
    renderRequests.push(request);
}

void TextDB::RenderText()
{
    SDL_Renderer* renderer = Renderer::GetRenderer();

    // Render Text
    while (!renderRequests.empty())
    {
        Helper::SDL_RenderCopyEx498(0, "", renderer, renderRequests.front().drawTexture, NULL, &renderRequests.front().drawRect, 0, NULL, SDL_FLIP_NONE);
        renderRequests.pop();
    }
}

TTF_Font* TextDB::FindFont(const std::string& fontName, int fontSize)
{
    // does cached font exist
    if (cachedFonts.find(fontName) != cachedFonts.end() && cachedFonts[fontName].find(fontSize) != cachedFonts[fontName].end())
    {
        return cachedFonts[fontName][fontSize];
    }
    else
    {
        std::string fontPath = "resources/fonts/" + fontName + ".ttf";
        TTF_Font* font = TTF_OpenFont(fontPath.c_str(), fontSize);

        if (font == nullptr)
        {
            std::cout << "error: font " << fontName << " missing";
            exit(0);
        }

        // cache font
        cachedFonts[fontName][fontSize] = font;

        return font;
    }

}


// EDITOR ONLY
void TextDB::Reset()
{
    cachedTextures.clear();
    cachedFonts.clear();

    while (!renderRequests.empty())
    {
        renderRequests.pop();
    }
}