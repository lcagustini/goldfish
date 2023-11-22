#ifndef MATERIAL_H
#define MATERIAL_H

#include <GL/glew.h>
#include <GL/gl.h>

#include <math/vector.h>
#include <math/matrix.h>

#include <stdbool.h>

#define MAX_LIGHTS 8

struct shader {
    GLuint modelLoc;
    GLuint viewLoc;
    GLuint projectionLoc;

    GLuint positionLoc;
    GLuint normalLoc;
    GLuint textCoordLoc;
    GLuint tangentLoc;

    GLuint cameraPosLoc;

    struct {
        GLuint direction;

        GLuint ambientColor;
        GLuint diffuseColor;
        GLuint specularColor;
    } dirLightLocs[MAX_LIGHTS];
    GLuint dirLightsLengthLoc;

    struct {
        GLuint position;

        GLuint attenuation;

        GLuint ambientColor;
        GLuint diffuseColor;
        GLuint specularColor;
    } pointLightLocs[MAX_LIGHTS];
    GLuint pointLightsLengthLoc;

    struct {
        GLuint position;
        GLuint direction;

        GLuint cutOff;

        GLuint ambientColor;
        GLuint diffuseColor;
        GLuint specularColor;
    } spotLightLocs[MAX_LIGHTS];
    GLuint spotLightsLengthLoc;

    GLuint shininessLoc;
    GLuint alphaClippingLoc;

    GLuint program;
};

enum textureType {
    TEXTURE_DIFFUSE,
    TEXTURE_NORMAL,
    TEXTURE_SPECULAR,
    TEXTURE_REFLECTANCE,
    TEXTURE_CUBEMAP,

    TEXTURE_MAX,
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
    float alphaClipping;

    bool transparent;

    struct shader shader;
};

struct uniformRenderData {
    enum {
        RENDERER_UNIFORM_MATRIX_4,
        RENDERER_UNIFORM_VECTOR_3,
        RENDERER_UNIFORM_VECTOR_2,
        RENDERER_UNIFORM_INT,
        RENDERER_UNIFORM_FLOAT
    } type;
    unsigned int location;
    unsigned int count;
    union {
        union mat4 matrix;
        struct vec3 vector3;
        struct vec2 vector2;
        int integer;
        float single;
    } data;
};

struct textureRenderData {
    enum {
        RENDERER_TEXTURE_2D,
        RENDERER_TEXTURE_CUBE_MAP
    } type;
    unsigned int slot;
    unsigned int buffer;
};

struct meshRenderData {
    struct shader shader;

    struct uniformRenderData uniforms[128];
    unsigned int uniformsLength;

    struct textureRenderData textures[16];
    unsigned int texturesLength;

    unsigned int VAO;
    unsigned int indicesLength;
};

GLuint loadShaderFromFile(const char *shaderFile, GLenum type);
void createMaterial(struct material *material, const char *diffusePath, const char *normalPath, const char *specularPath, const char *reflectancePath, bool transparent);

#endif
