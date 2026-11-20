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
