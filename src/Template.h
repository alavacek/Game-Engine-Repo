#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <string>

#include "Entity.h"
#include "glm/glm.hpp"

class Template : public Entity  // Make the inheritance public
{
public:
    Template(const std::string& templateName, const std::string& entityName, const std::unordered_map<std::string, Component*>& components)
        : Entity(entityName, components),
        templateName(templateName) {}

    std::string templateName;
    int instanceCountInScene = 0; // for naming
};

#endif