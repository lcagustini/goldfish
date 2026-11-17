#include <scripting/luaInternal.h>

#include <lauxlib.h>
#include <lualib.h>

#include <goldfish/print.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define NATIVE_CDEFS_FILE "build/generated/nativeCdefs.lua"
#define ENGINE_LUA_FILE GOLDFISH_ENGINE_ASSETS "/engine.lua"
#define GAME_MANIFEST_FILE "build/generated/gameManifest.lua"

lua_State *luaState;
struct world *activeWorld;

static void runScript(lua_State *L, const char *path) {
    if (luaL_loadfile(L, path) != LUA_OK) {
        print("[Lua script error] %s:\n%s\n", path, errorTraceback(L));
        lua_pop(L, 1);
        return;
    }
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        print("[Lua script error] %s:\n%s\n", path, errorTraceback(L));
        lua_pop(L, 1);
        // The next file still loads; the engine keeps running.
    }
}

static void runSweep(lua_State *L) {
    lua_getglobal(L, "engine");
    lua_getfield(L, -1, "sweep");
    if (!lua_isfunction(L, -1)) {
        luaFatal("engine.lua missing sweep", ENGINE_LUA_FILE);
    }
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        luaFatal("cdef size sweep failed", errorTraceback(L));
    }
    lua_pop(L, 1); // engine table
}

// The manifest is the whole game map, written by tools/genManifest.lua at configure time:
// components to declare, system scripts to run, and the scene list. Only the first
// scene starts; the rest are entry points for engine.loadSceneFile.
static void runManifest(lua_State *L) {
    if (luaL_loadfile(L, GAME_MANIFEST_FILE) != LUA_OK) {
        luaFatal("game manifest missing or invalid", errorTraceback(L));
    }
    if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
        luaFatal("game manifest failed to run", errorTraceback(L));
    }
    if (lua_type(L, -1) != LUA_TTABLE) {
        lua_pop(L, 1);
        luaFatal("game manifest must return a table", GAME_MANIFEST_FILE);
    }
    int manifest = lua_gettop(L);

    lua_getglobal(L, "engine");
    int engineTable = lua_gettop(L);

    // Every Lua-only component exists before any game code runs.
    lua_getfield(L, engineTable, "declareComponents");
    lua_getfield(L, manifest, "components");
    if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
        luaFatal("component declaration failed", errorTraceback(L));
    }

    // System scripts keep the old directory scan's soft-fail behavior.
    lua_getfield(L, manifest, "systems");
    int systemCount = (int)lua_objlen(L, -1);
    for (int i = 1; i <= systemCount; i++) {
        lua_rawgeti(L, -1, i);
        runScript(L, lua_tostring(L, -1));
        lua_pop(L, 1);
    }
    lua_pop(L, 1); // systems

    lua_getfield(L, manifest, "scenes");
    lua_rawgeti(L, -1, 1);
    if (lua_isstring(L, -1)) {
        lua_getfield(L, engineTable, "loadSceneFile");
        lua_pushvalue(L, -2);
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            print("[Lua scene error] %s\n", errorTraceback(L));
        }
    }
    lua_pop(L, 4); // scene path, scenes table, engine table, manifest
}

void luaInit(struct world *world) {
    activeWorld = world;

    luaState = luaL_newstate();
    if (luaState == NULL) {
        luaFatal("startup", "luaL_newstate failed");
    }
    luaL_openlibs(luaState);

    // The engine binding table exists before engine.lua extends it.
    lua_newtable(luaState);
    luaL_setfuncs(luaState, luaBindings, 0);
    lua_setglobal(luaState, "engine");

    // Native cdef text first, before any script can touch engine memory.
    runScript(luaState, NATIVE_CDEFS_FILE);
    // No fatal check here: the sweep below catches a bad generated file.

    runScript(luaState, ENGINE_LUA_FILE);
    lua_getglobal(luaState, "engine");
    lua_getfield(luaState, -1, "view");
    if (!lua_isfunction(luaState, -1)) {
        luaFatal("engine.lua failed to define view", ENGINE_LUA_FILE);
    }
    lua_pop(luaState, 2);

    runManifest(luaState);

    runSweep(luaState);
}

void luaEnd() {
    if (luaState != NULL) {
        lua_close(luaState);
        luaState = NULL;
    }
    activeWorld = NULL;
}
