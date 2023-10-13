#include <ecs/systems.h>

#include <math/float.h>

void updateFirstPersonTransform(struct systemRunData data) {
    struct controllerDataComponent *controllerData = GET_SINGLETON_COMPONENT(data.world, struct controllerDataComponent);

    struct transformComponent *transforms = GET_SYSTEM_COMPONENTS(data, 0, 0);
    struct firstPersonComponent *rotations = GET_SYSTEM_COMPONENTS(data, 0, 1);

    for (int i = 0; i < GET_SYSTEM_COMPONENTS_LENGTH(data, 0); i++) {
        struct transformComponent *transform = &transforms[i];
        struct firstPersonComponent *rotation = &rotations[i];

        struct vec2 leftAnalog = { (controllerData->lx - 128) / 128.0f , (controllerData->ly - 128) / 128.0f };
        struct vec2 rightAnalog = { (controllerData->rx - 128) / 128.0f , (controllerData->ry - 128) / 128.0f };

        if (vectorLenSquared2D(leftAnalog) < 0.1f) leftAnalog = (struct vec2) {0};
        else leftAnalog = vectorScale2D(data.dt, leftAnalog);
        if (vectorLenSquared2D(rightAnalog) < 0.1f) rightAnalog = (struct vec2) {0};
        else rightAnalog = vectorScale2D(data.dt, rightAnalog);

        rotation->rotation.x += rightAnalog.x * rotation->rotationSpeed;
        rotation->rotation.y += rightAnalog.y * rotation->rotationSpeed;
        rotation->rotation.y = clamp(rotation->rotation.y, -0.95f * M_PI_2, 0.95f * M_PI_2);

        struct quat xQuat = quatFromAxisAngle((struct vec3) { 0, -1, 0 }, rotation->rotation.x);
        struct quat yQuat = quatFromAxisAngle((struct vec3) { -1, 0, 0 }, rotation->rotation.y);
        transform->rotation = quatMult(xQuat, yQuat);

        struct vec3 dir = vectorRotate((struct vec3) { 0, 0, 1 }, transform->rotation);
        transform->position = vectorAdd(transform->position, vectorScale(leftAnalog.y * rotation->moveSpeed, dir));

        struct vec3 right = vectorRotate((struct vec3) { 1, 0, 0 }, transform->rotation);
        transform->position = vectorAdd(transform->position, vectorScale(leftAnalog.x * rotation->moveSpeed, right));

        struct vec3 up = { 0, 1, 0 };
        transform->position = vectorAdd(transform->position, vectorScale(controllerData->rb * data.dt * rotation->moveSpeed, up));
        transform->position = vectorSubtract(transform->position, vectorScale(controllerData->lb * data.dt * rotation->moveSpeed, up));
    }
}
