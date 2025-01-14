#ifndef TEMPLATEDB_H
#define TEMPLATEDB_H

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>

#include "ComponentDB.h"
#include "EngineUtils.h"
#include "glm/glm.hpp"
#include "lua.hpp"
#include "LuaBridge.h"
#include "LuaStateManager.h"
#include "Template.h"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"

class TemplateDB
{
public:
    static Template* FindEntity(std::string templateName);
    static void LoadTemplates();
    static void Reset();

private:
    static std::unordered_map<std::string, Template*> templates;
};

#endif

