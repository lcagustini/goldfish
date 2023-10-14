#ifndef MODEL_H
#define MODEL_H

#include <ecs/components.h>

struct vertex {
    struct vec3 position;
    struct vec3 normal;
    struct vec2 texture;
    struct vec3 tangent;
};

entityId loadModel(struct world *world, const char *modelPath, const char *diffusePath, const char *normalPath, const char *specularPath, const char *reflectancePath, bool transparent);
void destroyModel(struct modelComponent *model);

void printModel(struct modelComponent *model);

#endif
