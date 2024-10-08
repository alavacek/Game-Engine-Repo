#include "Engine.h"

int Engine::pixelsPerUnit = 100;

void Engine::GameLoop()
{
	Start();

	while (isRunning)
	{
		Input();

		Update();

		Render();

		SDL_Delay(16); //time for each frame
	}
}

void Engine::ReadResources() 
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

	configPath = "resources/game.config";
	EngineUtils::ReadJsonFile(configPath, configDocument);

	// rendering config
	std::string renderingConfig = "resources/rendering.config";

	// template config
	TemplateDB::LoadTemplates();

	// scene config
	if (configDocument.HasMember("initial_scene") && configDocument["initial_scene"].IsString())
	{
		std::string sceneName = configDocument["initial_scene"].GetString();

		currScene = new SceneDB();
		currScene->LoadScene(sceneName);
	}
	else
	{
		// no initial scene specified
		std::cout << "error: initial_scene unspecified";
		exit(0);
	}

	if (currScene->GetPlayerEntity() != nullptr)
	{
		if (configDocument.HasMember("hp_image") && configDocument["hp_image"].IsString())
		{
			hpImage = ImageDB::LoadImage(configDocument["hp_image"].GetString());
		}
		else
		{
			std::cout << "error: player actor requires an hp_image be defined";
			exit(0);
		}
	}
}

void Engine::Start()
{
	// set up renderer
	Renderer::RendererInit();
	renderer = Renderer::GetRenderer();

	ReadResources();

	std::string game_start_message = configDocument["game_start_message"].GetString();
	std::cout << game_start_message << "\n";

	// Add Player Info
	playerHealth = 3;
	playerScore = 0;

	// set up audio
	AudioDB::InitAudio();

	// load intro screen images
	bool hasIntroImages = ImageDB::LoadIntroImages(configDocument);

	if (hasIntroImages)
	{
		// set up text database
		if (configDocument.HasMember("font") && configDocument["font"].IsString())
		{
			TextDB::TextDB_Init(configDocument["font"].GetString());
		}
		else
		{
			std::cout << "error: text render failed. No font configured";
			exit(0);
		}

		TextDB::LoadIntroText(configDocument);
		AudioDB::LoadIntroAudio(configDocument);
	}
	else
	{
		state == INPROGRESS;
	}

	// set up camera
	cameraRect.w = Renderer::GetResolution().x;
	cameraRect.h = Renderer::GetResolution().y;
}

void Engine::Input()
{
	SDL_Event e;
	while (Helper::SDL_PollEvent498(&e)) // empty/consume all the events in the event queue
	{
		if (e.type == SDL_QUIT)
		{
			exit(1);
		}

		if (state == INTRO)
		{
			if ((e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) ||
				(e.type == SDL_KEYDOWN && (e.key.keysym.scancode == SDL_SCANCODE_SPACE || e.key.keysym.scancode == SDL_SCANCODE_RETURN)))
			{
				// Left-click to advance the image
				ImageDB::ProgressToNextIntroImage();
				TextDB::ProgressToNextIntroText();

				if (ImageDB::FinishedWithIntro() && TextDB::FinishedWithIntro())
				{
					state = INPROGRESS;
					AudioDB::EndIntroAudio();

					AudioDB::LoadGameplayAudio(configDocument);
				}
			}
		}
		else
		{
			if (e.type == SDL_KEYDOWN)
			{
				// Handle key presses as directional input or quit command
				switch (e.key.keysym.scancode)
				{
				case SDL_SCANCODE_N:
					userInput = "n";
					break;
				case SDL_SCANCODE_E:
					userInput = "e";
					break;
				case SDL_SCANCODE_S:
					userInput = "s";
					break;
				case SDL_SCANCODE_W:
					userInput = "w";
					break;
				}
			}
		}

	}
}

void Engine::Update()
{
	if (isRunning)
	{
		if (state != INPROGRESS)
		{
			return;
		}

		// move player according to input
		glm::ivec2 proposedPlayerPosition = currScene->GetPlayerEntity()->transform->position;
		
		if (currScene->GetPlayerEntity() != nullptr && userInput != "")
		{
			if (userInput == "n")
			{
				proposedPlayerPosition = currScene->GetPlayerEntity()->transform->position - glm::ivec2(0, 1);
			}
			else if (userInput == "e")
			{
				proposedPlayerPosition = currScene->GetPlayerEntity()->transform->position + glm::ivec2(1, 0);
			}
			else if (userInput == "s")
			{
				proposedPlayerPosition = currScene->GetPlayerEntity()->transform->position + glm::ivec2(0, 1);
			}
			else if (userInput == "w")
			{
				proposedPlayerPosition = currScene->GetPlayerEntity()->transform->position - glm::ivec2(1, 0);
			}
			userInput = "";
		}

		// Update Entities Position, iterate based on entityID
		for (int entityIndex = 0; entityIndex < currScene->GetNumberOfEntitiesInScene(); entityIndex++)
		{
			Entity* currEntity = currScene->GetEntityAtIndex(entityIndex);
			// NPC Movement	
			// only update position/velocity if they have a none zeroes velocity
			// update every 60 frames
			if (currEntity->entityName != "player" && Helper::GetFrameNumber() % 60 == 0 && currEntity->velocity != glm::ivec2(0, 0))
			{
				// Position and Movement
				glm::ivec2 proposedNPCMovement = currEntity->transform->position + currEntity->velocity;

				// is there a blocking actor at this location?
				bool blockingActorAtLocation = false;
				for (int i = 0; i < currScene->locationOfEntitiesInScene[proposedNPCMovement.x][proposedNPCMovement.y].size(); i++)
				{
					// does the current entity block?
					if (currScene->locationOfEntitiesInScene[proposedNPCMovement.x][proposedNPCMovement.y][i]->blocking)
					{
						blockingActorAtLocation = true;
						break;
					}
				}

				// ensure can move in that direction
				if (!blockingActorAtLocation)
				{
					currScene->ChangeEntityPosition(currEntity, proposedNPCMovement);
				}
				// direction was blocked, flip velocity
				else
				{
					currEntity->velocity *= -1;
				}
					
			}
			// Player Movement
			else if (currEntity->entityName == "player")
			{
				// is there a blocking actor at proposedPlayer location?
				bool blockingActorAtLocation = false;
				for (int i = 0; i < currScene->locationOfEntitiesInScene[proposedPlayerPosition.x][proposedPlayerPosition.y].size(); i++)
				{
					// does the current entity block?
					if (currScene->locationOfEntitiesInScene[proposedPlayerPosition.x][proposedPlayerPosition.y][i]->blocking)
					{
						blockingActorAtLocation = true;
						break;
					}
				}

				// ensure can move in that direction
				if (!blockingActorAtLocation)
				{
					currScene->ChangeEntityPosition(currEntity, proposedPlayerPosition);
				}
			}
		}

		if (inHealthCooldown)
		{
			if (Helper::GetFrameNumber() >= frameSinceDamageTaken + 180)
			{
				inHealthCooldown = false;
			}
		}


		// sort based on y positon or based on render order
		std::sort(currScene->entityRenderOrder.begin(), currScene->entityRenderOrder.end(), Entity::CompareEntities);
		
		// state can change in these next 2 lines
		DetermineDialoguesToPrint();

		if (playerHealth <= 0)
		{
			state = LOST;
		}

		//runs once 
		if (state == WON)
		{
			AudioHelper::Mix_HaltChannel498(0);
			if (configDocument.HasMember("game_over_good_audio") && configDocument["game_over_good_audio"].IsString())
			{
				AudioDB::PlayBGM(configDocument["game_over_good_audio"].GetString());
			}
		}
		else if (state == LOST)
		{
			AudioHelper::Mix_HaltChannel498(0);
			if (configDocument.HasMember("game_over_bad_audio") && configDocument["game_over_bad_audio"].IsString())
			{
				AudioDB::PlayBGM(configDocument["game_over_bad_audio"].GetString());
			}
		}
	}
}

void Engine::DetermineDialoguesToPrint()
{
	// sacrifices space for time
	entityDialoguesToPrint.clear();

	// Determine NPC Dialogue 
	DialogueType dialogueType = NONE;
	
	for (int xPos = std::max(0, currScene->GetPlayerEntity()->transform->position.x - 1); xPos <= currScene->GetPlayerEntity()->transform->position.x + 1; xPos++)
	{
		for (int yPos = std::max(0, currScene->GetPlayerEntity()->transform->position.y - 1); yPos <= currScene->GetPlayerEntity()->transform->position.y + 1; yPos++)
		{
			if (!currScene->locationOfEntitiesInScene[xPos].empty() && !currScene->locationOfEntitiesInScene[xPos][yPos].empty())
			{
				for (int entityIndex = 0; entityIndex < currScene->locationOfEntitiesInScene[xPos][yPos].size(); entityIndex++)
				{
					Entity* currEntity = currScene->locationOfEntitiesInScene[xPos][yPos][entityIndex];
					if (currEntity->entityName != "player")
					{
						// Dialogue
						// determine which dialogues to print and actions that will be taken
						dialogueType = NONE;

						// contact dialogue
						if (xPos == currScene->GetPlayerEntity()->transform->position.x && yPos == currScene->GetPlayerEntity()->transform->position.y)
						{
							//store dialogue commands
							dialogueType = currEntity->contactDialogueType;

							// if has proceed to command, dont render this dialogue
							if (currEntity->contactSceneToLoad != "")
							{
								pendingScene = currEntity->contactSceneToLoad;
							}
							else
							{
								entityDialoguesToPrint.push_back({ currEntity->entityID, currEntity->contactDialogue });
							}

							
						}
						// nearby dialogue
						else
						{
							//store dialogue commands
							dialogueType = currEntity->nearbyDialogueType;

							// if has proceed to command, dont render this dialogue
							if (currEntity->nearbySceneToLoad != "")
							{
								pendingScene = currEntity->nearbySceneToLoad;
							}
							else
							{
								entityDialoguesToPrint.push_back({ currEntity->entityID, currEntity->nearbyDialogue });
							}
						}

						//handle dialogue commands
						if (!inHealthCooldown && dialogueType == HEALTHDOWN)
						{
							playerHealth--;
							inHealthCooldown = true;
							frameSinceDamageTaken = Helper::GetFrameNumber();
						}
						else if (!currEntity->hasIncreasedScore && dialogueType == SCOREUP)
						{
							playerScore++;
							currEntity->hasIncreasedScore = true;
						}
						else if (dialogueType == YOUWIN)
						{
							state = WON;
						}
						else if (dialogueType == GAMEOVER)
						{
							//TODO: what if state was alr set to WON this frame?
							state = LOST;
						}
					}
				}
			}
		}
	}

	// sort based on entityID
	std::sort(entityDialoguesToPrint.begin(), entityDialoguesToPrint.end(), [](const std::pair<uint64_t, std::string>& a, const std::pair<uint64_t, std::string>& b) {
			return a.first < b.first;
	});

}

void Engine::Render()
{
	if (isRunning)
	{
		// TODO: May need to move this to begining of game loop
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);

		if (state == INTRO)
		{
			ImageDB::RenderCurrentIntroImage();
			TextDB::DrawCurrentIntroText(Renderer::GetResolution().y);
		}
		else if (state == INPROGRESS)
		{
			glm::ivec2 centerPos = currScene->GetPlayerEntity() != nullptr ? currScene->GetPlayerEntity()->transform->position : glm::ivec2(0, 0);
			glm::ivec2 resolution = Renderer::GetResolution();
			
			double zoomFactor = Renderer::GetZoomFactor();

			//Entity* player = currScene->GetPlayerEntity();
			cameraRect.x = static_cast<int>(std::round((centerPos.x * pixelsPerUnit) - (resolution.x / (2 * zoomFactor)) + Renderer::GetCameraOffset().x));
			cameraRect.y = static_cast<int>(std::round((centerPos.y * pixelsPerUnit) - (resolution.y / (2 * zoomFactor)) + Renderer::GetCameraOffset().y));

			//Render Map
			//int rowBoundMin = centerPos.y - ((resolution.y / pixelsPerUnit) / 2);
			//int rowBoundMax = centerPos.y + ((resolution.y / pixelsPerUnit) / 2);
			//int colBoundMin = centerPos.x - ((resolution.x / pixelsPerUnit) / 2);
			//int colBoundMax = centerPos.x + ((resolution.x / pixelsPerUnit) / 2);

			// render visible map
			// TODO: Culling
			SDL_RenderSetScale(renderer, zoomFactor, zoomFactor);

			for (Entity* entity : currScene->entityRenderOrder)
			{
				entity->spriteRenderer->RenderEntity(entity, &cameraRect, pixelsPerUnit);
			}
		
			// Reset for UI and alike
			SDL_RenderSetScale(renderer, 1.0, 1.0);

			//NPC Dialogue
			int numberOfDialoguesToShow = entityDialoguesToPrint.size();
			for (int dialogueIndex = 0; dialogueIndex < numberOfDialoguesToShow; dialogueIndex++)
			{
				int drawPositionY = resolution.y - 50 - (numberOfDialoguesToShow - 1 - dialogueIndex) * 50;
				TextDB::DrawText(entityDialoguesToPrint[dialogueIndex].second, 16, { 255, 255, 255, 255 }, 25, drawPositionY);
			}

			if (currScene->GetPlayerEntity() != nullptr)
			{
				int hpWidth, hpHeight;
				SDL_QueryTexture(hpImage, nullptr, nullptr, &hpWidth, &hpHeight);

				int startX = 5;
				int startY = 25;
				// hearts
				for (int i = 0; i < playerHealth; i++)
				{
					SDL_Rect dstRect;
					dstRect.x = startX + i * (hpWidth + 5);
					dstRect.y = startY;                     
					dstRect.w = hpWidth;                    
					dstRect.h = hpHeight;

					SDL_RenderCopy(renderer, hpImage, nullptr, &dstRect);
				}

				// score
				std::string scoreText = "score : " + std::to_string(playerScore);
				TextDB::DrawText(scoreText, 16, { 255, 255, 255, 255 }, 5, 5);

			}

			// possibly move to update?
			if (pendingScene != "")
			{
				delete currScene; // TODO: iss this appropriate or should I just load new scene?

				currScene = new SceneDB();
				currScene->LoadScene(pendingScene);
				pendingScene = "";

				Render();
			}
		}
		else if (state == WON)
		{
			// TODO: should this look up be cached?
			if (configDocument.HasMember("game_over_good_image") && configDocument["game_over_good_image"].IsString())
			{
				SDL_RenderCopy(Renderer::GetRenderer(), ImageDB::LoadImage(configDocument["game_over_good_image"].GetString()), NULL, NULL);
			}
			else
			{
				exit(1);
			}
		}
		else if (state == LOST)
		{
			// TODO: should this look up be cached?
			if (configDocument.HasMember("game_over_bad_image") && configDocument["game_over_bad_image"].IsString())
			{
				SDL_RenderCopy(Renderer::GetRenderer(), ImageDB::LoadImage(configDocument["game_over_bad_image"].GetString()), NULL, NULL);
			}
			else
			{
				exit(1);
			}
		}

		Helper::SDL_RenderPresent498(renderer);
	}
}

void Engine::EndGame()
{
	delete currScene;
}

Engine::~Engine()
{
	if (currScene != nullptr)
	{
		delete currScene;
	}
}

