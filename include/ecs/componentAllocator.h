#ifndef COMPONENT_ALLOCATOR_H
#define COMPONENT_ALLOCATOR_H

#define MAX_COMPONENT_COUNT 50

#include <ecs/components.h>

#include <stdbool.h>

struct world;

enum componentType {
    COMPONENT_TRANSFORM,
    COMPONENT_CAMERA,

    COMPONENT_MAX
};

struct componentAllocator {
    struct transformComponent transformComponents[MAX_COMPONENT_COUNT];
    bool validTransformComponents[MAX_COMPONENT_COUNT];

    struct cameraComponent cameraComponents[MAX_COMPONENT_COUNT];
    bool validCameraComponents[MAX_COMPONENT_COUNT];
};

unsigned int *getEntitiesWithComponents(struct world *world, enum componentType components[], unsigned int componentsLength);

#endif
