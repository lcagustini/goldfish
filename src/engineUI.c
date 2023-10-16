#include <engineUI.h>

#include <fontIconsMaterialDesign.h>
#include <global.h>

#include <ecs/components.h>

#include <string.h>

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

	if (igBegin(ICON_MD_FORMAT_LIST_BULLETED " Hierarchy", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
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

void drawEngineUI(struct world *world) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	igNewFrame();

	drawHierarchy(world);

	igRender();
	ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}
