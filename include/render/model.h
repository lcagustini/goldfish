#ifndef MODEL_H
#define MODEL_H

#include <render/material.h>
#include <math/vector.h>

struct vertex {
    struct vec3 position;
    struct vec3 normal;
    struct vec2 texture;
    struct vec3 tangent;
};

struct mesh {
    struct vertex *vertices;
    unsigned int verticesLength;

    unsigned int *indices;
    unsigned int indicesLength;

    struct material material;

    unsigned int VAO, VBO, EBO;
};

struct model {
    struct mesh *meshes;
    unsigned int meshesLength;

    const char *path;
};

struct model loadModel(const char *modelPath, const char *diffusePath, const char *normalPath, const char *specularPath);
void destroyModel(struct model);

void printModel(struct model *model);

#endif
