#include <ecs/systems.h>

#include <render/framebuffer.h>
#include <render/shader.h>

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

		rendererData->target = camera->framebuffer;

        glBindFramebuffer(GL_FRAMEBUFFER, activeFramebuffers[camera->framebuffer].FBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
}
