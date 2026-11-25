#ifndef GOLDFISH_SHADER_H
#define GOLDFISH_SHADER_H

#include <epoxy/gl.h>

#include <data/hashtable.h>

#include <stdbool.h>

enum textureType {
    TEXTURE_DIFFUSE,
    TEXTURE_NORMAL,
    TEXTURE_SPECULAR,
    TEXTURE_REFLECTANCE,
    TEXTURE_CUBEMAP,

    TEXTURE_MAX,
};

// Shadow samplers live in units after the material textures. These counts
// must match the defines in lights.glsl.
#define MAX_SHADOW_2D 6
#define MAX_SHADOW_CUBE 4
#define MAX_SHADOW_CASCADES 4

#define SHADOW_2D_UNIT_BASE TEXTURE_MAX
#define SHADOW_CUBE_UNIT_BASE (SHADOW_2D_UNIT_BASE + MAX_SHADOW_2D)

struct shader {
    GLuint program;
    struct hashtable *uniforms;
};

bool createShader(struct shader *shader, const char *vertexPath, const char *fragmentPath);
bool shaderReplace(struct shader *shader, const char *vertexPath, const char *fragmentPath);
GLint shaderUniformLocation(struct shader *shader, const char *name);
void destroyShader(struct shader *shader);
void shaderHotReload(void);

#endif
