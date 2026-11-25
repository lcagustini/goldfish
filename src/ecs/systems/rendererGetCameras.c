#include <ecs/systems.h>

#include <render/framebuffer.h>
#include <render/shader.h>

#include <math.h>

static void setupCameraUniforms(struct transformComponent *transform, struct cameraComponent *camera, struct meshRenderData *renderData) {
	shaderPushUniform(renderData, RENDERER_UNIFORM_MATRIX_4, "view", &camera->viewMat);
	shaderPushUniform(renderData, RENDERER_UNIFORM_MATRIX_4, "projection", &camera->projectionMat);
	shaderPushUniform(renderData, RENDERER_UNIFORM_VECTOR_3, "viewPos", &transform->position);
}

void rendererGetCameras(struct systemRunData data) {
    struct transformComponent *transforms = GET_SYSTEM_COMPONENTS(data, 0, 0);
    struct cameraComponent *cameras = GET_SYSTEM_COMPONENTS(data, 0, 1);
    struct rendererDataComponent *rendererDatas = GET_SYSTEM_COMPONENTS(data, 0, 2);

    for (int i = 0; i < GET_SYSTEM_COMPONENTS_LENGTH(data, 0); i++) {
        struct transformComponent *transform = &transforms[i];
        struct cameraComponent *camera = &cameras[i];
        struct rendererDataComponent *rendererData = &rendererDatas[i];

        for (int j = 0; j < rendererData->opaqueMeshesLength; j++) {
			struct meshRenderData *renderData = &rendererData->opaqueMeshes[j];
			setupCameraUniforms(transform, camera, renderData);
        }

        for (int j = 0; j < rendererData->transparentMeshesLength; j++) {
			struct meshRenderData *renderData = &rendererData->transparentMeshes[j];
			setupCameraUniforms(transform, camera, renderData);
        }

		struct vec3 forward = vectorRotate((struct vec3) { 0, 0, -1 }, transform->rotation);
		struct vec3 worldUp = { 0, 1, 0 };
		// A camera that looks straight up makes the cross product zero.
		if (fabsf(vectorDot(forward, worldUp)) > 0.99f) worldUp = (struct vec3) { 0, 0, 1 };
		struct vec3 right = vectorNormalize(vectorCross(forward, worldUp));

		rendererData->cameraForward = forward;
		rendererData->cameraRight = right;
		rendererData->cameraUp = vectorCross(right, forward);
		rendererData->cameraFov = camera->fov;
		rendererData->cameraAspect = camera->projectionMat.mat[0][0] != 0 ? camera->projectionMat.mat[1][1] / camera->projectionMat.mat[0][0] : 1.0f;
		rendererData->cameraNear = camera->near;
		rendererData->cameraFar = camera->far;
		rendererData->cameraOrthoSize = camera->size;
		rendererData->cameraOrthographic = camera->orthographic;

		rendererData->target = camera->framebuffer;
		rendererData->cameraPosition = transform->position;
		rendererData->cameraValid = true;

        glBindFramebuffer(GL_FRAMEBUFFER, activeFramebuffers[camera->framebuffer].FBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
}
