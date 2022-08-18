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

int main() {
    resetPrint();

    globalInit();

    print("All init OK.\n");

    struct model *chest = loadModel("app0:assets/chest.obj", "app0:assets/chest.qoi", "app0:assets/chest_normal.qoi", "app0:assets/chest_specular.qoi");
    printModel(chest);

    struct world ecsWorld = {0};
    addSystem(&ecsWorld, (struct system) { 0, SYSTEM_ON_CREATE, setupCamera });
    addSystem(&ecsWorld, (struct system) { 0, SYSTEM_ON_UPDATE, updateCameraView });

    unsigned int camera = createEntity(&ecsWorld);
    addComponent(&ecsWorld, camera, COMPONENT_TRANSFORM);
    addComponent(&ecsWorld, camera, COMPONENT_CAMERA);

    SceCtrlData ctrl;
    while (1) {
        sceCtrlPeekBufferPositive(0, &ctrl, 1);

        struct vec2 leftAnalog = { (ctrl.lx - 128) / 128.0f , (ctrl.ly - 128) / 128.0f };
        struct vec2 rightAnalog = { (ctrl.rx - 128) / 128.0f , (ctrl.ry - 128) / 128.0f };

        if (vectorLenSquared2D(leftAnalog) < 0.1f) leftAnalog = (struct vec2) {0};
        else leftAnalog = vectorScale2D(1 / 60.0f, leftAnalog);
        if (vectorLenSquared2D(rightAnalog) < 0.1f) rightAnalog = (struct vec2) {0};
        else rightAnalog = vectorScale2D(1 / 60.0f, rightAnalog);

        struct transformComponent *cameraTransform = &ecsWorld.componentAllocator.transformComponents[ecsWorld.entities[camera].components[COMPONENT_TRANSFORM]];
        struct vec3 rot = quatToEuler(cameraTransform->rotation);
        rot.x += rightAnalog.y;
        rot.y += rightAnalog.x;
        cameraTransform->rotation = eulerToQuat(rot);
        struct vec3 dir = vectorRotate((struct vec3) { 0, 0, 1 }, cameraTransform->rotation);
        cameraTransform->position = vectorAdd(cameraTransform->position, vectorScale(leftAnalog.y, dir));

        runWorldPhase(&ecsWorld, SYSTEM_ON_UPDATE);
        runWorldPhase(&ecsWorld, SYSTEM_ON_RENDER);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        union mat4 modelMat;
        loadIdentity(&modelMat);
        for (int i = 0; i < chest->meshesLength; i++) {
            glUniformMatrix4fv(chest->meshes[i].material.shader.modelLoc, 1, false, &modelMat.mat[0][0]);
            glUniformMatrix4fv(chest->meshes[i].material.shader.viewLoc, 1, false, &ecsWorld.componentAllocator.cameraComponents[ecsWorld.entities[camera].components[COMPONENT_CAMERA]].viewMat.mat[0][0]);
            glUniformMatrix4fv(chest->meshes[i].material.shader.projectionLoc, 1, false, &ecsWorld.componentAllocator.cameraComponents[ecsWorld.entities[camera].components[COMPONENT_CAMERA]].projectionMat.mat[0][0]);

            glUniform3fv(chest->meshes[i].material.shader.cameraPosLoc, 1, &ecsWorld.componentAllocator.transformComponents[ecsWorld.entities[camera].components[COMPONENT_TRANSFORM]].position.x);
        }
        drawModel(chest);

        eglSwapBuffers(globalState.display, globalState.surface);
    }

    globalEnd();

    return 0;
}
