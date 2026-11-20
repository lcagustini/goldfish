#include <ecs/systems.h>

#include <stdio.h>

static void setupPointLight(struct transformComponent *transform, struct pointLightComponent *light, struct meshRenderData *renderData, int i, int count) {
	char name[64];
	sprintf(name, "pointLights[%d].position", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_VECTOR_3, name, &transform->position);

	sprintf(name, "pointLights[%d].attenuation", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_VECTOR_3, name, &light->attenuation);

	sprintf(name, "pointLights[%d].ambientColor", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_VECTOR_3, name, &light->ambientColor);

	sprintf(name, "pointLights[%d].diffuseColor", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_VECTOR_3, name, &light->diffuseColor);

	sprintf(name, "pointLights[%d].specularColor", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_VECTOR_3, name, &light->specularColor);

	shaderPushUniform(renderData, RENDERER_UNIFORM_INT, "pointLightsLength", &count);
}

void rendererGetPointLights(struct systemRunData data) {
    struct transformComponent *transforms = GET_SYSTEM_COMPONENTS(data, 0, 0);
    struct pointLightComponent *lights = GET_SYSTEM_COMPONENTS(data, 0, 1);

    struct rendererDataComponent *rendererDatas = GET_SYSTEM_COMPONENTS(data, 1, 0);

    for (int i = 0; i < GET_SYSTEM_COMPONENTS_LENGTH(data, 0); i++) {
		struct transformComponent *transform = &transforms[i];
		struct pointLightComponent *light = &lights[i];

        for (int k = 0; k < GET_SYSTEM_COMPONENTS_LENGTH(data, 1); k++) {
			struct rendererDataComponent *rendererData = &rendererDatas[k];

			for (int j = 0; j < rendererData->opaqueMeshesLength; j++) {
				struct meshRenderData *renderData = &rendererData->opaqueMeshes[j];
				setupPointLight(transform, light, renderData, i, GET_SYSTEM_COMPONENTS_LENGTH(data, 0));
			}

			for (int j = 0; j < rendererData->transparentMeshesLength; j++) {
				struct meshRenderData *renderData = &rendererData->transparentMeshes[j];
				setupPointLight(transform, light, renderData, i, GET_SYSTEM_COMPONENTS_LENGTH(data, 0));
			}
        }
    }
}
