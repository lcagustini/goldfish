#ifndef GLOBAL_H
#define GLOBAL_H

#include <PVR_PSP2/EGL/eglplatform.h>
#include <PVR_PSP2/EGL/egl.h>

struct globalState {
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;

    EGLint surfaceWidth, surfaceHeight;
};

extern struct globalState globalState;

void globalInit();

void globalEnd();

#endif
