#ifndef GOLDFISH_LUA_H
#define GOLDFISH_LUA_H

#include <ecs/world.h>

void luaInit(struct world *world);
void luaEnd();

#endif
