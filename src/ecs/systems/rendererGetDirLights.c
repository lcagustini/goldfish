#include <ecs/systems.h>

#include <stdio.h>

static void setupDirLight(struct transformComponent *transform, struct dirLightComponent *light, struct meshRenderData *renderData, int i, int count) {
	struct vec3 direction = (struct vec3) { 0, 0, -1 };
	direction = vectorRotate(direction, transform->rotation);

	char name[64];
	sprintf(name, "dirLights[%d].direction", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_VECTOR_3, name, &direction);

	sprintf(name, "dirLights[%d].ambientColor", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_VECTOR_3, name, &light->ambientColor);

	sprintf(name, "dirLights[%d].diffuseColor", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_VECTOR_3, name, &light->diffuseColor);

	sprintf(name, "dirLights[%d].specularColor", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_VECTOR_3, name, &light->specularColor);

	shaderPushUniform(renderData, RENDERER_UNIFORM_INT, "dirLightsLength", &count);
}

void rendererGetDirLights(struct systemRunData data) {
    struct transformComponent *transforms = GET_SYSTEM_COMPONENTS(data, 0, 0);
    struct dirLightComponent *lights = GET_SYSTEM_COMPONENTS(data, 0, 1);

    struct rendererDataComponent *rendererDatas = GET_SYSTEM_COMPONENTS(data, 1, 0);

    for (int i = 0; i < GET_SYSTEM_COMPONENTS_LENGTH(data, 0); i++) {
		struct transformComponent *transform = &transforms[i];
		struct dirLightComponent *light = &lights[i];

        for (int k = 0; k < GET_SYSTEM_COMPONENTS_LENGTH(data, 1); k++) {
			struct rendererDataComponent *rendererData = &rendererDatas[k];

			for (int j = 0; j < rendererData->opaqueMeshesLength; j++) {
				struct meshRenderData *renderData = &rendererData->opaqueMeshes[j];
				setupDirLight(transform, light, renderData, i, GET_SYSTEM_COMPONENTS_LENGTH(data, 0));
			}

			for (int j = 0; j < rendererData->transparentMeshesLength; j++) {
				struct meshRenderData *renderData = &rendererData->transparentMeshes[j];
				setupDirLight(transform, light, renderData, i, GET_SYSTEM_COMPONENTS_LENGTH(data, 0));
			}
        }
    }
}
