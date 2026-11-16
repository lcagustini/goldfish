#include <scripting/luaInternal.h>

#include <lauxlib.h>
#include <lualib.h>

#include <goldfish/print.h>

#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define NATIVE_CDEFS_FILE "build/generated/nativeCdefs.lua"
#define ENGINE_LUA_FILE "goldfish/assets/engine.lua"
#define SCRIPTS_DIR "assets/scripts"

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

static void runScriptsDirectory(lua_State *L) {
    DIR *directory = opendir(SCRIPTS_DIR);
    if (directory == NULL) {
        luaFatal("scripts dir missing", SCRIPTS_DIR);
    }

    // Load scripts sorted: alphabetical order decides which declarations run first.
    int fileCount = 0;
    char (*names)[256] = NULL;
    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL) {
        size_t length = strlen(entry->d_name);
        if (length < 5 || strcmp(entry->d_name + length - 4, ".lua") != 0) continue;

        char (*next)[256] = realloc(names, (fileCount + 1) * sizeof(*names));
        if (next == NULL) luaFatal("out of memory", "script name list");
        names = next;
        snprintf(names[fileCount], sizeof(*names), "%s", entry->d_name);
        fileCount++;
    }
    closedir(directory);

    for (int i = 0; i + 1 < fileCount; i++) {
        for (int j = i + 1; j < fileCount; j++) {
            if (strcmp(names[i], names[j]) > 0) {
                char temp[256];
                memcpy(temp, names[i], sizeof(temp));
                memcpy(names[i], names[j], sizeof(temp));
                memcpy(names[j], temp, sizeof(temp));
            }
        }
    }

    for (int i = 0; i < fileCount; i++) {
        char path[512];
        snprintf(path, sizeof(path), SCRIPTS_DIR "/%s", names[i]);
        runScript(L, path);
    }
    free(names);
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

    runScriptsDirectory(luaState);

    runSweep(luaState);
}

void luaEnd() {
    if (luaState != NULL) {
        lua_close(luaState);
        luaState = NULL;
    }
    activeWorld = NULL;
}
