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

#include <goldfish/engineUI.h>
#include <goldfish/print.h>
#include <goldfish/global.h>

void worldSetup(struct world *world);

int main() {
    setPrintType(PRINT_STDOUT, NULL);

    globalInit();

    print("Golfish init OK.\n");

    struct world ecsWorld = createWorld();

    worldSetup(&ecsWorld);

    double currentTime = glfwGetTime();
    double lastTime = currentTime;
    while (!glfwWindowShouldClose(globalState.window)) {
        currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glfwPollEvents();

        runWorldPhase(&ecsWorld, SYSTEM_ON_PRE_UPDATE, deltaTime);
        runWorldPhase(&ecsWorld, SYSTEM_ON_UPDATE, deltaTime);
        runWorldPhase(&ecsWorld, SYSTEM_ON_POST_UPDATE, deltaTime);

        runWorldPhase(&ecsWorld, SYSTEM_ON_RENDER_SORT, deltaTime);
        runWorldPhase(&ecsWorld, SYSTEM_ON_RENDER_SETUP, deltaTime);

        runWorldPhase(&ecsWorld, SYSTEM_ON_RENDER_OPAQUE, deltaTime);
        runWorldPhase(&ecsWorld, SYSTEM_ON_RENDER_SKYBOX, deltaTime);
        runWorldPhase(&ecsWorld, SYSTEM_ON_RENDER_TRANSPARENT, deltaTime);

        runWorldPhase(&ecsWorld, SYSTEM_ON_RENDER_POST, deltaTime);
        runWorldPhase(&ecsWorld, SYSTEM_ON_RENDER_FINISH, deltaTime);

        drawEngineUI(&ecsWorld);

        glfwSwapBuffers(globalState.window);
    }

#if 0
    model = getComponent(&ecsWorld, chest1, modelId);
    destroyModel(model);
    model = getComponent(&ecsWorld, chest2, modelId);
    destroyModel(model);
    model = getComponent(&ecsWorld, chest3, modelId);
    destroyModel(model);
#endif

    destroyWorld(&ecsWorld);

    globalEnd();

    return 0;
}
