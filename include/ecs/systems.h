#ifndef SYSTEMS_H
#define SYSTEMS_H

#include <ecs/world.h>

#include <string.h>
#include <global.h>

void updateCameraView(struct systemRunData data);
void updateControllerData(struct systemRunData data);
void updateTransformMatrix(struct systemRunData data);
void setupTransform(struct systemRunData data);
void setupCamera(struct systemRunData data);
void updateFirstPersonTransform(struct systemRunData data);
void renderModel(struct systemRunData data);

#endif
