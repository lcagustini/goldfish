#include <ecs/systems.h>

#include <render/shadow.h>

#include <math/float.h>

#include <math.h>

static struct vec3 lightDirection(struct transformComponent *transform) {
    return vectorRotate((struct vec3) { 0, 0, -1 }, transform->rotation);
}

static int resolveResolution(int requested, int fallback) {
    if (requested <= 0) return fallback;
    if (requested < 256) return 256;
    if (requested > 4096) return 4096;
    return requested;
}

static void drawCasters(struct rendererDataComponent *rendererData) {
    for (int i = 0; i < rendererData->opaqueMeshesLength; i++) {
        struct meshRenderData *mesh = &rendererData->opaqueMeshes[i];
        if (mesh->castShadows) shadowDraw(mesh);
    }

    for (int i = 0; i < rendererData->transparentMeshesLength; i++) {
        struct meshRenderData *mesh = &rendererData->transparentMeshes[i];
        if (mesh->castShadows) shadowDraw(mesh);
    }
}

static struct vec3 upForDirection(struct vec3 direction) {
    // lookAt crosses dir with up; a parallel up gives a zero matrix.
    if (fabsf(vectorDot(direction, (struct vec3) { 0, 1, 0 })) > 0.99f) {
        return (struct vec3) { 0, 0, 1 };
    }
    return (struct vec3) { 0, 1, 0 };
}

// The last valid camera owns the shadow maps for the frame.
static struct rendererDataComponent *shadowCamera(struct systemRunData data) {
    struct rendererDataComponent *rendererDatas = GET_SYSTEM_COMPONENTS(data, 0, 0);
    struct rendererDataComponent *camera = NULL;

    for (int i = 0; i < GET_SYSTEM_COMPONENTS_LENGTH(data, 0); i++) {
        if (rendererDatas[i].cameraValid) camera = &rendererDatas[i];
    }

    return camera;
}

// Practical split scheme from the article: blend log and uniform splits.
static float splitDistance(float near, float far, float lambda, int index, int count) {
    float p = (float)index / (float)count;
    float logSplit = near * powf(far / near, p);
    float uniformSplit = near + (far - near) * p;
    return lambda * logSplit + (1.0f - lambda) * uniformSplit;
}

static void frustumCorners(struct rendererDataComponent *camera, float near, float far, struct vec3 corners[8]) {
    int index = 0;

    for (int side = 0; side < 2; side++) {
        float distance = side == 0 ? near : far;
        struct vec3 center = vectorAdd(camera->cameraPosition, vectorScale(distance, camera->cameraForward));

        float halfHeight;
        float halfWidth;
        if (camera->cameraOrthographic) {
            halfHeight = camera->cameraOrthoSize;
            halfWidth = camera->cameraOrthoSize * camera->cameraAspect;
        } else {
            float tangent = tanf(camera->cameraFov * (float)M_PI / 360.0f);
            halfHeight = tangent * distance;
            halfWidth = halfHeight * camera->cameraAspect;
        }

        struct vec3 right = vectorScale(halfWidth, camera->cameraRight);
        struct vec3 up = vectorScale(halfHeight, camera->cameraUp);

        corners[index++] = vectorAdd(center, vectorAdd(right, up));
        corners[index++] = vectorAdd(center, vectorAdd(vectorScale(-1.0f, right), up));
        corners[index++] = vectorAdd(center, vectorAdd(right, vectorScale(-1.0f, up)));
        corners[index++] = vectorAdd(center, vectorAdd(vectorScale(-1.0f, right), vectorScale(-1.0f, up)));
    }
}

static void fitSphere(struct vec3 corners[8], struct vec3 *center, float *radius) {
    struct vec3 sum = { 0, 0, 0 };
    for (int i = 0; i < 8; i++) sum = vectorAdd(sum, corners[i]);
    *center = vectorScale(1.0f / 8.0f, sum);

    float radiusSquared = 0;
    for (int i = 0; i < 8; i++) {
        struct vec3 offset = vectorSubtract(corners[i], *center);
        float distanceSquared = vectorDot(offset, offset);
        if (distanceSquared > radiusSquared) radiusSquared = distanceSquared;
    }

    *radius = sqrtf(radiusSquared);
}

static void buildCascade(struct vec3 direction, struct vec3 corners[8], int resolution, float padding, union mat4 *matrix) {
    struct vec3 center;
    float radius;
    fitSphere(corners, &center, &radius);

    // The light vector points toward the light; the shadow camera looks along its negation.
    struct vec3 forward = vectorScale(-1.0f, direction);
    struct vec3 up = upForDirection(forward);
    struct vec3 right = vectorNormalize(vectorCross(forward, up));
    up = vectorCross(right, forward);

    // Snap the center to whole texels so the map does not shimmer.
    float texel = 2.0f * radius / (float)resolution;
    float centerX = vectorDot(right, center);
    float centerY = vectorDot(up, center);
    float snappedX = roundf(centerX / texel) * texel;
    float snappedY = roundf(centerY / texel) * texel;
    center = vectorAdd(center, vectorAdd(vectorScale(snappedX - centerX, right), vectorScale(snappedY - centerY, up)));

    // The snap moves the center by less than one texel; pad the radius for it.
    radius += texel;

    float distance = radius + padding;
    struct vec3 lightPosition = vectorAdd(center, vectorScale(-distance, forward));

    union mat4 view, projection;
    lookAt(&view, lightPosition, forward, up);
    createOrthoProjectionMatrix(&projection, radius, 1.0f, 0.0f, distance + radius + padding);
    multMatrix(matrix, &view, &projection);
}

void rendererDirShadowPass(struct systemRunData data) {
    struct rendererDataComponent *camera = shadowCamera(data);

    struct transformComponent *transforms = GET_SYSTEM_COMPONENTS(data, 1, 0);
    struct dirLightComponent *lights = GET_SYSTEM_COMPONENTS(data, 1, 1);

    for (int i = 0; i < GET_SYSTEM_COMPONENTS_LENGTH(data, 1); i++) {
        struct dirLightComponent *light = &lights[i];
        light->shadowCascadeCount = 0;
        for (int c = 0; c < MAX_SHADOW_CASCADES; c++) light->shadowIndices[c] = -1;

        if (!light->castShadows || camera == NULL) continue;

        float near = camera->cameraNear > 0 ? camera->cameraNear : 0.1f;
        float far = light->shadowDistance > 0 ? light->shadowDistance : 50.0f;
        if (far > camera->cameraFar) far = camera->cameraFar;
        if (far <= near) continue;

        int cascadeCount = light->shadowCascades;
        if (cascadeCount <= 0) cascadeCount = MAX_SHADOW_CASCADES;
        if (cascadeCount > MAX_SHADOW_CASCADES) cascadeCount = MAX_SHADOW_CASCADES;

        int resolution = resolveResolution(light->shadowResolution, 2048);

        // Claim the maps first. If the pool is short, use the maps we got.
        int acquired = 0;
        for (; acquired < cascadeCount; acquired++) {
            int slot = shadowAcquire(SHADOW_KIND_2D, resolution);
            if (slot < 0) break;
            light->shadowIndices[acquired] = slot;
        }
        if (acquired == 0) continue;
        cascadeCount = acquired;

        float lambda = light->shadowSplitLambda;
        if (lambda < 0) lambda = 0;
        if (lambda > 1) lambda = 1;

        float padding = light->shadowCasterPadding > 0 ? light->shadowCasterPadding : 10.0f;
        struct vec3 direction = lightDirection(&transforms[i]);

        for (int c = 0; c < cascadeCount; c++) {
            float splitNear = c == 0 ? near : light->shadowSplits[c - 1];
            float splitFar = splitDistance(near, far, lambda, c + 1, cascadeCount);
            light->shadowSplits[c] = splitFar;

            struct vec3 corners[8];
            frustumCorners(camera, splitNear, splitFar, corners);
            buildCascade(direction, corners, resolution, padding, &light->shadowMatrices[c]);
        }

        // Unused cascades must never win the split test.
        for (int c = cascadeCount; c < MAX_SHADOW_CASCADES; c++) {
            light->shadowSplits[c] = 1e30f;
            light->shadowIndices[c] = -1;
        }
        light->shadowCascadeCount = cascadeCount;

        for (int c = 0; c < cascadeCount; c++) {
            shadowBegin2D(light->shadowIndices[c], &light->shadowMatrices[c]);
            drawCasters(camera);
            shadowEnd();
        }
    }
}

void rendererSpotShadowPass(struct systemRunData data) {
    struct rendererDataComponent *camera = shadowCamera(data);

    struct transformComponent *transforms = GET_SYSTEM_COMPONENTS(data, 1, 0);
    struct spotLightComponent *lights = GET_SYSTEM_COMPONENTS(data, 1, 1);

    for (int i = 0; i < GET_SYSTEM_COMPONENTS_LENGTH(data, 1); i++) {
        struct spotLightComponent *light = &lights[i];
        light->shadowIndex = -1;
        if (!light->castShadows || camera == NULL) continue;

        float range = light->range > 0 ? light->range : 25.0f;
        int resolution = resolveResolution(light->shadowResolution, 1024);

        int slot = shadowAcquire(SHADOW_KIND_2D, resolution);
        if (slot < 0) continue;

        struct vec3 direction = lightDirection(&transforms[i]);

        // The outer cone uses the smaller cosine.
        float outerCos = fminf(light->cutOff.x, light->cutOff.y);
        if (outerCos < -1.0f) outerCos = -1.0f;
        if (outerCos > 1.0f) outerCos = 1.0f;

        float fov = acosf(outerCos) * 2.0f * 180.0f / (float)M_PI;
        if (fov < 1.0f) fov = 1.0f;
        if (fov > 179.0f) fov = 179.0f;

        union mat4 view, projection;
        lookAt(&view, transforms[i].position, direction, upForDirection(direction));
        createPerspProjectionMatrix(&projection, fov, 1.0f, 0.1f, range);

        multMatrix(&light->shadowMatrix, &view, &projection);
        light->shadowIndex = slot;

        shadowBegin2D(slot, &light->shadowMatrix);
        drawCasters(camera);
        shadowEnd();
    }
}

void rendererPointShadowPass(struct systemRunData data) {
    static const struct vec3 faceDirections[6] = {
        { 1, 0, 0 }, { -1, 0, 0 },
        { 0, 1, 0 }, { 0, -1, 0 },
        { 0, 0, 1 }, { 0, 0, -1 },
    };
    static const struct vec3 faceUps[6] = {
        { 0, -1, 0 }, { 0, -1, 0 },
        { 0, 0, 1 }, { 0, 0, -1 },
        { 0, -1, 0 }, { 0, -1, 0 },
    };

    struct rendererDataComponent *camera = shadowCamera(data);

    struct transformComponent *transforms = GET_SYSTEM_COMPONENTS(data, 1, 0);
    struct pointLightComponent *lights = GET_SYSTEM_COMPONENTS(data, 1, 1);

    for (int i = 0; i < GET_SYSTEM_COMPONENTS_LENGTH(data, 1); i++) {
        struct pointLightComponent *light = &lights[i];
        light->shadowIndex = -1;
        if (!light->castShadows || camera == NULL) continue;

        float range = light->range > 0 ? light->range : 25.0f;
        int resolution = resolveResolution(light->shadowResolution, 512);

        int slot = shadowAcquire(SHADOW_KIND_CUBE, resolution);
        if (slot < 0) continue;

        struct vec3 position = transforms[i].position;

        union mat4 projection;
        createPerspProjectionMatrix(&projection, 90.0f, 1.0f, 1.0f, range);

        light->shadowIndex = slot;

        for (int face = 0; face < 6; face++) {
            union mat4 view, matrix;
            lookAt(&view, position, faceDirections[face], faceUps[face]);
            multMatrix(&matrix, &view, &projection);

            shadowBeginCube(slot, face, &matrix, position, range);
            drawCasters(camera);
            shadowEnd();
        }
    }
}
