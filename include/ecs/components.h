#ifndef GOLDFISH_COMPONENTS_H
#define GOLDFISH_COMPONENTS_H

#include <stdbool.h>

#include <math/vector.h>
#include <math/matrix.h>

#include <render/material.h>
#include <render/framebuffer.h>

#include <ecs/world.h>

struct rendererDataComponent {
    struct meshRenderData opaqueMeshes[512];
    unsigned int opaqueMeshesLength;

    struct meshRenderData transparentMeshes[512];
    unsigned int transparentMeshesLength;

    framebufferId target;
};

struct skyboxComponent {
    unsigned int shaderProgram;

    unsigned int texture;
    unsigned int VAO, VBO, EBO;
};

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
};

struct transformComponent {
    struct vec3 position;
    struct quat rotation;
    struct vec3 scale;

    entityId parent;

    union mat4 modelMatrix;
};

struct cameraComponent {
    bool orthographic;
    union { float fov; float size; };
    float near;
    float far;

    union mat4 viewMat;
    union mat4 projectionMat;

    framebufferId framebuffer;
    unsigned int shader;
    unsigned int VAO, VBO;
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
