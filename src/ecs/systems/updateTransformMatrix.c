#include <ecs/systems.h>

static union mat4 getMatrixForTransform(struct transformComponent *t) {
    union mat4 m;

    loadIdentity(&m);

    translationMatrix(&m, t->position);
    rotationMatrix(&m, t->rotation);
    scalingMatrix(&m, t->scale);

    return m;
}

void updateTransformMatrix(struct systemRunData data) {
    struct transformComponent *transforms = GET_SYSTEM_COMPONENTS(data, 0, 0);

    for (int i = 0; i < GET_SYSTEM_COMPONENTS_LENGTH(data, 0); i++) {
        struct transformComponent *originalTransform = &transforms[i];
        loadIdentity(&originalTransform->modelMatrix);

        struct transformComponent *currentTransform = originalTransform;
        while (currentTransform != NULL) {
            union mat4 currentModel = getMatrixForTransform(currentTransform);
            multMatrix(&originalTransform->modelMatrix, &originalTransform->modelMatrix, &currentModel);

            currentTransform = GET_COMPONENT(data.world, currentTransform->parent, struct transformComponent);
        }
    }
}

