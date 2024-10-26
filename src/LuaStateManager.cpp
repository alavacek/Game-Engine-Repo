// LuaStateManager.cpp
#include "LuaStateManager.h"

lua_State* LuaStateManager::luaState = nullptr;

void LuaStateManager::Initialize() {
    if (!luaState) {
        luaState = luaL_newstate();
        luaL_openlibs(luaState);
    }
}

void LuaStateManager::Close() {
    if (luaState) {
        lua_close(luaState);
        luaState = nullptr;
    }
}

lua_State* LuaStateManager::GetLuaState() {
    return luaState;
}

