#include <ecs/systems.h>

#include <render/framebuffer.h>
#include <print.h>

static GLuint shader = -1;

static void loadLightShader() {
    GLuint vshader = loadShaderFromFile("assets/shaders/light_v.glsl", GL_VERTEX_SHADER);
    GLuint fshader = loadShaderFromFile("assets/shaders/light_f.glsl", GL_FRAGMENT_SHADER);

    shader = glCreateProgram();
    if (shader) {
        glAttachShader(shader, vshader);
        glAttachShader(shader, fshader);
        glLinkProgram(shader);
    }
}

void renderShadowDepthMap(struct systemRunData data) {
    struct transformComponent *transforms = GET_SYSTEM_COMPONENTS(data, 0, 0);
    struct dirLightComponent *lights = GET_SYSTEM_COMPONENTS(data, 0, 1);

    struct rendererDataComponent *rendererDatas = GET_SYSTEM_COMPONENTS(data, 1, 0);

    if (shader == -1) loadLightShader();

	glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

    for (int i = 0; i < GET_SYSTEM_COMPONENTS_LENGTH(data, 0); i++) {
		struct transformComponent *transform = &transforms[i];
		struct dirLightComponent *light = &lights[i];

		glBindFramebuffer(GL_FRAMEBUFFER, activeFramebuffers[light->shadowDepthMap].FBO);
		glViewport(0, 0, 1024, 1024);
        glClear(GL_DEPTH_BUFFER_BIT);

        union mat4 lightProjection;
        int width, height;
		glfwGetWindowSize(globalState.window, &width, &height);
		createOrthoProjectionMatrix(&lightProjection, 2.0f, (float)width / (float)height, 1, 10);

        union mat4 lightView;
        struct vec3 dir = vectorRotate((struct vec3) { 0, 0, -1 }, transform->rotation);
		lookAt(&lightView, transform->position, dir, (struct vec3) { 0, 1, 0 });

        for (int k = 0; k < GET_SYSTEM_COMPONENTS_LENGTH(data, 1); k++) {
			struct rendererDataComponent *rendererData = &rendererDatas[k];

			for (int j = 0; j < rendererData->opaqueMeshesLength; j++) {
				struct meshRenderData mesh = rendererData->opaqueMeshes[j];

				glUseProgram(shader);
				glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, &lightView.a1);
				glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, &lightProjection.a1);

				for (int k = 0; k < mesh.uniformsLength; k++) {
					struct uniformRenderData uniform = mesh.uniforms[k];

					switch (uniform.type) {
						case RENDERER_UNIFORM_MATRIX_4:
							glUniformMatrix4fv(uniform.location, uniform.count, false, &uniform.data.matrix.a1);
							break;
						case RENDERER_UNIFORM_VECTOR_3:
							glUniform3fv(uniform.location, uniform.count, &uniform.data.vector3.x);
							break;
						case RENDERER_UNIFORM_VECTOR_2:
							glUniform2fv(uniform.location, uniform.count, &uniform.data.vector2.x);
							break;
						case RENDERER_UNIFORM_INT:
							glUniform1i(uniform.location, uniform.data.integer);
							break;
						case RENDERER_UNIFORM_FLOAT:
							glUniform1f(uniform.location, uniform.data.single);
							break;
					}
				}

				glBindVertexArray(mesh.VAO);

				for (int k = 0; k < mesh.texturesLength; k++) {
					struct textureRenderData texture = mesh.textures[k];

					glActiveTexture(texture.slot);
					switch (texture.type) {
						case RENDERER_TEXTURE_2D:
							glBindTexture(GL_TEXTURE_2D, texture.buffer);
							break;
						case RENDERER_TEXTURE_CUBE_MAP:
							glBindTexture(GL_TEXTURE_CUBE_MAP, texture.buffer);
							break;
					}
				}

				glDrawElements(GL_TRIANGLES, mesh.indicesLength, GL_UNSIGNED_INT, 0);

				glBindVertexArray(0);
            }
        }
    }
}
