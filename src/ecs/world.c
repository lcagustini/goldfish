#include <ecs/world.h>

#include <string.h>

#define membersize(type, member) sizeof(((type *)0)->member)

unsigned int createEntity(struct world *world) {
    for (int i = 0; i < MAX_ENTITY; i++) {
        if (!world->validEntities[i]) {
            world->entities[i].id = i;
            for (int j = 0; j < COMPONENT_MAX; j++) world->entities[i].components[j] = INVALID_COMPONENT_ID;

            world->validEntities[i] = true;

            for (int i = 0; i < world->systemsLength; i++) {
                if (world->systems[i].phase == SYSTEM_ON_CREATE) {
                    world->systems[i].callback(i);
                }
            }

            return i;
        }
    }

    return INVALID_ENTITY_ID;
}

void deleteEntity(struct world *world, unsigned int id) {
    for (int i = 0; i < world->systemsLength; i++) {
        if (world->systems[i].phase == SYSTEM_ON_DELETE) {
            world->systems[i].callback(id);
        }
    }

    world->validEntities[id] = false;
}

void addSystem(struct world *world, struct system system) {
    world->systems[world->systemsLength] = system;
    world->systemsLength++;

    for (int i = 1; i < world->systemsLength; i++) {
        struct system x = world->systems[i];
        int j = i - 1;

        while (j >= 0 && world->systems[j].priority > x.priority) {
            world->systems[j + 1] = world->systems[j];
            j = j - 1;
        }

        world->systems[j + 1] = x;
    }
}

void runWorldPhase(struct world *world, enum systemPhase phase) {
    for (int i = 0; i < world->systemsLength; i++) {
        if (world->systems[i].phase == phase) {
            world->systems[i].callback(INVALID_ENTITY_ID);
        }
    }
}
