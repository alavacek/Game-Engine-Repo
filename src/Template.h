#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <string>

#include "Entity.h"
#include "glm/glm.hpp"

class Template : public Entity  // Make the inheritance public
{
public:
    Template(const std::string& templateName, const std::string& entityName, const std::unordered_map<std::string, std::shared_ptr<luabridge::LuaRef>>& components)
        : Entity(entityName, components),
        templateName(templateName) {}

    Template() {}

    std::string templateName;
};

#endif