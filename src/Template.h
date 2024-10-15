#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <string>

#include "Entity.h"
#include "glm/glm.hpp"

class Template : public Entity  // Make the inheritance public
{
public:
    Template(std::string templateName, std::string entityName, glm::vec2 initialVelocity,
        std::string nearbyDialogue, std::string contactDialogue,
        Transform* transformIn, SpriteRenderer* spriteIn, Collider* colliderIn)
        : Entity(entityName, initialVelocity, nearbyDialogue, contactDialogue, transformIn, spriteIn, colliderIn),
        templateName(templateName) {}

    Template() {}

    std::string templateName;
};

#endif