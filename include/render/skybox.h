#ifndef GOLDFISH_SKYBOX_H
#define GOLDFISH_SKYBOX_H

#include <ecs/components.h>

void loadSkybox(const char *facePaths[6], struct skyboxComponent *skybox);
void destroySkybox(struct skyboxComponent *skybox);

#endif
