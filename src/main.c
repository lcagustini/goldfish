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

#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

//SCE
int _newlib_heap_size_user   = 16 * 1024 * 1024;
unsigned int sceLibcHeapSize = 3 * 1024 * 1024;

//EGL
EGLDisplay Display;
EGLSurface Surface;
EGLContext Context;

//PIB cube egl stuff
static GLuint program;
static GLuint vertexID;
static GLuint colorID;

static GLuint mvpLoc;
static GLuint positionLoc;
static GLuint colorLoc;

typedef struct {
    GLfloat mat[4][4];
} glMatrix;

static glMatrix *projectionMat;
static glMatrix *modelviewMat;
static glMatrix *mvpMat;

static EGLint surface_width, surface_height;

static const GLfloat vertices[] =
{
    /* front */
    -0.5f, 0.5f, 0.5f,
    -0.5f, -0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,
    -0.5f, -0.5f, 0.5f,
    0.5f, -0.5f, 0.5f,

    /* right */
    0.5f, 0.5f, 0.5f,
    0.5f, -0.5f, 0.5f,
    0.5f, 0.5f, -0.5f,
    0.5f, 0.5f, -0.5f,
    0.5f, -0.5f, 0.5f,
    0.5f, -0.5f, -0.5f,

    /* back */
    0.5f, 0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    -0.5f, 0.5f, -0.5f,
    -0.5f, 0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,

    /* left */
    -0.5f, 0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, 0.5f, 0.5f,
    -0.5f, 0.5f, 0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, 0.5f,

    /* top */
    -0.5f, 0.5f, -0.5f,
    -0.5f, 0.5f, 0.5f,
    0.5f, 0.5f, -0.5f,
    0.5f, 0.5f, -0.5f,
    -0.5f, 0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,

    /* bottom */
    -0.5f, -0.5f, 0.5f,
    -0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, 0.5f,
    0.5f, -0.5f, 0.5f,
    -0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, -0.5f
};

static const GLfloat colors[] =
{
    /* front */
    0.0625f,0.57421875f,0.92578125f,1.0f,
    0.0625f,0.57421875f,0.92578125f,1.0f,
    0.0625f,0.57421875f,0.92578125f,1.0f,
    0.0625f,0.57421875f,0.92578125f,1.0f,
    0.0625f,0.57421875f,0.92578125f,1.0f,
    0.0625f,0.57421875f,0.92578125f,1.0f,

    /* right */
    0.29296875f,0.66796875f,0.92578125f,1.0f,
    0.29296875f,0.66796875f,0.92578125f,1.0f,
    0.29296875f,0.66796875f,0.92578125f,1.0f,
    0.29296875f,0.66796875f,0.92578125f,1.0f,
    0.29296875f,0.66796875f,0.92578125f,1.0f,
    0.29296875f,0.66796875f,0.92578125f,1.0f,

    /* back */
    0.52734375f,0.76171875f,0.92578125f,1.0f,
    0.52734375f,0.76171875f,0.92578125f,1.0f,
    0.52734375f,0.76171875f,0.92578125f,1.0f,
    0.52734375f,0.76171875f,0.92578125f,1.0f,
    0.52734375f,0.76171875f,0.92578125f,1.0f,
    0.52734375f,0.76171875f,0.92578125f,1.0f,

    /* left */
    0.0625f,0.57421875f,0.92578125f,1.0f,
    0.0625f,0.57421875f,0.92578125f,1.0f,
    0.0625f,0.57421875f,0.92578125f,1.0f,
    0.0625f,0.57421875f,0.92578125f,1.0f,
    0.0625f,0.57421875f,0.92578125f,1.0f,
    0.0625f,0.57421875f,0.92578125f,1.0f,

    /* top */
    0.29296875f,0.66796875f,0.92578125f,1.0f,
    0.29296875f,0.66796875f,0.92578125f,1.0f,
    0.29296875f,0.66796875f,0.92578125f,1.0f,
    0.29296875f,0.66796875f,0.92578125f,1.0f,
    0.29296875f,0.66796875f,0.92578125f,1.0f,
    0.29296875f,0.66796875f,0.92578125f,1.0f,

    /* bottom */
    0.52734375f,0.76171875f,0.92578125f,1.0f,
    0.52734375f,0.76171875f,0.92578125f,1.0f,
    0.52734375f,0.76171875f,0.92578125f,1.0f,
    0.52734375f,0.76171875f,0.92578125f,1.0f,
    0.52734375f,0.76171875f,0.92578125f,1.0f,
    0.52734375f,0.76171875f,0.92578125f,1.0f
};

void print(const char *format, ...) {
    va_list args;
    va_start(args, format);

    FILE *f = fopen("ux0:data/a.txt", "a");
    vfprintf(f, format, args);
    fclose(f);

    va_end(args);
}

#include "vector.c"
#include "model.c"
#include "matrix.c"
#include "shader.c"

void render(void) {
    glViewport(0, 0, surface_width, surface_height);

    /* Typical render pass */
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program);

    /* Enable and bind the vertex information */
    glEnableVertexAttribArray(positionLoc);
    glBindBuffer(GL_ARRAY_BUFFER, vertexID);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

    /* Enable and bind the color information */
    glEnableVertexAttribArray(colorLoc);
    glBindBuffer(GL_ARRAY_BUFFER, colorID);
    glVertexAttribPointer(colorLoc, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

    rotationMatrix(modelviewMat, 2.0f, -0.8f, -1.0f, -0.3f);
    multMatrix(mvpMat, modelviewMat, projectionMat);
    glUniformMatrix4fv(mvpLoc, 1, false, &mvpMat->mat[0][0]);

    /* Same draw call as in GLES1 */
    glDrawArrays(GL_TRIANGLES, 0, 36);

    /* Disable attribute arrays */
    glDisableVertexAttribArray(positionLoc);
    glDisableVertexAttribArray(colorLoc);

    eglSwapBuffers(Display, Surface);
}

/* This handles creating a view matrix for the Vita */
int initViewMatrix(void) {
    mvpLoc = glGetUniformLocation(program, "u_mvpMat");
    positionLoc = glGetAttribLocation(program, "a_position");
    colorLoc = glGetAttribLocation(program, "a_color");

    /* Generate vertex and color buffers and fill with data */
    glGenBuffers(1, &vertexID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &colorID);
    glBindBuffer(GL_ARRAY_BUFFER, colorID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

    projectionMat = (glMatrix*)malloc(sizeof(glMatrix));
    loadIdentity(projectionMat);

    GLfloat aspect = (float)surface_width/(float)surface_height;
    GLfloat near = -1.0f;
    GLfloat far  = 1.0f;
    GLfloat yFOV  = 75.0f;
    GLfloat height = tanf(yFOV / 360.0f * M_PI) * near;
    GLfloat width = height * aspect;

    frustumMatrix(projectionMat, -width, width, -height, height, near, far);

    if (surface_width > surface_height) {
        scaleMatrix(projectionMat, (float)surface_height/(float)surface_width, 1.0f, 1.0f);
    }
    else {
        scaleMatrix(projectionMat, 1.0f, (float)surface_width/(float)surface_height, 1.0f);
    }

    modelviewMat = (glMatrix*)malloc(sizeof(glMatrix));
    loadIdentity(modelviewMat);
    mvpMat = (glMatrix*)malloc(sizeof(glMatrix));

    return 0;
}

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

    // PIB cube demo
    eglQuerySurface(Display, Surface, EGL_WIDTH, &surface_width);
    eglQuerySurface(Display, Surface, EGL_HEIGHT, &surface_height);
    print("Surface Width: %d, Surface Height: %d\n", surface_width, surface_height);
    glClearDepthf(1.0f);
    glClearColor(0.0f,0.0f,0.0f,1.0f); // You can change the clear color to whatever

    glEnable(GL_CULL_FACE);

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
    print("All init OK.\n");

    initShaders();
    initViewMatrix();

    loadModel("app0:model/test.obj", NULL, VERTEX_ALL, 0);

    while (1) {
        render();
    }

    EGLEnd();

    sceKernelExitProcess(0);
    return 0;
}
