// Matches MAX_LIGHTS in render/material.h.
#define MAX_LIGHTS 8

struct dirLight {
    vec3 direction;

    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
};

struct pointLight {
    vec3 position;

    vec3 attenuation;

    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
};

struct spotLight {
    vec3 position;
    vec3 direction;

    vec2 cutOff;

    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
};
