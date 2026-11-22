#ifndef GOLDFISH_COMPONENTS_H
#define GOLDFISH_COMPONENTS_H

#include <stdbool.h>

#include <math/vector.h>
#include <math/matrix.h>

#include <render/material.h>
#include <render/framebuffer.h>

#include <ecs/world.h>

#include <goldfish/input.h>

struct inputComponent {
    struct inputAction actions[INPUT_MAX_ACTIONS];
    unsigned int actionCount;

    bool keys[INPUT_KEY_COUNT];
    bool previousKeys[INPUT_KEY_COUNT];

    bool mouseButtons[INPUT_MOUSE_BUTTON_COUNT];
    bool previousMouseButtons[INPUT_MOUSE_BUTTON_COUNT];

    double mouseX;
    double mouseY;
    double previousMouseX;
    double previousMouseY;
    double mouseDeltaX;
    double mouseDeltaY;
    double scrollX;
    double scrollY;

    struct inputGamepad gamepads[INPUT_MAX_GAMEPADS];

    bool focused;
    bool firstPoll;
    bool resetMouse;
    bool uiCaptureKeyboard;
    bool uiCaptureMouse;

    struct inputCaptureState capture;
};

struct rendererDataComponent {
    struct meshRenderData opaqueMeshes[512];
    unsigned int opaqueMeshesLength;

    struct meshRenderData transparentMeshes[512];
    unsigned int transparentMeshesLength;

    framebufferId target;
};

struct skyboxComponent {
    struct shader *shader;

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
    struct shader *shader;
    unsigned int VAO, VBO;
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
