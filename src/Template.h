#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <string>

#include "Entity.h"
#include "glm/glm.hpp"

class Template : public Entity  // Make the inheritance public
{
public:
    Template(std::string templateName, std::string entity_name, char view, glm::vec2 initial_velocity,
        bool blocking, std::string nearby_dialogue, std::string contact_dialogue,
        Transform* transform_in, SpriteRenderer* sprite_in)
        : Entity(entity_name, view, initial_velocity, blocking, nearby_dialogue, contact_dialogue, transform_in, sprite_in), 
        templateName(templateName) {}

    Template() {}

    std::string templateName;
};

#endif