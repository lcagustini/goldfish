#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <stdbool.h>

#include <math/vector.h>
#include <math/matrix.h>

#include <render/material.h>

#include <ecs/world.h>

struct meshComponent {
    struct vertex *vertices;
    unsigned int verticesLength;

    unsigned int *indices;
    unsigned int indicesLength;

    struct material material;

    unsigned int VAO, VBO, EBO;
};

struct modelComponent {
    struct meshComponent *meshes;
    unsigned int meshesLength;

    const char *path;
};

struct transformComponent {
    struct vec3 position;
    struct quat rotation;
    struct vec3 scale;

    entityId parent;

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
