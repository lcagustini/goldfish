#ifndef COMPONENT_ALLOCATOR_H
#define COMPONENT_ALLOCATOR_H

#define MAX_COMPONENT_COUNT 50

#include <ecs/components.h>

#include <stdbool.h>

struct world;

enum componentType {
    COMPONENT_TRANSFORM,
    COMPONENT_CAMERA,
    COMPONENT_CONTROLLER_DATA,
    COMPONENT_FIRST_PERSON,
    COMPONENT_MODEL,

    COMPONENT_MAX
};

struct componentAllocator {
    struct transformComponent transformComponents[MAX_COMPONENT_COUNT];
    bool validTransformComponents[MAX_COMPONENT_COUNT];

    struct cameraComponent cameraComponents[MAX_COMPONENT_COUNT];
    bool validCameraComponents[MAX_COMPONENT_COUNT];

    struct controllerDataComponent controllerDataComponents[MAX_COMPONENT_COUNT];
    bool validControllerDataComponents[MAX_COMPONENT_COUNT];

    struct firstPersonComponent firstPersonComponents[MAX_COMPONENT_COUNT];
    bool validFirstPersonComponents[MAX_COMPONENT_COUNT];

    struct modelComponent modelComponents[MAX_COMPONENT_COUNT];
    bool validModelComponents[MAX_COMPONENT_COUNT];
};

unsigned int *getEntitiesWithComponents(struct world *world, enum componentType components[], unsigned int componentsLength);

#endif
