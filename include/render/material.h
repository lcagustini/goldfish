#ifndef GOLDFISH_MATERIAL_H
#define GOLDFISH_MATERIAL_H

#include <epoxy/gl.h>
#include <GL/gl.h>

#include <render/shader.h>

#include <math/vector.h>
#include <math/matrix.h>

#include <stdbool.h>

#define MAX_LIGHTS 8

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

    struct shader *shader;
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

    union mat4 modelMatrix;

    bool castShadows;
    bool receiveShadows;

    unsigned int VAO;
    unsigned int indicesLength;
};

void shaderApplyUniform(const struct uniformRenderData *uniform);

// data points to the value matching type.
void shaderPushUniform(struct meshRenderData *mesh, unsigned int type, const char *name, const void *data);

void createMaterial(struct material *material, const char *diffusePath, const char *normalPath, const char *specularPath, const char *reflectancePath, bool transparent);

#endif
