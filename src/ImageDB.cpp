#include "ImageDB.h"

std::unordered_map<std::string, SDL_Texture*> ImageDB::cachedTextures;
std::vector<SDL_Texture*> ImageDB::introTextures;
uint16_t ImageDB::currIntroImage = 0;
bool ImageDB::finishedIntro = false;

bool ImageDB::LoadIntroImages(const rapidjson::Document& configDocument)
{
    if (configDocument.HasMember("intro_image") && configDocument["intro_image"].IsArray())
    {
        // Get the actors array
        const rapidjson::Value& introImages = configDocument["intro_image"];

        for (rapidjson::SizeType i = 0; i < introImages.Size(); i++) 
        {
            if (introImages[i].IsString()) 
            {
                std::string imageName = introImages[i].GetString();
                
                introTextures.push_back(LoadImage(imageName));
            }
        }

        if (introTextures.size() == 0)
        {
            return false;
        }

        return true;
    }

    return false;
}

void ImageDB::ProgressToNextIntroImage()
{ 
    if (currIntroImage < introTextures.size())
    {
        currIntroImage++;

        if (currIntroImage == introTextures.size())
        {
            finishedIntro = true;
        }
    }
}

void ImageDB::RenderCurrentIntroImage()
{
    if (currIntroImage < introTextures.size())
    {
        SDL_RenderCopy(Renderer::GetRenderer(), introTextures[currIntroImage], NULL, NULL);
    }
    else if (finishedIntro && introTextures.size() > 0)
    {
        SDL_RenderCopy(Renderer::GetRenderer(), introTextures[introTextures.size() - 1], NULL, NULL);
    }
}

SDL_Texture* ImageDB::LoadImage(const std::string& imageName)
{
    if (cachedTextures.find(imageName) != cachedTextures.end())
    {
        return cachedTextures[imageName];
    }

    std::string path = "resources/images/" + imageName + ".png";
    SDL_Texture* texture = IMG_LoadTexture(Renderer::GetRenderer(), path.c_str());
    
    if (!texture) {
        std::cout << "error: missing image " << imageName;
        exit(0);
    }

    cachedTextures[imageName] = texture;

    return texture;
}