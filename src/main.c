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

static union mat4 *projectionMat;
static union mat4 *viewMat;
static union mat4 *modelMat;

void testSystem1(unsigned int entity) {
    print("1\n");
}

void testSystem2(unsigned int entity) {
    print("2\n");
}

void testSystem3(unsigned int entity) {
    print("3\n");
}

int main() {
    resetPrint();

    globalInit();

    projectionMat = (union mat4*)malloc(sizeof(union mat4));
    viewMat = (union mat4*)malloc(sizeof(union mat4));
    modelMat = (union mat4*)malloc(sizeof(union mat4));
    loadIdentity(modelMat);

    print("All init OK.\n");

    createProjectionMatrix(projectionMat, 75, (float)globalState.surfaceWidth/(float)globalState.surfaceHeight);

    struct model *chest = loadModel("app0:assets/chest.obj", "app0:assets/chest.qoi", "app0:assets/chest_normal.qoi", "app0:assets/chest_specular.qoi");
    printModel(chest);

    struct world ecsWorld = {0};
    addSystem(&ecsWorld, (struct system) { 2, SYSTEM_ON_UPDATE, testSystem3 });
    addSystem(&ecsWorld, (struct system) { 0, SYSTEM_ON_UPDATE, testSystem1 });
    addSystem(&ecsWorld, (struct system) { 1, SYSTEM_ON_UPDATE, testSystem2 });

    const struct vec3 up = {0, 1, 0};
    struct vec3 pos = {2, 0.6f, 2};
    struct vec3 dir = {-0.706665f, -0.0353333f, -0.706665f};

    SceCtrlData ctrl;
    while (1) {
        sceCtrlPeekBufferPositive(0, &ctrl, 1);

        struct vec2 leftAnalog = { (ctrl.lx - 128) / 128.0f , (ctrl.ly - 128) / 128.0f };
        struct vec2 rightAnalog = { (ctrl.rx - 128) / 128.0f , (ctrl.ry - 128) / 128.0f };

        if (vectorLenSquared2D(leftAnalog) < 0.1f) leftAnalog = (struct vec2) {0};
        else leftAnalog = vectorScale2D(1 / 60.0f, leftAnalog);
        if (vectorLenSquared2D(rightAnalog) < 0.1f) rightAnalog = (struct vec2) {0};
        else rightAnalog = vectorScale2D(1 / 60.0f, rightAnalog);

        pos = vectorAdd(pos, (struct vec3) { leftAnalog.x, 0, leftAnalog.y });
        dir = vectorNormalize(vectorAdd(dir, (struct vec3) { rightAnalog.x, 0, rightAnalog.y }));

        lookAt(viewMat, pos, vectorAdd(pos, dir), up);

        loadIdentity(modelMat);
        //translationMatrix(modelMat, 0, (sin(20*i) + sin(40*i))/20.0f, 0);

        runWorldPhase(&ecsWorld, SYSTEM_ON_UPDATE);
        runWorldPhase(&ecsWorld, SYSTEM_ON_RENDER);
        print("--\n");

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (int i = 0; i < chest->meshesLength; i++) {
            glUniformMatrix4fv(chest->meshes[i].material.shader.modelLoc, 1, false, &modelMat->mat[0][0]);
            glUniformMatrix4fv(chest->meshes[i].material.shader.viewLoc, 1, false, &viewMat->mat[0][0]);
            glUniformMatrix4fv(chest->meshes[i].material.shader.projectionLoc, 1, false, &projectionMat->mat[0][0]);

            glUniform3fv(chest->meshes[i].material.shader.cameraPosLoc, 1, &pos.x);
        }
        drawModel(chest);

        eglSwapBuffers(globalState.display, globalState.surface);
    }

    free(projectionMat);
    free(viewMat);
    free(modelMat);

    globalEnd();

    return 0;
}
