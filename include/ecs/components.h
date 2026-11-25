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

    struct vec3 cameraPosition;
    struct vec3 cameraForward;
    struct vec3 cameraRight;
    struct vec3 cameraUp;

    float cameraFov;
    float cameraAspect;
    float cameraNear;
    float cameraFar;
    float cameraOrthoSize;
    bool cameraOrthographic;

    bool cameraValid;

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

    bool castShadows;
    bool receiveShadows;
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

    bool castShadows;
    int shadowResolution;
    float shadowDistance;
    int shadowCascades;
    float shadowSplitLambda;
    float shadowCascadeBlend;
    float shadowBias;
    float shadowNormalBias;
    float shadowSlopeBias;
    float shadowSoftness;
    float shadowCasterPadding;

    // Runtime: written by rendererDirShadowPass, read by rendererGetDirLights.
    int shadowIndices[MAX_SHADOW_CASCADES];
    union mat4 shadowMatrices[MAX_SHADOW_CASCADES];
    float shadowSplits[MAX_SHADOW_CASCADES];
    int shadowCascadeCount;
};

struct pointLightComponent {
    struct vec3 attenuation;

    struct vec3 ambientColor;
    struct vec3 diffuseColor;
    struct vec3 specularColor;

    float range;

    bool castShadows;
    int shadowResolution;
    float shadowBias;
    float shadowNormalBias;
    float shadowSoftness;

    // Runtime: written by rendererPointShadowPass.
    int shadowIndex;
};

struct spotLightComponent {
    struct vec2 cutOff;

    struct vec3 ambientColor;
    struct vec3 diffuseColor;
    struct vec3 specularColor;

    float range;

    bool castShadows;
    int shadowResolution;
    float shadowBias;
    float shadowNormalBias;
    float shadowSlopeBias;
    float shadowSoftness;

    // Runtime: written by rendererSpotShadowPass, read by rendererGetSpotLights.
    int shadowIndex;
    union mat4 shadowMatrix;
};

#endif
