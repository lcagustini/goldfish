#ifndef GOLDFISH_LUA_INTERNAL_H
#define GOLDFISH_LUA_INTERNAL_H

#include <scripting/lua.h>

#include <lauxlib.h>

extern lua_State *luaState;
extern struct world *activeWorld;

void luaFatal(const char *what, const char *detail);
char *persistentString(const char *source);
const char *errorTraceback(lua_State *L);

void luaSystemCallback(struct systemRunData data);
void (*findNativeSystem(const char *name))(struct systemRunData);

extern const luaL_Reg luaBindings[];

#endif
