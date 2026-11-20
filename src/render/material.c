#include <render/material.h>

#include <stdlib.h>
#include <string.h>
#include <qoi.h>

#include <goldfish/print.h>
#include <render/shader.h>

void shaderApplyUniform(const struct uniformRenderData *uniform) {
    switch (uniform->type) {
        case RENDERER_UNIFORM_MATRIX_4:
            glUniformMatrix4fv(uniform->location, uniform->count, false, &uniform->data.matrix.a1);
            break;
        case RENDERER_UNIFORM_VECTOR_3:
            glUniform3fv(uniform->location, uniform->count, &uniform->data.vector3.x);
            break;
        case RENDERER_UNIFORM_VECTOR_2:
            glUniform2fv(uniform->location, uniform->count, &uniform->data.vector2.x);
            break;
        case RENDERER_UNIFORM_INT:
            glUniform1i(uniform->location, uniform->data.integer);
            break;
        case RENDERER_UNIFORM_FLOAT:
            glUniform1f(uniform->location, uniform->data.single);
            break;
    }
}

void shaderPushUniform(struct meshRenderData *mesh, unsigned int type, const char *name, const void *data) {
    struct uniformRenderData *uniform = &mesh->uniforms[mesh->uniformsLength++];
    uniform->type = type;
    uniform->location = shaderUniformLocation(&mesh->shader, name);
    uniform->count = 1;

    switch (type) {
        case RENDERER_UNIFORM_MATRIX_4:
            uniform->data.matrix = *(const union mat4 *)data;
            break;
        case RENDERER_UNIFORM_VECTOR_3:
            uniform->data.vector3 = *(const struct vec3 *)data;
            break;
        case RENDERER_UNIFORM_VECTOR_2:
            uniform->data.vector2 = *(const struct vec2 *)data;
            break;
        case RENDERER_UNIFORM_INT:
            uniform->data.integer = *(const int *)data;
            break;
        case RENDERER_UNIFORM_FLOAT:
            uniform->data.single = *(const float *)data;
            break;
    }
}

static void loadTexture(struct texture *texture, const char *path, enum textureType textureType) {
    glGenTextures(1, &texture->textureBuffer);

    qoi_desc desc;
    void *rgbPixels = qoi_read(path, &desc, 4);

    glBindTexture(GL_TEXTURE_2D, texture->textureBuffer);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, desc.width, desc.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbPixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    free(rgbPixels);

    texture->type = textureType;
    texture->path = path;
}

void createMaterial(struct material *material, const char *diffusePath, const char *normalPath, const char *specularPath, const char *reflectancePath, bool transparent) {
    memset(material, 0, sizeof(struct material));

    material->shader = malloc(sizeof(struct shader));
    if (!createShader(material->shader, GOLDFISH_ENGINE_ASSETS "/shaders/default_v.glsl", GOLDFISH_ENGINE_ASSETS "/shaders/default_f.glsl")) {
        free(material->shader);
        material->shader = NULL;
        return;
    }

    if (diffusePath) {
        loadTexture(&material->textures[material->texturesLength], diffusePath, TEXTURE_DIFFUSE);
        material->texturesLength++;
    }
    if (normalPath) {
        loadTexture(&material->textures[material->texturesLength], normalPath, TEXTURE_NORMAL);
        material->texturesLength++;
    }
    if (specularPath) {
        loadTexture(&material->textures[material->texturesLength], specularPath, TEXTURE_SPECULAR);
        material->texturesLength++;
    }
    if (reflectancePath) {
        loadTexture(&material->textures[material->texturesLength], reflectancePath, TEXTURE_REFLECTANCE);
        material->texturesLength++;
    }

    material->shininess = 1.0f;
    material->alphaClipping = 0.0f;
    material->transparent = transparent;
}
