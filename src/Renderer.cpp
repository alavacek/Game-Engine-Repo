#include "Renderer.h"

SDL_Window* Renderer::window;
SDL_Renderer* Renderer::renderer;
glm::ivec2 Renderer::resolution;
glm::ivec2 Renderer::cameraOffset = { 0,0 };
double Renderer::zoomFactor = 1;
float Renderer::cameraEaseFactor = 1;
bool Renderer::xScaleActorFlippingOnMovement = false;

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

		if (renderingConfigDoc.HasMember("cam_offset_x") && renderingConfigDoc["cam_offset_x"].IsInt())
		{
			cameraOffset.x = renderingConfigDoc["cam_offset_x"].GetInt();
		}

		if (renderingConfigDoc.HasMember("cam_offset_y") && renderingConfigDoc["cam_offset_y"].IsInt())
		{
			cameraOffset.y = renderingConfigDoc["cam_offset_y"].GetInt();
		}

		if (renderingConfigDoc.HasMember("zoom_factor") && renderingConfigDoc["zoom_factor"].IsDouble())
		{
			zoomFactor = renderingConfigDoc["zoom_factor"].GetDouble();
		}

		if (renderingConfigDoc.HasMember("zoom_factor") && renderingConfigDoc["zoom_factor"].IsDouble())
		{
			zoomFactor = renderingConfigDoc["zoom_factor"].GetDouble();
		}

		if (renderingConfigDoc.HasMember("cam_ease_factor") && renderingConfigDoc["cam_ease_factor"].IsFloat())
		{
			cameraEaseFactor = renderingConfigDoc["cam_ease_factor"].GetFloat();
		}

		if (renderingConfigDoc.HasMember("x_scale_actor_flipping_on_movement") && renderingConfigDoc["x_scale_actor_flipping_on_movement"].GetBool())
		{
			xScaleActorFlippingOnMovement = renderingConfigDoc["x_scale_actor_flipping_on_movement"].GetBool();
		}
	}

	window = Helper::SDL_CreateWindow498(windowName.c_str(), 100, 100, xResolution, yResolution, SDL_WINDOW_SHOWN);

	resolution = glm::ivec2(xResolution, yResolution);

	renderer = Helper::SDL_CreateRenderer498(window, -1, 0);
}

