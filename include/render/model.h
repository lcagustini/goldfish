#ifndef MODEL_H
#define MODEL_H

#include <math/vector.h>
#include <render/shader.h>

struct vertex {
    struct vec3 position;
    struct vec3 normal;
    struct vec2 texture;
};

enum textureType {
    TEXTURE_DIFFUSE,
    TEXTURE_NORMAL,
    TEXTURE_SPECULAR
};

struct texture {
    unsigned int textureBuffer;
    enum textureType type;
};

struct mesh {
    struct vertex *vertices;
    unsigned int verticesLength;

    unsigned int *indices;
    unsigned int indicesLength;

    struct texture *textures;
    unsigned int texturesLength;

    unsigned int VAO, VBO, EBO;
};

struct model {
    struct mesh *meshes;
    unsigned int meshesLength;

    const char *path;
};

struct model *loadModel(const char *path);

void drawModel(struct model *model, struct shader shader);

void printModel(struct model *model);

#endif
