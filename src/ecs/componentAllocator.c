#include <stdlib.h>
#include <assert.h>

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

void *getComponent(struct world *world, unsigned int id, enum componentType component) {
    unsigned int componentId = world->entities[id].components[component];

    if (componentId == INVALID_COMPONENT_ID) return NULL;

    switch (component) {
        case COMPONENT_TRANSFORM:
            return &world->componentAllocator.transformComponents[componentId];
        case COMPONENT_CAMERA:
            return &world->componentAllocator.cameraComponents[componentId];
        case COMPONENT_CONTROLLER_DATA:
            return &world->componentAllocator.controllerDataComponents[componentId];
        case COMPONENT_FIRST_PERSON:
            return &world->componentAllocator.firstPersonComponents[componentId];
        case COMPONENT_MODEL:
            return &world->componentAllocator.modelComponents[componentId];
        case COMPONENT_DIR_LIGHT:
            return &world->componentAllocator.dirLightComponents[componentId];
        case COMPONENT_POINT_LIGHT:
            return &world->componentAllocator.pointLightComponents[componentId];
        case COMPONENT_SPOT_LIGHT:
            return &world->componentAllocator.spotLightComponents[componentId];
        default:
            assert(false);
            break;
    }
}
