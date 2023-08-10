#include <ecs/systems.h>

void setupTransform(struct systemRunData data) {
    struct transformComponent *transform = GET_SYSTEM_COMPONENT(data);
    transform->position = (struct vec3) {0};
    transform->rotation = (struct quat) {0, 0, 0, 1};
    transform->scale = (struct vec3) {1, 1, 1};
    transform->parent = NULL;
}

