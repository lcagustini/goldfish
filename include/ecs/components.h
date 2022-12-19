#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <stdbool.h>

#include <math/vector.h>
#include <math/matrix.h>

#include <render/model.h>

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
    unsigned char lx;
    unsigned char ly;

    unsigned char rx;
    unsigned char ry;

    bool up;
    bool right;
    bool down;
    bool left;

    bool lt;
    bool rt;

    bool lb;
    bool rb;

    bool A;
    bool B;
    bool X;
    bool Y;
};

struct firstPersonComponent {
};

struct modelComponent {
    struct model model;
};

struct dirLightComponent {
    struct vec3 ambientColor;
    struct vec3 diffuseColor;
    struct vec3 specularColor;
};

struct pointLightComponent {
    struct vec3 attenuation;

    struct vec3 ambientColor;
    struct vec3 diffuseColor;
    struct vec3 specularColor;
};

struct spotLightComponent {
    struct vec2 cutOff;

    struct vec3 ambientColor;
    struct vec3 diffuseColor;
    struct vec3 specularColor;
};

#endif
