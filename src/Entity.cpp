#include "Entity.h"

bool Entity::CompareEntities(const Entity* a, const Entity* b)
{
	// If both have a render_order, compare them
	if (a->spriteRenderer->renderOrder.has_value() && b->spriteRenderer->renderOrder.has_value()) 
	{
		if (a->spriteRenderer->renderOrder.value() != b->spriteRenderer->renderOrder.value()) 
		{
			return a->spriteRenderer->renderOrder.value() < b->spriteRenderer->renderOrder.value();
		}
	}
	// If one has a render_order
	else if (a->spriteRenderer->renderOrder.has_value()) 
	{
		return a->spriteRenderer->renderOrder.value() < b->transform->position.y;
	}
	else if (b->spriteRenderer->renderOrder.has_value()) 
	{
		return a->transform->position.y < b->spriteRenderer->renderOrder.value();
	}

	// Otherwise, compare by transform_position_y
	if (a->transform->position.y != b->transform->position.y) 
	{
		return a->transform->position.y < b->transform->position.y;
	}

	// As a last resort, compare by entityID (to maintain consistent order for equal y positions)
	return a->entityID < b->entityID;
}

void SpriteRenderer::ChangeSprite(const std::string& viewImageName_in, glm::dvec2 pivot)
{
	viewImageName = viewImageName_in;

	if (!viewImageName.empty()) {
		viewImage = ImageDB::LoadImage(viewImageName);
	}

	// Calculate default pivot offset if not provided
	if (viewImage && (viewPivotOffset.x == -1 || viewPivotOffset.y == -1))
	{
		int w = 0, h = 0;
		SDL_QueryTexture(viewImage, nullptr, nullptr, &w, &h);

		if (viewPivotOffset.x == -1)
		{
			viewPivotOffset.x = w * 0.5;
		}

		if (viewPivotOffset.y == -1)
		{
			viewPivotOffset.y = h * 0.5;
		}
	}
}

void SpriteRenderer::RenderEntity(Entity* entity, SDL_Rect* cameraRect, int pixelsPerUnit, bool bounce)
{
	if (!entity->spriteRenderer->viewImage) return;  // Skip rendering if no view_image

	// Get image dimensions
	int imgWidth, imgHeight;
	SDL_QueryTexture(entity->spriteRenderer->viewImage, nullptr, nullptr, &imgWidth, &imgHeight);

	// Calculate pivot point (default to center)
	double pivotX = entity->spriteRenderer->viewPivotOffset.x;
	double pivotY = entity->spriteRenderer->viewPivotOffset.y;
	pivotX = std::round(pivotX);
	pivotY = std::round(pivotY);

	double zoomFactor = Renderer::GetZoomFactor();

	// Calculate destination rectangle
	SDL_Rect dstRect;
	dstRect.w = static_cast<int>(std::round(imgWidth * std::abs(entity->transform->scale.x)));
	dstRect.h = static_cast<int>(std::round(imgHeight * std::abs(entity->transform->scale.y)));
	dstRect.x = static_cast<int>(std::round(entity->transform->position.x * pixelsPerUnit - cameraRect->x - pivotX * std::abs(entity->transform->scale.x)));
	dstRect.y = static_cast<int>(std::round(entity->transform->position.y * pixelsPerUnit - cameraRect->y - pivotY * std::abs(entity->transform->scale.y)));

	if (movementBounce && bounce)
	{
		dstRect.y += -glm::abs(glm::sin(Helper::GetFrameNumber() * 0.15f)) * 10.0f;
	}

	// Determine flip
	SDL_RendererFlip flip = SDL_FLIP_NONE;
	// entity->transform->scale.x < 0 XOR flipSpriteVertically
	if (!(entity->transform->scale.x < 0) != !flipSpriteVertically) flip = (SDL_RendererFlip)(flip | SDL_FLIP_HORIZONTAL);
	if (entity->transform->scale.y < 0) flip = (SDL_RendererFlip)(flip | SDL_FLIP_VERTICAL);

	// Render with rotation around the pivot point
	SDL_Point pivot = { static_cast<int>(pivotX), static_cast<int>(pivotY) };

	if (showBackImage && entity->spriteRenderer->viewImageBack)
	{
		Helper::SDL_RenderCopyEx498(entity->entityID, entity->entityName, Renderer::GetRenderer(), entity->spriteRenderer->viewImageBack, nullptr, &dstRect, entity->transform->rotationDegrees, &pivot, flip);
	}
	else
	{
		Helper::SDL_RenderCopyEx498(entity->entityID, entity->entityName, Renderer::GetRenderer(), entity->spriteRenderer->viewImage, nullptr, &dstRect, entity->transform->rotationDegrees, &pivot, flip);
	}
	
}

Entity::~Entity()
{
	delete transform;
	delete spriteRenderer;
}