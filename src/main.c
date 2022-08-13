#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/clib.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>

#include <PVR_PSP2/EGL/eglplatform.h>
#include <PVR_PSP2/EGL/egl.h>
#include <PVR_PSP2/gpu_es4/psp2_pvr_hint.h>

#include <PVR_PSP2/GLES2/gl2.h>
#include <PVR_PSP2/GLES2/gl2ext.h>

#include <psp2/ctrl.h>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

//SCE
int _newlib_heap_size_user = 200 * 1024 * 1024;
unsigned int sceLibcHeapSize = 32 * 1024 * 1024;

//EGL
EGLDisplay Display;
EGLSurface Surface;
EGLContext Context;

//PIB cube egl stuff
static unsigned int program;

typedef struct {
    GLfloat mat[4][4];
} glMatrix;

static glMatrix *projectionMat;
static glMatrix *viewMat;
static glMatrix *modelMat;

static EGLint surface_width, surface_height;

#define GLERROR() \
if (glGetError() != GL_NO_ERROR) { \
    print("!!!!! GL error on line %s: %d !!!!!!\n", __func__, __LINE__); \
    assert(false);\
}

void print(const char *format, ...) {
    va_list args;
    va_start(args, format);

    FILE *f = fopen("ux0:data/a.txt", "a");
    vfprintf(f, format, args);
    fclose(f);

    va_end(args);
}

#define QOI_IMPLEMENTATION
#include "qoi.h"

#include "vector.c"
#include "matrix.c"
#include "shader.c"
#include "model.c"

void ModuleInit() {
    sceKernelLoadStartModule("vs0:sys/external/libfios2.suprx", 0, NULL, 0, NULL, NULL);
    sceKernelLoadStartModule("vs0:sys/external/libc.suprx", 0, NULL, 0, NULL, NULL);
    sceKernelLoadStartModule("app0:module/libgpu_es4_ext.suprx", 0, NULL, 0, NULL, NULL);
    sceKernelLoadStartModule("app0:module/libIMGEGL.suprx", 0, NULL, 0, NULL, NULL);
    print("Module init OK\n");
}

void PVR_PSP2Init() {
    PVRSRV_PSP2_APPHINT hint;
    PVRSRVInitializeAppHint(&hint);
    PVRSRVCreateVirtualAppHint(&hint);
    print("PVE_PSP2 init OK.\n");
}

void EGLInit() {
    EGLBoolean Res;
    Display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (!Display) {
        print("EGL display get failed.\n");
        return;
    }

    EGLint NumConfigs, MajorVersion, MinorVersion;
    Res = eglInitialize(Display, &MajorVersion, &MinorVersion);
    if (Res == EGL_FALSE) {
        print("EGL initialize failed.\n");
        return;
    }

    //PIB cube demo
    eglBindAPI(EGL_OPENGL_ES_API);

    EGLint ConfigAttr[] =
    {
        EGL_BUFFER_SIZE, EGL_DONT_CARE,
        EGL_DEPTH_SIZE, 16,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_STENCIL_SIZE, 8,
        EGL_SURFACE_TYPE, 5,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };
    EGLConfig Config;
    Res = eglChooseConfig(Display, ConfigAttr, &Config, 1, &NumConfigs);
    if (Res == EGL_FALSE) {
        print("EGL config initialize failed.\n");
        return;
    }

    Surface = eglCreateWindowSurface(Display, Config, (EGLNativeWindowType)0, NULL);
    if (!Surface) {
        print("EGL surface create failed.\n");
        return;
    }

    EGLint ContextAttributeList[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    Context = eglCreateContext(Display, Config, EGL_NO_CONTEXT, ContextAttributeList);
    if (!Context) {
        print("EGL content create failed.\n");
        return;
    }

    eglMakeCurrent(Display, Surface, Surface, Context);

    GLERROR()

    // PIB cube demo
    eglQuerySurface(Display, Surface, EGL_WIDTH, &surface_width);
    eglQuerySurface(Display, Surface, EGL_HEIGHT, &surface_height);
    print("Surface Width: %d, Surface Height: %d\n", surface_width, surface_height);
    GLERROR()
    glClearDepthf(1.0f);
    glClearColor(0.1f, 0.1f, 0.3f, 1.0f);

    GLERROR()
    //glEnable(GL_CULL_FACE);
    GLERROR()
    glEnable(GL_DEPTH_TEST);
    GLERROR()
    //glDepthFunc(GL_LEQUAL);
    //GLERROR()

    glViewport(0, 0, surface_width, surface_height);
    GLERROR()

    print("EGL init OK.\n");
}

void EGLEnd() {
    eglDestroySurface(Display, Surface);
    eglDestroyContext(Display, Context);
    eglTerminate(Display);
    print("EGL terminated.\n");
}

void SCEInit() {
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
    print("SCE init OK\n");
}

int main() {
    ModuleInit();
    PVR_PSP2Init();
    EGLInit();
    SCEInit();

    FILE *f = fopen("ux0:data/a.txt", "w");
    fclose(f);

    projectionMat = (glMatrix*)malloc(sizeof(glMatrix));
    viewMat = (glMatrix*)malloc(sizeof(glMatrix));
    modelMat = (glMatrix*)malloc(sizeof(glMatrix));
    loadIdentity(modelMat);

    GLERROR()
    print("All init OK.\n");

    GLERROR()
    initShaders();
    GLERROR()
    createProjectionMatrix(75);

    GLERROR()

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

        glUniformMatrix4fv(modelLoc, 1, false, &modelMat->mat[0][0]);
        glUniformMatrix4fv(viewLoc, 1, false, &viewMat->mat[0][0]);
        glUniformMatrix4fv(projectionLoc, 1, false, &projectionMat->mat[0][0]);

        glUniform3fv(cameraPosLoc, 1, &pos.x);
        drawModel(chest, program);

        eglSwapBuffers(Display, Surface);
    }

    free(projectionMat);
    free(viewMat);
    free(modelMat);

    EGLEnd();

    sceKernelExitProcess(0);
    return 0;
}
