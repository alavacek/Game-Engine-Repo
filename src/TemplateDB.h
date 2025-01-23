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
    static Template* FindTemplate(std::string templateName);
    static void LoadTemplates();

    // EDITOR ONLY
    static void Reset();
    
    static void ResetInstancesCountPerScene();

    // EDITOR ONLY
    static std::vector<std::string> GetBuiltInTemplates() { return builtInTemplates; }

private:
    static std::unordered_map<std::string, Template*> templates; // ones created by user

    // EDITOR ONLY
    static std::vector<std::string> builtInTemplates; // built in
};

#endif

