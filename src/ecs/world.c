#include <ecs/world.h>

#include <string.h>

#define membersize(type, member) sizeof(((type *)0)->member)

unsigned int createEntity(struct world *world) {
    for (int i = 0; i < MAX_ENTITY; i++) {
        if (!world->validEntities[i]) {
            world->entities[i].id = i;
            memset(&world->entities[i].components, 0, membersize(struct entity, components));

            world->validEntities[i] = true;

            return i;
        }
    }

    return INVALID_ENTITY_ID;
}

void addSystem(struct world *world, struct system system) {
    world->systems[world->systemsLength] = system;
    world->systemsLength++;
}
