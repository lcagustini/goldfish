#include <ecs/systems.h>

#include <stdio.h>

static void setupSpotLight(struct transformComponent *transform, struct spotLightComponent *light, struct meshRenderData *renderData, int i, int count) {
	struct vec3 direction = (struct vec3) { 0, 0, -1 };
	direction = vectorRotate(direction, transform->rotation);

	char name[64];
	sprintf(name, "spotLights[%d].position", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_VECTOR_3, name, &transform->position);

	sprintf(name, "spotLights[%d].direction", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_VECTOR_3, name, &direction);

	sprintf(name, "spotLights[%d].cutOff", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_VECTOR_2, name, &light->cutOff);

	sprintf(name, "spotLights[%d].ambientColor", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_VECTOR_3, name, &light->ambientColor);

	sprintf(name, "spotLights[%d].diffuseColor", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_VECTOR_3, name, &light->diffuseColor);

	sprintf(name, "spotLights[%d].specularColor", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_VECTOR_3, name, &light->specularColor);

	shaderPushUniform(renderData, RENDERER_UNIFORM_INT, "spotLightsLength", &count);
}

void rendererGetSpotLights(struct systemRunData data) {
    struct transformComponent *transforms = GET_SYSTEM_COMPONENTS(data, 0, 0);
    struct spotLightComponent *lights = GET_SYSTEM_COMPONENTS(data, 0, 1);

    struct rendererDataComponent *rendererDatas = GET_SYSTEM_COMPONENTS(data, 1, 0);

    for (int i = 0; i < GET_SYSTEM_COMPONENTS_LENGTH(data, 0); i++) {
		struct transformComponent *transform = &transforms[i];
		struct spotLightComponent *light = &lights[i];

        for (int k = 0; k < GET_SYSTEM_COMPONENTS_LENGTH(data, 1); k++) {
			struct rendererDataComponent *rendererData = &rendererDatas[k];

			for (int j = 0; j < rendererData->opaqueMeshesLength; j++) {
				struct meshRenderData *renderData = &rendererData->opaqueMeshes[j];
				setupSpotLight(transform, light, renderData, i, GET_SYSTEM_COMPONENTS_LENGTH(data, 0));
			}

			for (int j = 0; j < rendererData->transparentMeshesLength; j++) {
				struct meshRenderData *renderData = &rendererData->transparentMeshes[j];
				setupSpotLight(transform, light, renderData, i, GET_SYSTEM_COMPONENTS_LENGTH(data, 0));
			}
        }
    }
}
