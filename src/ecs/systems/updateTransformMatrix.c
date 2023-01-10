#include <ecs/systems.h>

void updateTransformMatrix(struct systemRunData data) {
    struct transformComponent *transforms = GET_SYSTEM_COMPONENTS(data, 0);

    for (int i = 0; i < GET_SYSTEM_COMPONENTS_LENGTH(data); i++) {
        struct transformComponent *transform = &transforms[i];

        loadIdentity(&transform->modelMatrix);
        translationMatrix(&transform->modelMatrix, transform->position);
        rotationMatrix(&transform->modelMatrix, transform->rotation);
        scalingMatrix(&transform->modelMatrix, transform->scale);
    }
}

