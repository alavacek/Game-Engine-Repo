#include "Renderer.h"

SDL_Window* Renderer::window;
SDL_Renderer* Renderer::renderer;

glm::ivec2 Renderer::resolution;
glm::vec2 Renderer::cameraPosition;

float Renderer::zoomFactor;
float Renderer::originalZoomFactor;


int Renderer::pixelsPerUnit = 100;

void Renderer::RendererInit()
{
	// check for resource folder
	if (!std::filesystem::exists("resources"))
	{
		std::cout << "error: resources/ missing";
		exit(0);
	}
	else if (!std::filesystem::exists("resources/game.config"))
	{
		std::cout << "error: resources/game.config missing";
		exit(0);
	}

	std::string configPath = "resources/game.config";
	rapidjson::Document configDocument;

	EngineUtils::ReadJsonFile(configPath, configDocument);
	std::string windowName = configDocument.HasMember("game_title") && configDocument["game_title"].IsString() ? configDocument["game_title"].GetString() : "";

	// rendering config
	std::string renderingConfig = "resources/rendering.config";
	uint32_t xResolution = 640;
	uint32_t yResolution = 360;

	// set default values
	zoomFactor = 1;
	cameraPosition = glm::vec2(0, 0);

	if (std::filesystem::exists(renderingConfig))
	{
		rapidjson::Document renderingConfigDoc;
		EngineUtils::ReadJsonFile(renderingConfig, renderingConfigDoc);

		if (renderingConfigDoc.HasMember("x_resolution") && renderingConfigDoc["x_resolution"].IsInt())
		{
			xResolution = renderingConfigDoc["x_resolution"].GetInt();
		}

		if (renderingConfigDoc.HasMember("y_resolution") && renderingConfigDoc["y_resolution"].IsInt())
		{
			yResolution = renderingConfigDoc["y_resolution"].GetInt();
		}

		if (renderingConfigDoc.HasMember("zoom_factor") && renderingConfigDoc["zoom_factor"].IsDouble())
		{
			zoomFactor = renderingConfigDoc["zoom_factor"].GetDouble();
		}
		
		originalZoomFactor = zoomFactor;
	}

	window = Helper::SDL_CreateWindow498(windowName.c_str(), 100, 100, xResolution, yResolution, SDL_WINDOW_SHOWN);

	resolution = glm::ivec2(xResolution, yResolution);

	renderer = Helper::SDL_CreateRenderer498(window, -1, 0);
}

void Renderer::RendererResetDefaults()
{
	zoomFactor = originalZoomFactor;
	cameraPosition = glm::vec2(0, 0);
}





