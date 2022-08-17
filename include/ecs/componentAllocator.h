#ifndef COMPONENT_ALLOCATOR_H
#define COMPONENT_ALLOCATOR_H

#define MAX_COMPONENT 50

#include <ecs/components.h>

enum componentType {
    COMPONENT_TRANSFORM,

    COMPONENT_MAX
};

struct componentAllocator {
    struct transformComponent transformComponents[MAX_COMPONENT];
    unsigned int transformComponentsLength;
};

#endif
