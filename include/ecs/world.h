#ifndef WORLD_H
#define WORLD_H

#define MAX_ENTITY 50
#define MAX_SYSTEM 20
#define INVALID_ENTITY_ID (MAX_ENTITY + 1)

#include <ecs/system.h>
#include <ecs/componentAllocator.h>

#include <stdbool.h>

struct entity {
    int components[COMPONENT_MAX];
    unsigned int id;
};

struct world {
    struct entity entities[MAX_ENTITY];
    bool validEntities[MAX_ENTITY];

    struct componentAllocator componentAllocator;

    struct system systems[MAX_SYSTEM];
    unsigned int systemsLength;
};

unsigned int createEntity(struct world *world);

void addSystem(struct world *world, struct system system);

#endif
