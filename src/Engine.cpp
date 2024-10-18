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

	if (configDocument.HasMember("player_movement_speed") && configDocument["player_movement_speed"].IsFloat())
	{
		playerSpeed = configDocument["player_movement_speed"].GetFloat();
	}
	else
	{
		playerSpeed = 0.02f;
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
	glm::ivec2 resolution = Renderer::GetResolution();
	double zoomFactor = Renderer::GetZoomFactor();

	cameraRect.w = static_cast<int>(std::round(resolution.x / zoomFactor));
	cameraRect.h = static_cast<int>(std::round(resolution.y / zoomFactor));

	glm::vec2 centerPos = currScene->GetPlayerEntity() != nullptr ? currScene->GetPlayerEntity()->transform->position : glm::vec2(0, 0);

	cameraRect.x = static_cast<int>(std::round((centerPos.x * pixelsPerUnit) - (cameraRect.w / 2) + Renderer::GetCameraOffset().x));
	cameraRect.y = static_cast<int>(std::round((centerPos.y * pixelsPerUnit) - (cameraRect.h / 2) + Renderer::GetCameraOffset().y));
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
			Input::ProcessEvent(e);

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
		glm::vec2 proposedPlayerPosition = currScene->GetPlayerEntity()->transform->position;
		glm::vec2 proposedPlayerMovement = glm::vec2(0, 0);
		bool actorFlippingOnMovement = Renderer::GetXScaleActorFlippingOnMovement();

		if (currScene->GetPlayerEntity() != nullptr)
		{
			if (Input::GetKey(SDL_SCANCODE_W) || Input::GetKey(SDL_SCANCODE_UP))
			{
				proposedPlayerMovement += glm::vec2(0, -1);
			}
			if (Input::GetKey(SDL_SCANCODE_D) || Input::GetKey(SDL_SCANCODE_RIGHT))
			{
				proposedPlayerMovement += glm::vec2(1, 0);
			}
			if (Input::GetKey(SDL_SCANCODE_S) || Input::GetKey(SDL_SCANCODE_DOWN))
			{
				proposedPlayerMovement += glm::vec2(0, 1);
			}
			if (Input::GetKey(SDL_SCANCODE_A) || Input::GetKey(SDL_SCANCODE_LEFT))
			{
				proposedPlayerMovement += glm::vec2(-1, 0);
			}

			//normalize if necessary
			if (proposedPlayerMovement != glm::vec2(0, 0))
			{
				proposedPlayerMovement = glm::normalize(proposedPlayerMovement) * playerSpeed;
			}

			proposedPlayerPosition += proposedPlayerMovement;
		}

		// Update Entities Position, iterate based on entityID
		// Determine Collisions and Movement Updates
		for (int entityIndex = 0; entityIndex < currScene->GetNumberOfEntitiesInScene(); entityIndex++)
		{
			Entity* currEntity = currScene->GetEntityAtIndex(entityIndex);

			// Attack and Damage Image updates
			// Attack
			if (currEntity->framesLeftOfAttackIndicator > 0)
			{
				currEntity->framesLeftOfAttackIndicator--;
			}
			// Damage
			if (currEntity->framesLeftOfDamageIndicator > 0)
			{
				currEntity->framesLeftOfDamageIndicator--;
			}

			if (currEntity->collider != nullptr)
			{
				// Reset collision structure
				currEntity->collider->collidingEntitiesThisFrame.clear();
			}

			// NPC Movement	
			// only update position/velocity if they have a none zeroes velocity
			// update every 60 frames
			if (currEntity->entityName != "player" && currEntity->velocity != glm::vec2(0.0f, 0.0f))
			{
				// Position and Movement
				glm::vec2 proposedNPCPosition = currEntity->transform->position + currEntity->velocity;

				// is there a blocking actor at this location?
				// ensure can move in that direction
				if (currScene->CanMoveEntityToPosition(currEntity, proposedNPCPosition))
				{
					currScene->ChangeEntityPosition(currEntity, proposedNPCPosition);
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
				if (proposedPlayerMovement != glm::vec2(0, 0))
				{
					// glm::ivec2 proposedHashedBucket = currScene->HashPositionToBucket(proposedPlayerPosition);
					// is there a blocking actor at proposedPlayer location?
					if (currScene->CanMoveEntityToPosition(currEntity, proposedPlayerPosition))
					{
						currScene->ChangeEntityPosition(currEntity, proposedPlayerPosition);
						currEntity->velocity = proposedPlayerMovement;

						// Play walking audio if 20th frame
						if (Helper::GetFrameNumber() % 20 == 0)
						{
							// TODO: is accessing configDoc this much expensive? Assess w profilers
							if (configDocument.HasMember("step_sfx") && configDocument["step_sfx"].IsString())
							{
								AudioDB::PlayChannel(Helper::GetFrameNumber() % 48 + 2, configDocument["step_sfx"].GetString(), false);
							}
						}
					}
				}
				else
				{
					currEntity->velocity = glm::vec2(0, 0);
				}	
			}

			if (actorFlippingOnMovement)
			{
				if (currEntity->velocity.x < 0)
				{
					currEntity->spriteRenderer->flipSpriteVertically = true;
				}
				else if (currEntity->velocity.x > 0)
				{
					currEntity->spriteRenderer->flipSpriteVertically = false;
				}

				if (currEntity->velocity.y < 0)
				{
					currEntity->spriteRenderer->showBackImage = true;
				}
				else if (currEntity->velocity.y > 0)
				{
					currEntity->spriteRenderer->showBackImage = false;
				}
			}
		}

		// Determine Trigger Collider updates
		for (int entityIndex = 0; entityIndex < currScene->GetNumberOfEntitiesInScene(); entityIndex++)
		{
			Entity* currEntity = currScene->GetEntityAtIndex(entityIndex);

			if (currEntity->triggerCollider != nullptr)
			{
				// Reset trigger collision structure
				currEntity->triggerCollider->triggeringEntitiesThisFrame.clear();

				currScene->DetermineEntityTriggerCollisions(currEntity);
			}
		}

		// Determine if still in health cooldown
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

		// Late Update
		Input::LateUpdate();
	}
}

void Engine::DetermineDialoguesToPrint()
{
	Entity* player = currScene->GetPlayerEntity();

	if (player == nullptr)
	{
		return;
	}

	// sacrifices space for time
	entityDialoguesToPrint.clear();

	// Determine NPC Dialogue 
	DialogueType dialogueType = NONE;
	
	// Collisions
	if (player->collider)
	{
		// determine which dialogues to print and actions that will be taken
		for (const auto& currEntity : player->collider->collidingEntitiesThisFrame)
		{
			if (currEntity->contactDialogue == "")
			{
				continue;
			}

			dialogueType = NONE;

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

			HandleDialogueCommands(dialogueType, currEntity);




		}
	}
	
	if (player->triggerCollider)
	{
		for (const auto& currEntity : player->triggerCollider->triggeringEntitiesThisFrame)
		{
			if (currEntity->nearbyDialogue == "")
			{
				continue;
			}

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

			// handle first audio output for nearby 
			if (!currEntity->hasTriggeredNearbyDialogue)
			{
				currEntity->hasTriggeredNearbyDialogue = true;
				// Play score audio
				if (configDocument.HasMember("nearby_dialogue_sfx") && configDocument["nearby_dialogue_sfx"].IsString())
				{
					AudioDB::PlayChannel(Helper::GetFrameNumber() % 48 + 2, configDocument["nearby_dialogue_sfx"].GetString(), false);
				}
			}

			HandleDialogueCommands(dialogueType, currEntity);
		}
	}

	// sort based on entityID
	std::sort(entityDialoguesToPrint.begin(), entityDialoguesToPrint.end(), [](const std::pair<uint64_t, std::string>& a, const std::pair<uint64_t, std::string>& b) {
			return a.first < b.first;
	});

}

void Engine::HandleDialogueCommands(DialogueType type, Entity* speakingEntity)
{
	//handle dialogue commands
	if (!inHealthCooldown && type == HEALTHDOWN)
	{
		playerHealth--;
		inHealthCooldown = true;
		frameSinceDamageTaken = Helper::GetFrameNumber();

		// Set up for visual indicator
		if (currScene->GetPlayerEntity()->spriteRenderer && currScene->GetPlayerEntity()->spriteRenderer->viewImageDamage)
		{
			currScene->GetPlayerEntity()->framesLeftOfDamageIndicator = framesOfDamageIndicator;
		}

		if (speakingEntity->spriteRenderer && speakingEntity->spriteRenderer->viewImageAttack)
		{
			speakingEntity->framesLeftOfAttackIndicator = framesOfAttackIndicator;
		}

		// Play damage audio
		if (configDocument.HasMember("damage_sfx") && configDocument["damage_sfx"].IsString())
		{
			// the +2 prevents us from clobbering channel 0 or 1
			AudioDB::PlayChannel(Helper::GetFrameNumber() % 48 + 2, configDocument["damage_sfx"].GetString(), false);
		}
	}
	else if (!speakingEntity->hasIncreasedScore && type == SCOREUP)
	{
		playerScore++;
		speakingEntity->hasIncreasedScore = true;

		// Play score audio
		if (configDocument.HasMember("score_sfx") && configDocument["score_sfx"].IsString())
		{
			AudioDB::PlayChannel(1, configDocument["score_sfx"].GetString(), false);
		}
	}
	else if (type == YOUWIN)
	{
		state = WON;
	}
	else if (type == GAMEOVER)
	{
		//TODO: what if state was alr set to WON this frame?
		state = LOST;
	}
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
			glm::vec2 centerPos = currScene->GetPlayerEntity() != nullptr ? currScene->GetPlayerEntity()->transform->position : glm::vec2(0, 0);
			glm::ivec2 resolution = Renderer::GetResolution();
			
			double zoomFactor = Renderer::GetZoomFactor();

			cameraRect.x = glm::mix(cameraRect.x, static_cast<int>(std::round((centerPos.x * pixelsPerUnit) - (cameraRect.w / 2) + Renderer::GetCameraOffset().x)), Renderer::GetCameraEaseFactor());
			cameraRect.y = glm::mix(cameraRect.y, static_cast<int>(std::round((centerPos.y * pixelsPerUnit) - (cameraRect.h / 2) + Renderer::GetCameraOffset().y)), Renderer::GetCameraEaseFactor());

			//Render Map
			//int rowBoundMin = centerPos.y - ((resolution.y / pixelsPerUnit) / 2);
			//int rowBoundMax = centerPos.y + ((resolution.y / pixelsPerUnit) / 2);
			//int colBoundMin = centerPos.x - ((resolution.x / pixelsPerUnit) / 2);
			//int colBoundMax = centerPos.x + ((resolution.x / pixelsPerUnit) / 2);

			// render visible map
			SDL_RenderSetScale(renderer, zoomFactor, zoomFactor);
			
			for (Entity* entity : currScene->entityRenderOrder)
			{
				entity->spriteRenderer->RenderEntity(entity, &cameraRect, pixelsPerUnit, entity->velocity != glm::vec2(0,0), debugShowCollisions);
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

