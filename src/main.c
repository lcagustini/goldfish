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

#include <render/shader.h>
#include <render/model.h>

#include <print.h>
#include <global.h>

int _newlib_heap_size_user = 200 * 1024 * 1024;
unsigned int sceLibcHeapSize = 32 * 1024 * 1024;

static union mat4 *projectionMat;
static union mat4 *viewMat;
static union mat4 *modelMat;

int main() {
    resetPrint();

    globalInit();

    projectionMat = (union mat4*)malloc(sizeof(union mat4));
    viewMat = (union mat4*)malloc(sizeof(union mat4));
    modelMat = (union mat4*)malloc(sizeof(union mat4));
    loadIdentity(modelMat);

    print("All init OK.\n");

    struct shader shader;
    initShaders(&shader);
    createProjectionMatrix(projectionMat, 75, (float)globalState.surfaceWidth/(float)globalState.surfaceHeight);

    //loadModel("app0:assets/chest.obj", "app0:assets/chest.qoi", "app0:assets/chest_specular.qoi", "app0:assets/chest_normal.qoi", VERTEX_ALL);
    struct model *chest = loadModel("app0:assets/chest.obj");
    printModel(chest);

    SceCtrlData ctrl;

    float i = 0;
    const float radius = 80;
    while (1) {
        sceCtrlPeekBufferPositive(0, &ctrl, 1);

        if (ctrl.buttons & SCE_CTRL_LTRIGGER) {
        }
        if (ctrl.buttons & SCE_CTRL_RTRIGGER) {
        }

        i += 0.01f;

        float camX = sin(i) * radius;
        float camZ = cos(i) * radius;

        struct vec3 pos = {camX, 40, camZ};
        struct vec3 target = {0, 40, 0};
        struct vec3 up = {0, 1, 0};
        lookAt(viewMat, pos, target, up);

        loadIdentity(modelMat);
        //translationMatrix(modelMat, 0, (sin(20*i) + sin(40*i))/20.0f, 0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUniformMatrix4fv(shader.modelLoc, 1, false, &modelMat->mat[0][0]);
        glUniformMatrix4fv(shader.viewLoc, 1, false, &viewMat->mat[0][0]);
        glUniformMatrix4fv(shader.projectionLoc, 1, false, &projectionMat->mat[0][0]);

        glUniform3fv(shader.cameraPosLoc, 1, &pos.x);
        drawModel(chest, shader);

        eglSwapBuffers(globalState.display, globalState.surface);
    }

    free(projectionMat);
    free(viewMat);
    free(modelMat);

    globalEnd();

    return 0;
}
