#include <stdlib.h>

#include <ecs/componentAllocator.h>
#include <ecs/world.h>

unsigned int *getEntitiesWithComponents(struct world *world, enum componentType components[], unsigned int componentsLength) {
    unsigned int *entitiesFound = malloc((MAX_ENTITY_COUNT + 1) * sizeof(unsigned int));
    entitiesFound[0] = 1;

    for (int i = 0; i < MAX_ENTITY_COUNT; i++) {
        if (!world->validEntities[i]) continue;

        bool passCheck = true;
        for (int j = 0; j < componentsLength; j++) {
            if (world->entities[i].components[components[j]] == INVALID_COMPONENT_ID) {
                passCheck = false;
                break;
            }
        }

        if (passCheck) {
            entitiesFound[entitiesFound[0]++] = i;
        }
    }

    return entitiesFound;
}
