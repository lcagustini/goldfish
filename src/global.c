#include <global.h>
#include <print.h>

#include <stdlib.h>

struct globalState globalState;

static void GLAPIENTRY errorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    fprintf(stderr, "[GL %s] type = 0x%x, severity = 0x%x, message = %s\n", ( type == GL_DEBUG_TYPE_ERROR ? "Error" : "Info" ), type, severity, message );
}

static void windowResizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void globalInit() {
    if (!glfwInit()) {
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    globalState.window = glfwCreateWindow(1280, 720, "psvita-opengl", NULL, NULL);
    if (!globalState.window) {
        print("Error creating window\n");
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(globalState.window);
    glfwSetFramebufferSizeCallback(globalState.window, windowResizeCallback);
    glfwSwapInterval(1);

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
    glDepthFunc(GL_LEQUAL);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

	glEnable(GL_FRAMEBUFFER_SRGB); 

#if 0
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(errorCallback, 0);
#endif

    print("OpenGL: %s - %s\n\n", glGetString(GL_VERSION), glGetString(GL_RENDERER));
}

void globalEnd() {
    glfwTerminate();
}
