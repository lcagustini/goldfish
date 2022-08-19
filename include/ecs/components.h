#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <math/vector.h>
#include <math/matrix.h>

#include <render/model.h>

#include <psp2/ctrl.h>

struct transformComponent {
    struct vec3 position;
    struct quat rotation;
    struct vec3 scale;

    union mat4 modelMatrix;
};

struct cameraComponent {
    union mat4 viewMat;
    union mat4 projectionMat;
};

struct controllerDataComponent {
    SceCtrlData data;
};

struct firstPersonComponent {
};

struct modelComponent {
    struct model model;
};

#endif
