#ifndef WORLD_H
#define WORLD_H

#define MAX_ENTITY_COUNT 50
#define MAX_SYSTEM_COUNT 20

#define INVALID_ENTITY_ID (MAX_ENTITY_COUNT + 1)
#define INVALID_COMPONENT_ID (MAX_COMPONENT_COUNT + 1)

#include <ecs/system.h>
#include <ecs/componentAllocator.h>

#include <stdbool.h>

struct entity {
    unsigned int components[COMPONENT_MAX];
    unsigned int id;
};

struct world {
    struct entity entities[MAX_ENTITY_COUNT];
    bool validEntities[MAX_ENTITY_COUNT];

    struct componentAllocator componentAllocator;

    struct system systems[MAX_SYSTEM_COUNT];
    unsigned int systemsLength;
};

unsigned int createEntity(struct world *world);
void deleteEntity(struct world *world, unsigned int id);

void addSystem(struct world *world, struct system system);
void addComponent(struct world *world, unsigned int entity, enum componentType component);

void runWorldPhase(struct world *world, enum systemPhase phase);

#endif
