#include <global.h>
#include <print.h>

#include <stdlib.h>

struct globalState globalState;

void globalInit() {
    /*
    sceKernelLoadStartModule("vs0:sys/external/libfios2.suprx", 0, NULL, 0, NULL, NULL);
    sceKernelLoadStartModule("vs0:sys/external/libc.suprx", 0, NULL, 0, NULL, NULL);
    sceKernelLoadStartModule("app0:module/libgpu_es4_ext.suprx", 0, NULL, 0, NULL, NULL);
    sceKernelLoadStartModule("app0:module/libIMGEGL.suprx", 0, NULL, 0, NULL, NULL);

    PVRSRV_PSP2_APPHINT hint;
    PVRSRVInitializeAppHint(&hint);
    PVRSRVCreateVirtualAppHint(&hint);

    EGLBoolean res;
    globalState.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (!globalState.display) {
        print("EGL display get failed.\n");
        return;
    }

    EGLint numConfigs, majorVersion, minorVersion;
    res = eglInitialize(globalState.display, &majorVersion, &minorVersion);
    if (res == EGL_FALSE) {
        print("EGL initialize failed.\n");
        return;
    }

    eglBindAPI(EGL_OPENGL_ES_API);

    EGLint configAttr[] =
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
    EGLConfig config;
    res = eglChooseConfig(globalState.display, configAttr, &config, 1, &numConfigs);
    if (res == EGL_FALSE) {
        print("EGL config initialize failed.\n");
        return;
    }

    globalState.surface = eglCreateWindowSurface(globalState.display, config, (EGLNativeWindowType)0, NULL);
    if (!globalState.surface) {
        print("EGL surface create failed.\n");
        return;
    }

    EGLint contextAttributeList[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    globalState.context = eglCreateContext(globalState.display, config, EGL_NO_CONTEXT, contextAttributeList);
    if (!globalState.context) {
        print("EGL content create failed.\n");
        return;
    }

    eglMakeCurrent(globalState.display, globalState.surface, globalState.surface, globalState.context);

    eglQuerySurface(globalState.display, globalState.surface, EGL_WIDTH, &globalState.surfaceWidth);
    eglQuerySurface(globalState.display, globalState.surface, EGL_HEIGHT, &globalState.surfaceHeight);
    print("Surface Width: %d, Surface Height: %d\n", globalState.surfaceWidth, globalState.surfaceHeight);
    glClearDepthf(1.0f);
    glClearColor(0.1f, 0.1f, 0.3f, 1.0f);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    glViewport(0, 0, globalState.surfaceWidth, globalState.surfaceHeight);

    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
    */

    if (!glfwInit()) {
        return;
    }

    //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    globalState.window = glfwCreateWindow(1280, 720, "Hello World", NULL, NULL);
    if (!globalState.window) {
        print("Error creating window\n");
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(globalState.window);

    glewExperimental = 1;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        print("Error creating window\n");
        glfwTerminate();
        return;
    }

    glClearDepthf(1.0f);
    glClearColor(0.1f, 0.1f, 0.3f, 1.0f);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    print("%s - %s\n\n", glGetString(GL_VERSION), glGetString(GL_RENDERER));
}

void globalEnd() {
    /*
    eglDestroySurface(globalState.display, globalState.surface);
    eglDestroyContext(globalState.display, globalState.context);
    eglTerminate(globalState.display);
    */
    glfwTerminate();
}
