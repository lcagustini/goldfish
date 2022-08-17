#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <math/vector.h>
#include <math/matrix.h>

struct transformComponent {
    struct vec3 position;
    struct quat rotation;
    struct vec3 scale;

    union mat4 modelMatrix;
};

#endif
