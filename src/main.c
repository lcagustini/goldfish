#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define QOI_IMPLEMENTATION
#include <qoi.h>

#include <math/vector.h>
#include <math/matrix.h>

#include <render/model.h>
#include <render/skybox.h>

#include <ecs/world.h>
#include <ecs/systems.h>

#include <scripting/lua.h>

#include <goldfish/engineUI.h>
#include <goldfish/print.h>
#include <goldfish/global.h>

static void printUsage(const char *program) {
    print("Usage: %s [--headless]\n", program);
    print("  --headless   Run with a hidden window and no engine UI.\n");
}

static void parseArguments(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];
        if (strcmp(arg, "--headless") == 0) {
            globalState.headless = true;
        } else if (strcmp(arg, "--help") == 0) {
            printUsage(argv[0]);
            exit(0);
        } else {
            fprintf(stderr, "Unknown argument: %s\n", arg);
            printUsage(argv[0]);
            exit(1);
        }
    }
}

int main(int argc, char **argv) {
    parseArguments(argc, argv);

    setPrintType(PRINT_STDOUT, NULL);

    globalInit();

    print("Golfish init OK.\n");

    struct world ecsWorld = createWorld();

    luaInit(&ecsWorld);

    double currentTime = glfwGetTime();
    double lastTime = currentTime;
    while (!globalState.quitRequested && !glfwWindowShouldClose(globalState.window)) {
        currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glfwPollEvents();

        runWorldPhase(&ecsWorld, SYSTEM_ON_PRE_UPDATE, deltaTime);
        runWorldPhase(&ecsWorld, SYSTEM_ON_UPDATE, deltaTime);
        runWorldPhase(&ecsWorld, SYSTEM_ON_LATE_UPDATE, deltaTime);
        runWorldPhase(&ecsWorld, SYSTEM_ON_POST_UPDATE, deltaTime);

        runWorldPhase(&ecsWorld, SYSTEM_ON_RENDER_SORT, deltaTime);
        runWorldPhase(&ecsWorld, SYSTEM_ON_RENDER_SHADOW, deltaTime);
        runWorldPhase(&ecsWorld, SYSTEM_ON_RENDER_SETUP, deltaTime);

        runWorldPhase(&ecsWorld, SYSTEM_ON_RENDER_OPAQUE, deltaTime);
        runWorldPhase(&ecsWorld, SYSTEM_ON_RENDER_SKYBOX, deltaTime);
        runWorldPhase(&ecsWorld, SYSTEM_ON_RENDER_TRANSPARENT, deltaTime);

        runWorldPhase(&ecsWorld, SYSTEM_ON_RENDER_FINISH, deltaTime);

        if (!globalState.headless) {
            drawEngineUI(&ecsWorld);
            glfwSwapBuffers(globalState.window);
        }
    }

    luaEnd();

    destroyWorld(&ecsWorld);

    globalEnd();

    return 0;
}
