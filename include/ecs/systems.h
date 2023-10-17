#ifndef SYSTEMS_H
#define SYSTEMS_H

#include <ecs/world.h>
#include <ecs/components.h>

#include <string.h>
#include <global.h>

void setupTransform(struct systemRunData data);
void setupCamera(struct systemRunData data);

void updateControllerData(struct systemRunData data);
void updateCameraView(struct systemRunData data);
void updateFirstPersonTransform(struct systemRunData data);

void updateTransformMatrix(struct systemRunData data);

void rendererGetModels(struct systemRunData data);
void rendererGetCameras(struct systemRunData data);
void rendererGetSkybox(struct systemRunData data);
void rendererGetDirLights(struct systemRunData data);
void rendererGetSpotLights(struct systemRunData data);
void rendererGetPointLights(struct systemRunData data);

void rendererOpaqueRender(struct systemRunData data);
void renderSkybox(struct systemRunData data);
void rendererTransparentRender(struct systemRunData data);

void finishRender(struct systemRunData data);

#endif
