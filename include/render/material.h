#ifndef MATERIAL_H
#define MATERIAL_H

#include <GL/glew.h>
#include <GL/gl.h>

#define MAX_LIGHTS 8

struct dirLight {
    GLuint direction;

    GLuint ambientColor;
    GLuint diffuseColor;
    GLuint specularColor;
};

struct pointLight {
    GLuint position;

    GLuint attenuation;

    GLuint ambientColor;
    GLuint diffuseColor;
    GLuint specularColor;
};

struct spotLight {
    GLuint position;
    GLuint direction;

    GLuint cutOff;

    GLuint ambientColor;
    GLuint diffuseColor;
    GLuint specularColor;
};

struct shader {
    GLuint modelLoc;
    GLuint viewLoc;
    GLuint projectionLoc;

    GLuint positionLoc;
    GLuint normalLoc;
    GLuint textCoordLoc;
    GLuint tangentLoc;

    GLuint cameraPosLoc;

    struct dirLight dirLightLocs[MAX_LIGHTS];
    GLuint dirLightsLengthLoc;

    struct pointLight pointLightLocs[MAX_LIGHTS];
    GLuint pointLightsLengthLoc;

    struct spotLight spotLightLocs[MAX_LIGHTS];
    GLuint spotLightsLengthLoc;

    GLuint shininessLoc;

    GLuint program;
};

enum textureType {
    TEXTURE_DIFFUSE,
    TEXTURE_NORMAL,
    TEXTURE_SPECULAR,

    TEXTURE_MAX
};

struct texture {
    GLuint textureBuffer;
    enum textureType type;
    const char *path;
};

struct material {
    struct texture textures[TEXTURE_MAX];
    unsigned int texturesLength;

    float shininess;

    struct shader shader;
};

void createMaterial(struct material *material, const char *diffusePath, const char *normalPath, const char *specularPath);

#endif
