#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>

#include <PVR_PSP2/EGL/eglplatform.h>
#include <PVR_PSP2/EGL/egl.h>
#include <PVR_PSP2/gpu_es4/psp2_pvr_hint.h>
#include <PVR_PSP2/GLES2/gl2.h>

#include <psp2/ctrl.h>

#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <math/vector.h>
#include <math/matrix.h>

#include <render/model.h>

#include <ecs/world.h>

#include <print.h>
#include <global.h>

int _newlib_heap_size_user = 200 * 1024 * 1024;
unsigned int sceLibcHeapSize = 32 * 1024 * 1024;

void setupTransform(struct world *world, unsigned int id) {
    unsigned int transformId = world->entities[id].components[COMPONENT_TRANSFORM];

    if (transformId == INVALID_COMPONENT_ID) return;

    struct transformComponent *transform = &world->componentAllocator.transformComponents[transformId];

    transform->position = (struct vec3) {0};
    transform->rotation = (struct quat) {0, 0, 0, 1};
    transform->scale = (struct vec3) {1, 1, 1};
}

void setupCamera(struct world *world, unsigned int id) {
    unsigned int cameraId = world->entities[id].components[COMPONENT_CAMERA];

    if (cameraId == INVALID_COMPONENT_ID) return;

    struct cameraComponent *camera = &world->componentAllocator.cameraComponents[cameraId];

    createProjectionMatrix(&camera->projectionMat, 75, (float)globalState.surfaceWidth/(float)globalState.surfaceHeight);
}

void updateCameraView(struct world *world, unsigned int id) {
    enum componentType types[] = { COMPONENT_TRANSFORM, COMPONENT_CAMERA };
    unsigned int *entities = getEntitiesWithComponents(world, types, 2);

    for (int i = 1; i < entities[0]; i++) {
        unsigned int cameraId = world->entities[entities[i]].components[COMPONENT_CAMERA];
        struct cameraComponent *camera = &world->componentAllocator.cameraComponents[cameraId];

        unsigned int transformId = world->entities[entities[i]].components[COMPONENT_TRANSFORM];
        struct transformComponent *transform = &world->componentAllocator.transformComponents[transformId];

        struct vec3 dir = vectorRotate((struct vec3) { 0, 0, -1 }, transform->rotation);
        lookAt(&camera->viewMat, transform->position, vectorAdd(transform->position, dir), (struct vec3) { 0, 1, 0 });
    }

    free(entities);
}

void updateFirstPersonTransform(struct world *world, unsigned int id) {
    enum componentType controllerType[] = { COMPONENT_CONTROLLER_DATA };
    unsigned int *controllerEntity = getEntitiesWithComponents(world, controllerType, 1);

    unsigned int controllerDataId = world->entities[controllerEntity[1]].components[COMPONENT_CONTROLLER_DATA];
    struct controllerDataComponent *controllerData = &world->componentAllocator.controllerDataComponents[controllerDataId];

    enum componentType types[] = { COMPONENT_TRANSFORM, COMPONENT_FIRST_PERSON };
    unsigned int *entities = getEntitiesWithComponents(world, types, 2);

    for (int i = 1; i < entities[0]; i++) {
        unsigned int transformId = world->entities[entities[i]].components[COMPONENT_TRANSFORM];
        struct transformComponent *transform = &world->componentAllocator.transformComponents[transformId];

        SceCtrlData ctrl = controllerData->data;

        struct vec2 leftAnalog = { (ctrl.lx - 128) / 128.0f , (ctrl.ly - 128) / 128.0f };
        struct vec2 rightAnalog = { (ctrl.rx - 128) / 128.0f , (ctrl.ry - 128) / 128.0f };

        if (vectorLenSquared2D(leftAnalog) < 0.1f) leftAnalog = (struct vec2) {0};
        else leftAnalog = vectorScale2D(1 / 60.0f, leftAnalog);
        if (vectorLenSquared2D(rightAnalog) < 0.1f) rightAnalog = (struct vec2) {0};
        else rightAnalog = vectorScale2D(1 / 60.0f, rightAnalog);

        struct quat xQuat = quatFromAxisAngle((struct vec3) { 0, -1, 0 }, rightAnalog.x);
        struct quat yQuat = quatFromAxisAngle((struct vec3) { -1, 0, 0 }, rightAnalog.y);
        struct quat rot = quatMult(xQuat, yQuat);
        transform->rotation = quatMult(rot, transform->rotation);

        struct vec3 dir = vectorRotate((struct vec3) { 0, 0, 1 }, transform->rotation);
        transform->position = vectorAdd(transform->position, vectorScale(leftAnalog.y, dir));

        struct vec3 right = vectorRotate((struct vec3) { 1, 0, 0 }, transform->rotation);
        transform->position = vectorAdd(transform->position, vectorScale(leftAnalog.x, right));
    }

    free(controllerEntity);
    free(entities);
}

void updateControllerData(struct world *world, unsigned int id) {
    enum componentType types[] = { COMPONENT_CONTROLLER_DATA };
    unsigned int *entities = getEntitiesWithComponents(world, types, 1);

    for (int i = 1; i < entities[0]; i++) {
        unsigned int controllerDataId = world->entities[entities[i]].components[COMPONENT_CONTROLLER_DATA];
        struct controllerDataComponent *controllerData = &world->componentAllocator.controllerDataComponents[controllerDataId];

        sceCtrlPeekBufferPositive(0, &controllerData->data, 1);
    }

    free(entities);
}

void renderModel(struct world *world, unsigned int id) {
    enum componentType types[] = { COMPONENT_TRANSFORM, COMPONENT_MODEL };
    unsigned int *entities = getEntitiesWithComponents(world, types, 2);

    enum componentType cameraType[] = { COMPONENT_TRANSFORM, COMPONENT_CAMERA };
    unsigned int *cameraEntity = getEntitiesWithComponents(world, cameraType, 2);

    for (int i = 1; i < entities[0]; i++) {
        unsigned int transformId = world->entities[entities[i]].components[COMPONENT_TRANSFORM];
        struct transformComponent *transform = &world->componentAllocator.transformComponents[transformId];

        unsigned int modelId = world->entities[entities[i]].components[COMPONENT_MODEL];
        struct modelComponent *model = &world->componentAllocator.modelComponents[modelId];

        for (int i = 0; i < model->model.meshesLength; i++) {
            glUniformMatrix4fv(model->model.meshes[i].material.shader.modelLoc, 1, false, &transform->modelMatrix.mat[0][0]);
            glUniformMatrix4fv(model->model.meshes[i].material.shader.viewLoc, 1, false, &world->componentAllocator.cameraComponents[world->entities[cameraEntity[1]].components[COMPONENT_CAMERA]].viewMat.mat[0][0]);
            glUniformMatrix4fv(model->model.meshes[i].material.shader.projectionLoc, 1, false, &world->componentAllocator.cameraComponents[world->entities[cameraEntity[1]].components[COMPONENT_CAMERA]].projectionMat.mat[0][0]);

            glUniform3fv(model->model.meshes[i].material.shader.cameraPosLoc, 1, &world->componentAllocator.transformComponents[world->entities[cameraEntity[1]].components[COMPONENT_TRANSFORM]].position.x);
        }
        drawModel(&model->model);
    }

    free(entities);
    free(cameraEntity);
}

void updateTransformMatrix(struct world *world, unsigned int id) {
    enum componentType types[] = { COMPONENT_TRANSFORM };
    unsigned int *entities = getEntitiesWithComponents(world, types, 1);

    for (int i = 1; i < entities[0]; i++) {
        unsigned int transformId = world->entities[entities[i]].components[COMPONENT_TRANSFORM];
        struct transformComponent *transform = &world->componentAllocator.transformComponents[transformId];

        loadIdentity(&transform->modelMatrix);
        scalingMatrix(&transform->modelMatrix, transform->scale);
        rotationMatrix(&transform->modelMatrix, transform->rotation);
        translationMatrix(&transform->modelMatrix, transform->position);
    }

    free(entities);
}

int main() {
    resetPrint();

    globalInit();

    print("All init OK.\n");

    struct world ecsWorld = {0};
    addSystem(&ecsWorld, (struct system) { "setupTransform", 0, SYSTEM_ON_CREATE, setupTransform });
    addSystem(&ecsWorld, (struct system) { "setupCamera", 0, SYSTEM_ON_CREATE, setupCamera });

    addSystem(&ecsWorld, (struct system) { "updateControllerData", 0, SYSTEM_ON_UPDATE, updateControllerData });
    addSystem(&ecsWorld, (struct system) { "updateFirstPersonTransform", 1, SYSTEM_ON_UPDATE, updateFirstPersonTransform });
    addSystem(&ecsWorld, (struct system) { "updateCameraView", 2, SYSTEM_ON_UPDATE, updateCameraView });
    addSystem(&ecsWorld, (struct system) { "updateTransformMatrix", 10, SYSTEM_ON_UPDATE, updateTransformMatrix });

    addSystem(&ecsWorld, (struct system) { "renderModel", 0, SYSTEM_ON_RENDER, renderModel });

    unsigned int camera = createEntity(&ecsWorld);
    addComponent(&ecsWorld, camera, COMPONENT_TRANSFORM);
    addComponent(&ecsWorld, camera, COMPONENT_CAMERA);
    addComponent(&ecsWorld, camera, COMPONENT_FIRST_PERSON);

    unsigned int input = createEntity(&ecsWorld);
    addComponent(&ecsWorld, input, COMPONENT_CONTROLLER_DATA);

    unsigned int chest = createEntity(&ecsWorld);
    addComponent(&ecsWorld, chest, COMPONENT_TRANSFORM);
    addComponent(&ecsWorld, chest, COMPONENT_MODEL);

    ecsWorld.componentAllocator.modelComponents[ecsWorld.entities[chest].components[COMPONENT_MODEL]].model = loadModel("app0:assets/chest.obj", "app0:assets/chest.qoi", "app0:assets/chest_normal.qoi", "app0:assets/chest_specular.qoi");

    while (1) {
        ecsWorld.componentAllocator.transformComponents[ecsWorld.entities[chest].components[COMPONENT_TRANSFORM]].rotation = quatMult(ecsWorld.componentAllocator.transformComponents[ecsWorld.entities[chest].components[COMPONENT_TRANSFORM]].rotation, (struct quat) { 0, 0.005, 0, 0.9999875 });

        runWorldPhase(&ecsWorld, SYSTEM_ON_UPDATE);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        runWorldPhase(&ecsWorld, SYSTEM_ON_RENDER);

        eglSwapBuffers(globalState.display, globalState.surface);
    }

    globalEnd();

    return 0;
}
