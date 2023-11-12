#include <stdio.h>

#include <global.h>

int main(void) {
    initGlobal();

    while (!glfwWindowShouldClose(globalState.window)) {
        glfwPollEvents();
        drawFrame();
    }

    destroyGlobal();

    return 0;
}