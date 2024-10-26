// LuaStateManager.h
#ifndef LUASTATEMANAGER_H
#define LUASTATEMANAGER_H

#include <lua.hpp>

class LuaStateManager {
public:
    static void Initialize();
    static void Close();
    static lua_State* GetLuaState();

private:
    static lua_State* luaState;
};

#endif // LUASTATEMANAGER_H

