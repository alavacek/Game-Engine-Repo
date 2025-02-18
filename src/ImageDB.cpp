#include "ImageDB.h"

std::unordered_map<std::string, SDL_Texture*> ImageDB::cachedTextures;
std::deque<ImageRenderRequest> ImageDB::renderRequests;
std::deque<ImageUIRenderRequest> ImageDB::renderUIRequests;
std::deque<PixelRenderRequest> ImageDB::renderPixelRequests;
std::deque<LineRenderRequest> ImageDB::renderLineRequests;

SDL_Texture* ImageDB::LoadImage(const std::string& imageName)
{
    if (cachedTextures.find(imageName) != cachedTextures.end())
    {
        return cachedTextures[imageName];
    }

    std::string path = "resources/images/" + imageName + ".png";
    SDL_Texture* texture = IMG_LoadTexture(Renderer::GetRenderer(), path.c_str());
    
    if (!texture) {
        //std::cout << "error: missing image " << imageName;
        //exit(0);

        std::string message = "missing image " + imageName;

        DebugDB::AddStatement(DebugType::LogError, "", "", message);
        return nullptr;
    }

    cachedTextures[imageName] = texture;

    return texture;
}

void ImageDB::RenderImages()
{
    // Regular Render Requests
    std::stable_sort(renderRequests.begin(), renderRequests.end(), CompareImageRequests);

    float zoomFactor = Renderer::GetZoomFactor();
    int pixelsPerUnit = Renderer::GetPixelsPerUnit();

    SDL_RenderSetScale(Renderer::GetRenderer(), zoomFactor, zoomFactor);

    for (auto& request : renderRequests)
    {
        glm::vec2 finalRenderingPosition = glm::vec2(request.x, request.y) - Renderer::GetCameraPosition();

        SDL_Texture* texture = LoadImage(request.imageName);

        // failed to load
        if (texture == nullptr)
        {
            continue;
        }

        SDL_Rect textureRect;

        SDL_QueryTexture(texture, NULL, NULL, &textureRect.w, &textureRect.h);

        // Apply scale
        int flipMode = SDL_FLIP_NONE;
        if (request.scaleX < 0)
        {
            flipMode |= SDL_FLIP_HORIZONTAL;
        }
        if (request.scaleY < 0)
        {
            flipMode |= SDL_FLIP_VERTICAL;
        }

        float xScale = std::abs(request.scaleX);
        float yScale = std::abs(request.scaleY);

        textureRect.w *= xScale;
        textureRect.h *= yScale;

        SDL_Point pivotPoint = { static_cast<int>(request.pivotX * textureRect.w), static_cast<int>(request.pivotY * textureRect.h) };

        glm::ivec2 camDimensions = Renderer::GetResolution();

        textureRect.x = static_cast<int>(finalRenderingPosition.x * pixelsPerUnit + camDimensions.x * 0.5f * (1.0f / zoomFactor) - pivotPoint.x);
        textureRect.y = static_cast<int>(finalRenderingPosition.y * pixelsPerUnit + camDimensions.y * 0.5f * (1.0f / zoomFactor) - pivotPoint.y);

        // culling
        if ((textureRect.x + textureRect.w < 0) ||
            (textureRect.y + textureRect.h < 0) ||
            (textureRect.x > (camDimensions.x / zoomFactor)) ||
            (textureRect.y > (camDimensions.y / zoomFactor)))
        {
            continue;
        }

        SDL_SetTextureColorMod(texture, request.r, request.g, request.b);
        SDL_SetTextureAlphaMod(texture, request.a);

        SDL_RenderCopyEx(Renderer::GetRenderer(), texture, NULL, &textureRect, request.rotationDegrees,
            &pivotPoint, static_cast<SDL_RendererFlip> (flipMode));

        SDL_RenderSetScale(Renderer::GetRenderer(), zoomFactor, zoomFactor);

        // Remove tint / alpha from texture
        SDL_SetTextureColorMod(texture, 255, 255, 255);
        SDL_SetTextureAlphaMod(texture, 255);
    }

    SDL_RenderSetScale(Renderer::GetRenderer(), 1, 1);
    renderRequests.clear();
}

void ImageDB::RenderUIImages()
{
    // UI Render Requests
    std::stable_sort(renderUIRequests.begin(), renderUIRequests.end(), CompareUIImageRequests);

    for (auto& request : renderUIRequests)
    {
        SDL_Texture* texture = LoadImage(request.imageName);

        // failed to load
        if (texture == nullptr)
        {
            continue;
        }

        SDL_Rect textureRect;

        SDL_QueryTexture(texture, NULL, NULL, &textureRect.w, &textureRect.h);

        textureRect.x = static_cast<int>(request.x);
        textureRect.y = static_cast<int>(request.y);

        SDL_SetTextureColorMod(texture, request.r, request.g, request.b);
        SDL_SetTextureAlphaMod(texture, request.a);

        SDL_RenderCopy(Renderer::GetRenderer(), texture, NULL, &textureRect);

        // Remove tint / alpha from texture
        SDL_SetTextureColorMod(texture, 255, 255, 255);
        SDL_SetTextureAlphaMod(texture, 255);
    }

    renderUIRequests.clear();
}

void ImageDB::RenderPixels()
{
    SDL_Renderer* renderer = Renderer::GetRenderer();
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    for (auto& request : renderPixelRequests)
    {   
        SDL_SetRenderDrawColor(renderer, request.r, request.g, request.b, request.a);

        SDL_RenderDrawPoint(Renderer::GetRenderer(), request.x, request.y);

        // Remove tint / alpha from texture
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    renderPixelRequests.clear();
}

void ImageDB::RenderLines()
{
    float zoomFactor = Renderer::GetZoomFactor();
    int pixelsPerUnit = Renderer::GetPixelsPerUnit();

    SDL_Renderer* renderer = Renderer::GetRenderer();
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    for (auto& request : renderLineRequests)
    {
        glm::ivec2 camDimensions = Renderer::GetResolution();

        glm::vec2 finalRenderingPosition1 = (glm::vec2(request.x1, request.y1) - Renderer::GetCameraPosition());
        glm::vec2 finalRenderingPosition2 = (glm::vec2(request.x2, request.y2) - Renderer::GetCameraPosition());

        int x1 = static_cast<int>(finalRenderingPosition1.x * pixelsPerUnit * zoomFactor + camDimensions.x * 0.5f);
        int y1 = static_cast<int>(finalRenderingPosition1.y * pixelsPerUnit * zoomFactor + camDimensions.y * 0.5f);

        int x2 = static_cast<int>(finalRenderingPosition2.x * pixelsPerUnit * zoomFactor + camDimensions.x * 0.5f);
        int y2 = static_cast<int>(finalRenderingPosition2.y * pixelsPerUnit * zoomFactor + camDimensions.y * 0.5f);


        // culling
        if ((x1 < 0 && x2 < 0) ||
            (y1 < 0 && y2 < 0) ||
            (x1 > (camDimensions.x / zoomFactor) && x2 > (camDimensions.x / zoomFactor)) ||
            (y1 > (camDimensions.y / zoomFactor) && y2 > (camDimensions.x / zoomFactor)))
        {
            continue;
        }

        SDL_SetRenderDrawColor(renderer, request.r, request.g, request.b, request.a);

        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);

        // Remove tint / alpha from texture
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    renderLineRequests.clear();
}

void ImageDB::Draw(const std::string& imageName, float x, float y)
{
    ImageRenderRequest request = ImageRenderRequest(imageName, x, y);
    renderRequests.push_back(request);
}

void ImageDB::DrawEx(const std::string& imageName, float x, float y,
    float rotationDegrees, float scaleX, float scaleY, float pivotX, float pivotY, float r, float g, float b, float a, float sortingOrder)
{
    ImageRenderRequest request = ImageRenderRequest(imageName, x, y, static_cast<int>(rotationDegrees), scaleX, scaleY, pivotX, pivotY,
        static_cast<int>(r), static_cast<int>(g), static_cast<int>(b), static_cast<int>(a), static_cast<int>(sortingOrder));
    renderRequests.push_back(request);
}

void ImageDB::DrawUI(const std::string& imageName, float x, float y)
{
    ImageUIRenderRequest request = ImageUIRenderRequest(imageName, x, y);
    renderUIRequests.push_back(request);
}

void ImageDB::DrawUIEx(const std::string& imageName, float x, float y, float r, float g, float b, float a, float sortingOrder)
{
    ImageUIRenderRequest request = ImageUIRenderRequest(imageName, x, y, static_cast<int>(r), static_cast<int>(g), static_cast<int>(b), static_cast<int>(a), static_cast<int>(sortingOrder));
    renderUIRequests.push_back(request);
}

void ImageDB::DrawPixel(float x, float y, float r, float g, float b, float a)
{
    PixelRenderRequest request = PixelRenderRequest(x, y, static_cast<int>(r), static_cast<int>(g), static_cast<int>(b), static_cast<int>(a));
    renderPixelRequests.push_back(request);
}

// BY SCENE COORDINATES!
void ImageDB::DrawLine(float x1, float y1, float x2, float y2, float r, float g, float b, float a)
{
    LineRenderRequest request = LineRenderRequest(x1, y1, x2, y2, static_cast<int>(r), static_cast<int>(g), static_cast<int>(b), static_cast<int>(a));
    renderLineRequests.push_back(request);
}

// EDITOR ONLY
void ImageDB::Reset()
{
    renderRequests.clear();
    renderUIRequests.clear();
    renderPixelRequests.clear();
    renderLineRequests.clear();
    cachedTextures.clear();
}

bool ImageDB::CompareImageRequests(const ImageRenderRequest& a, const ImageRenderRequest& b)
{
    return a.sortingOrder < b.sortingOrder;
}

bool ImageDB::CompareUIImageRequests(const ImageUIRenderRequest& a, const ImageUIRenderRequest& b)
{
    return a.sortingOrder < b.sortingOrder;
}