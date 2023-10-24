#include <engineUI.h>

#include <fontIconsMaterialDesign.h>
#include <global.h>
#include <print.h>

#include <ecs/components.h>

#include <string.h>

struct guiState {
	bool hierarchyWindowOpened;
	bool systemsWindowOpened;
	bool componentsWindowOpened;
	bool tablesWindowOpened;
	bool rendererDataOpened;
	bool framebuffersWindowOpen;
};

static struct guiState guiState;

static const char *systemEventName(enum systemEvent event) {
	switch (event) {
		case SYSTEM_ON_COMPONENT_ADD:
			return "Component Add";
		default:
			return "";
	}
}

static const char *systemPhaseName(enum systemPhase phase) {
	switch (phase) {
		case SYSTEM_ON_PRE_UPDATE:
			return "Pre Update";
		case SYSTEM_ON_UPDATE:
			return "Update";
		case SYSTEM_ON_POST_UPDATE:
			return "Post Update";
		case SYSTEM_ON_RENDER_SORT:
			return "Render Sort";
		case SYSTEM_ON_RENDER_SETUP:
			return "Render Setup";
		case SYSTEM_ON_RENDER_SHADOW:
			return "Render Shadow";
		case SYSTEM_ON_RENDER_OPAQUE:
			return "Render Opaque";
		case SYSTEM_ON_RENDER_SKYBOX:
			return "Render Skybox";
		case SYSTEM_ON_RENDER_TRANSPARENT:
			return "Render Transparent";
		case SYSTEM_ON_RENDER_POST:
			return "Render Post";
		case SYSTEM_ON_RENDER_FINISH:
			return "Render Finish";
		default:
			return "";
	}
}

static void drawMenuBar(struct world *world) {
	if (igBeginMainMenuBar()) {
		if (igBeginMenu("World", true)) {
			if (igMenuItem_Bool("Entities", NULL, false, true)) { 
				guiState.hierarchyWindowOpened = true;
			}
			if (igMenuItem_Bool("Systems", NULL, false, true)) { 
				guiState.systemsWindowOpened = true;
			}
			if (igMenuItem_Bool("Components", NULL, false, true)) { 
				guiState.componentsWindowOpened = true;
			}
			if (igMenuItem_Bool("Tables", NULL, false, true)) { 
				guiState.tablesWindowOpened = true;
			}
			igEndMenu();
		}
		if (igBeginMenu("Render", true)) {
			if (igMenuItem_Bool("Renderer Data", NULL, false, true)) { 
				guiState.rendererDataOpened = true;
			}
			if (igMenuItem_Bool("Framebuffers", NULL, false, true)) { 
				guiState.framebuffersWindowOpen = true;
			}
			igEndMenu();
		}
		igEndMainMenuBar();
	}
}

static void drawRendererData(struct world *world) {
	if (guiState.rendererDataOpened) {
		if (igBegin("Renderer Data", &guiState.rendererDataOpened, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize)) {
			struct filter *filter = getFilter(world, "rendererDataFilter");

			for (int i = 0; i < filter->resultsLength; i++) {
				unsigned int rendererDatasLength;
				struct rendererDataComponent *rendererDatas = getComponentsFromTable(world, filter->results[i], GET_COMPONENT_ID(struct rendererDataComponent), &rendererDatasLength);

				for (int j = 0; j < rendererDatasLength; j++) {
					struct rendererDataComponent *renderer = &rendererDatas[j];

					char rendererName[30] = { 0 };
					sprintf(rendererName, "%d %d (%u)", i, j, renderer->target);

					if (igTreeNode_Str(rendererName)) {
						if (igTreeNode_Str("Opaques")) {
							for (int k = 0; k < renderer->opaqueMeshesLength; k++) {
								struct meshRenderData *meshData = &renderer->opaqueMeshes[k];

								char meshName[100] = { 0 };
								sprintf(meshName, "shader: %u - VAO: %u - indices: %u\n", meshData->shader.program, meshData->VAO, meshData->indicesLength);

								if (igTreeNode_Str(meshName)) {
									if (igTreeNode_Str("Uniforms")) {
										for (int l = 0; l < meshData->uniformsLength; l++) {
											struct uniformRenderData *uniformData = &meshData->uniforms[l];
											igText("uniform (type: %d) (loc: %u) (count: %u) (data: %p)", uniformData->type, uniformData->location, uniformData->count, uniformData->data);
										}
										igTreePop();
									}
									if (igTreeNode_Str("Textures")) {
										for (int l = 0; l < meshData->texturesLength; l++) {
											struct textureRenderData *textureData = &meshData->textures[l];
											igText("texture (type: %d) (slot: %u) (buffer: %u)\n", textureData->type, textureData->slot, textureData->buffer);
										}
										igTreePop();
									}
									igTreePop();
								}
							}
							igTreePop();
						}
						if (igTreeNode_Str("Transparents")) {
							for (int k = 0; k < renderer->transparentMeshesLength; k++) {
								struct meshRenderData *meshData = &renderer->transparentMeshes[k];

								char meshName[100] = { 0 };
								sprintf(meshName, "shader: %u - VAO: %u - indices: %u\n", meshData->shader.program, meshData->VAO, meshData->indicesLength);

								if (igTreeNode_Str(meshName)) {
									if (igTreeNode_Str("Uniforms")) {
										for (int l = 0; l < meshData->uniformsLength; l++) {
											struct uniformRenderData *uniformData = &meshData->uniforms[l];
											igText("uniform (type: %d) (loc: %u) (count: %u) (data: %p)", uniformData->type, uniformData->location, uniformData->count, uniformData->data);
										}
										igTreePop();
									}
									if (igTreeNode_Str("Textures")) {
										for (int l = 0; l < meshData->texturesLength; l++) {
											struct textureRenderData *textureData = &meshData->textures[l];
											igText("texture (type: %d) (slot: %u) (buffer: %u)\n", textureData->type, textureData->slot, textureData->buffer);
										}
										igTreePop();
									}
									igTreePop();
								}
							}
							igTreePop();
						}
						igTreePop();
					}
				}
			}
		}
		igEnd();
	}
}

static void drawSystems(struct world *world) {
	if (guiState.systemsWindowOpened) {
		if (igBegin("Systems", &guiState.systemsWindowOpened, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize)) {
			for (int j = 0; j < SYSTEM_PHASE_MAX; j++) {
				if (world->phaseSystems[j].bufferCount == 0) continue;

				if (igTreeNode_Str(systemPhaseName(j))) {
					for (int i = 0; i < world->phaseSystems[j].bufferCount; i++) {
						struct system *system = dynarrayGet(&world->phaseSystems[j], i);

						if (igTreeNode_Str(system->name)) {
							for (int l = 0; l < system->filtersLength; l++) {
								struct filter *filter = hashtableGet(&world->filters, system->filters[l]);

								char filterName[10] = { 0 };
								sprintf(filterName, "%d", l);

								if (igTreeNode_Str(filterName)) {
									for (int k = 0; k < filter->componentsLength; k++) {
										igText(filter->components[k]);
									}
									igTreePop();
								}
							}
							igTreePop();
						}
					}
					igTreePop();
				}
			}

			igSeparator();

			for (int j = 0; j < SYSTEM_EVENT_MAX; j++) {
				if (world->eventSystems[j].bufferCount == 0) continue;

				if (igTreeNode_Str(systemEventName(j))) {
					for (int i = 0; i < world->eventSystems[j].bufferCount; i++) {
						struct system *system = dynarrayGet(&world->eventSystems[j], i);

						if (igTreeNode_Str(system->name)) {
							for (int l = 0; l < system->filtersLength; l++) {
								struct filter *filter = hashtableGet(&world->filters, system->filters[l]);

								char filterName[10] = { 0 };
								sprintf(filterName, "%d", l);

								if (igTreeNode_Str(filterName)) {
									for (int k = 0; k < filter->componentsLength; k++) {
										igText(filter->components[k]);
									}
									igTreePop();
								}
							}
							igTreePop();
						}
					}
					igTreePop();
				}
			}
		}
		igEnd();
	}
}

static void drawComponents(struct world *world) {
	if (guiState.componentsWindowOpened) {
		if (igBegin("Components", &guiState.componentsWindowOpened, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize)) {
			struct component *components = world->components.buffer;
			for (int i = 0; i < world->components.bufferCount; i++) {
				if (!world->components.valids[i]) continue;

				igText("%s (%u bytes)", components[i].name, components[i].size);
			}
		}
		igEnd();
	}
}

static void drawTables(struct world *world) {
	if (guiState.tablesWindowOpened) {
		if (igBegin("Tables", &guiState.tablesWindowOpened, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize)) {
			for (int i = 0; i < world->tables.bufferCount; i++) {
				struct table *table = dynarrayGet(&world->tables, i);

				char tableName[40] = { 0 };
				sprintf(tableName, "%d (%u entities)", i, table->componentsLength);

				if (igTreeNode_Str(tableName)) {
					for (int j = 0; j < table->recordsLength; j++) {
						igText(table->records[j].componentType);
					}
					igTreePop();
				}
			}
		}
		igEnd();
	}
}

static void drawHierarchy(struct world *world) {
	componentId transformComponentId = GET_COMPONENT_ID(struct transformComponent);
	tableId tables[256];
	unsigned int tablesLength = getAllTablesWithComponents(world, &transformComponentId, 1, tables, 256);

	entityId queue[1024] = { 0 };
	unsigned int queueLength = 0;

	for (int i = 0; i < tablesLength; i++) {
		unsigned int length = (((struct table *)dynarrayGet(&world->tables, tables[i]))->componentsLength);
		for (int j = 0; j < length; j++) {
			queue[queueLength++] = searchForEntity(world, tables[i], j);
		}
	}

	entityId drawEntities[1024];
	int depth[1024];
	unsigned int drawEntitiesLength = 0;

	int j = 0;
	while (queueLength > 0) {
		struct transformComponent *t = GET_COMPONENT(world, queue[j], struct transformComponent);

		if (t->parent == NULL) {
			depth[drawEntitiesLength] = 0;
			drawEntities[drawEntitiesLength++] = queue[j];
			goto removeFromQueue;
		}
		else {
			for (int i = 0; i < drawEntitiesLength; i++) {
				if (strcmp(t->parent, drawEntities[i]) == 0) {
					for (int k = drawEntitiesLength; k > i; k--) {
						drawEntities[k] = drawEntities[k - 1];
						depth[k] = depth[k - 1];
					}
					depth[i + 1] = depth[i] + 1;
					drawEntities[i + 1] = queue[j];
					drawEntitiesLength++;
					goto removeFromQueue;
				}
			}

			j = (j + 1) % queueLength;
			continue;
		}

removeFromQueue:
		for (int i = j; i < queueLength - 1; i++) {
			queue[i] = queue[i + 1];
		}
		queueLength--;
		if (queueLength > 0) j = (j + 1) % queueLength;
	}

	if (guiState.hierarchyWindowOpened) {
		if (igBegin("Entities", &guiState.hierarchyWindowOpened, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize)) {
			int currentLevel = 0;
			for (int i = 0; i < drawEntitiesLength; i++) {
				int diff = currentLevel - depth[i];
				for (int j = 0; j < diff; j++) {
					igTreePop();
					currentLevel--;
				}
				if (currentLevel < depth[i]) continue;

				if (i + 1 < drawEntitiesLength && depth[i + 1] > depth[i]) {
					if (igTreeNode_Str(drawEntities[i])) {
						currentLevel++;
					}
				}
				else {
					igIndent(0);
					igText(drawEntities[i]);
					igUnindent(0);
				}
			}
			int diff = currentLevel - depth[drawEntitiesLength - 1];
			for (int j = 0; j < diff; j++) {
				igTreePop();
			}

			igSeparator();

			for (int i = 0; i < world->entities.bufferCount; i++) {
				if (!world->entities.valids[i]) continue;

				bool found = false;
				for (int j = 0; j < drawEntitiesLength; j++) {
					if (strcmp(world->entities.keys[i], drawEntities[j]) == 0) {
						found = true;
						break;
					}
				}

				if (!found) {
					igText(world->entities.keys[i]);
				}
			}
		}
		igEnd();
	}
}

static void drawFramebuffers() {
	if (guiState.framebuffersWindowOpen) {
		if (igBegin("Framebuffers", &guiState.framebuffersWindowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize)) {
			for (int i = 0; i < MAX_FRAMEBUFFERS; i++) {
				igText("(valid: %d) (scaleToWindow: %d) (color: %u) (FBO: %u) (RBO: %u)", activeFramebuffers[i].valid, activeFramebuffers[i].scaleToWindow, activeFramebuffers[i].colorBuffer, activeFramebuffers[i].FBO, activeFramebuffers[i].RBO);
			}
		}
		igEnd();
	}
}

void drawEngineUI(struct world *world) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	igNewFrame();

	drawMenuBar(world);
	drawSystems(world);
	drawComponents(world);
	drawTables(world);
	drawRendererData(world);
	drawHierarchy(world);
	drawFramebuffers();

	igRender();
	ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}
