// Matches the counts in render/material.h and render/shader.h.
#define MAX_LIGHTS 8
#define MAX_SHADOW_2D 6
#define MAX_SHADOW_CUBE 4
#define MAX_SHADOW_CASCADES 4

struct dirLight {
    vec3 direction;

    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;

    int shadowCascadeCount;
    float shadowBias;
    float shadowNormalBias;
    float shadowSlopeBias;
    float shadowSoftness;
    float shadowCascadeBlend;

    mat4 shadowMatrices[MAX_SHADOW_CASCADES];
    float shadowSplits[MAX_SHADOW_CASCADES];
    int shadowIndices[MAX_SHADOW_CASCADES];
};

struct pointLight {
    vec3 position;

    vec3 attenuation;

    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;

    float farPlane;
    int shadowIndex;
    float shadowBias;
    float shadowNormalBias;
    float shadowSoftness;
};

struct spotLight {
    vec3 position;
    vec3 direction;

    vec2 cutOff;

    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;

    mat4 lightSpaceMatrix;
    int shadowIndex;
    float shadowBias;
    float shadowNormalBias;
    float shadowSlopeBias;
    float shadowSoftness;
};
