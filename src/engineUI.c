#include <goldfish/engineUI.h>

#include <goldfish/global.h>
#include <goldfish/input.h>
#include <goldfish/print.h>
#include <ecs/components.h>
#include <render/shader.h>

#include <fontIconsMaterialDesign.h>
#include <string.h>

struct guiState {
    bool hierarchyWindowOpened;
    bool systemsWindowOpened;
    bool componentsWindowOpened;
    bool tablesWindowOpened;
    bool rendererDataOpened;
    bool framebuffersWindowOpen;
    bool inputWindowOpened;
    bool inspectorWindowOpened;

    char selectedEntity[256];
};

static struct guiState guiState;

static void selectEntity(entityId entity) {
    snprintf(guiState.selectedEntity, sizeof(guiState.selectedEntity), "%s", entity);
    guiState.inspectorWindowOpened = true;
}

static bool entityAlive(struct world *world, const char *entity) {
    if (entity == NULL || entity[0] == '\0') return false;
    return hashtableGet(&world->entities, entity) != NULL;
}

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
        case SYSTEM_ON_LATE_UPDATE:
            return "Late Update";
        case SYSTEM_ON_POST_UPDATE:
            return "Post Update";
        case SYSTEM_ON_RENDER_SORT:
            return "Render Sort";
        case SYSTEM_ON_RENDER_SHADOW:
            return "Render Shadow";
        case SYSTEM_ON_RENDER_SETUP:
            return "Render Setup";
        case SYSTEM_ON_RENDER_OPAQUE:
            return "Render Opaque";
        case SYSTEM_ON_RENDER_SKYBOX:
            return "Render Skybox";
        case SYSTEM_ON_RENDER_TRANSPARENT:
            return "Render Transparent";
        case SYSTEM_ON_RENDER_FINISH:
            return "Render Finish";
        default:
            return "";
    }
}

static const char *componentFieldTypeName(enum componentFieldType type) {
    switch (type) {
        case COMPONENT_FIELD_BOOL: return "bool";
        case COMPONENT_FIELD_INT: return "int";
        case COMPONENT_FIELD_UINT: return "uint";
        case COMPONENT_FIELD_FLOAT: return "float";
        case COMPONENT_FIELD_DOUBLE: return "double";
        case COMPONENT_FIELD_VEC2: return "vec2";
        case COMPONENT_FIELD_VEC3: return "vec3";
        case COMPONENT_FIELD_QUAT: return "quat";
        case COMPONENT_FIELD_MAT4: return "mat4";
        case COMPONENT_FIELD_ENTITY: return "entity";
        case COMPONENT_FIELD_POINTER: return "pointer";
        case COMPONENT_FIELD_OPAQUE: return "opaque";
        default: return "unknown";
    }
}

static void drawMenuBar(struct world *world) {
    if (igBeginMainMenuBar()) {
        if (igBeginMenu("World", true)) {
            if (igMenuItem_Bool("Entities", NULL, false, true)) {
                guiState.hierarchyWindowOpened = true;
            }
            if (igMenuItem_Bool("Inspector", NULL, false, true)) {
                guiState.inspectorWindowOpened = true;
            }
            if (igMenuItem_Bool("Systems", NULL, false, true)) {
                guiState.systemsWindowOpened = true;
            }
            if (igMenuItem_Bool("Input", NULL, false, true)) {
                guiState.inputWindowOpened = true;
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
            if (igMenuItem_Bool("Reload Shaders", NULL, false, true)) {
                // After this frame's render phases: takes effect next frame.
                shaderHotReload();
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
                struct rendererDataComponent *rendererDatas = getComponentsFromTable(world, filter->results[i], GET_COMPONENT_ID(rendererDataComponent), &rendererDatasLength);

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

                char label[128];
                snprintf(label, sizeof(label), "%s (%u bytes, %u fields)", components[i].name, components[i].size, components[i].fieldsLength);

                if (igTreeNode_Str(label)) {
                    for (unsigned int j = 0; j < components[i].fieldsLength; j++) {
                        struct componentField *field = &components[i].fields[j];
                        igText("%s: %s%s (%u bytes)", field->name, componentFieldTypeName(field->type), field->count > 1 ? "[]" : "", field->elementSize);
                    }
                    igTreePop();
                }
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

static bool fieldTypeEditable(enum componentFieldType type) {
    switch (type) {
        case COMPONENT_FIELD_BOOL:
        case COMPONENT_FIELD_INT:
        case COMPONENT_FIELD_FLOAT:
        case COMPONENT_FIELD_DOUBLE:
        case COMPONENT_FIELD_VEC2:
        case COMPONENT_FIELD_VEC3:
        case COMPONENT_FIELD_QUAT:
        case COMPONENT_FIELD_ENTITY:
            return true;
        default:
            return false;
    }
}

static bool entityInParentChain(struct world *world, entityId entity, entityId candidate) {
    if (candidate == entity) return true;

    struct transformComponent *transform = GET_COMPONENT(world, candidate, transformComponent);
    while (transform != NULL && transform->parent != NULL) {
        if (transform->parent == entity) return true;
        transform = GET_COMPONENT(world, transform->parent, transformComponent);
    }
    return false;
}

static void drawEntityField(struct world *world, entityId owner, const char *label, entityId *value) {
    entityId current = *value;

    if (igBeginCombo(label, current != NULL ? current : "(none)", 0)) {
        if (igSelectable_Bool("(none)", current == NULL, 0, (ImVec2) { 0, 0 })) {
            *value = NULL;
        }

        struct entity *entities = world->entities.buffer;
        for (unsigned int i = 0; i < world->entities.bufferCount; i++) {
            if (!world->entities.valids[i]) continue;

            entityId candidate = entities[i].name;
            if (entityInParentChain(world, owner, candidate)) continue;

            if (igSelectable_Bool(candidate, candidate == current, 0, (ImVec2) { 0, 0 })) {
                *value = candidate;
            }
        }

        igEndCombo();
    }
}

static void drawFieldValue(struct world *world, entityId owner, const char *label, enum componentFieldType type, void *value) {
    switch (type) {
        case COMPONENT_FIELD_BOOL:
            igCheckbox(label, value);
            break;
        case COMPONENT_FIELD_INT:
            igDragScalar(label, ImGuiDataType_S32, value, 1.0f, NULL, NULL, "%d", 0);
            break;
        case COMPONENT_FIELD_UINT:
            igText("%s: %u", label, *(unsigned int *)value);
            break;
        case COMPONENT_FIELD_FLOAT:
            igDragScalar(label, ImGuiDataType_Float, value, 0.01f, NULL, NULL, "%.3f", 0);
            break;
        case COMPONENT_FIELD_DOUBLE:
            igDragScalar(label, ImGuiDataType_Double, value, 0.01f, NULL, NULL, "%.5f", 0);
            break;
        case COMPONENT_FIELD_VEC2:
            igDragFloat2(label, value, 0.01f, 0.0f, 0.0f, "%.3f", 0);
            break;
        case COMPONENT_FIELD_VEC3:
            igDragFloat3(label, value, 0.01f, 0.0f, 0.0f, "%.3f", 0);
            break;
        case COMPONENT_FIELD_QUAT:
            igDragFloat4(label, value, 0.01f, 0.0f, 0.0f, "%.3f", 0);
            break;
        case COMPONENT_FIELD_MAT4: {
            char title[160];
            snprintf(title, sizeof(title), "%s (matrix)", label);
            if (igTreeNode_Str(title)) {
                union mat4 *matrix = value;
                for (int row = 0; row < 4; row++) {
                    igText("%.3f %.3f %.3f %.3f", matrix->mat[row][0], matrix->mat[row][1], matrix->mat[row][2], matrix->mat[row][3]);
                }
                igTreePop();
            }
            break;
        }
        case COMPONENT_FIELD_ENTITY:
            drawEntityField(world, owner, label, value);
            break;
        case COMPONENT_FIELD_POINTER:
            igText("%s: %p", label, *(void **)value);
            break;
        case COMPONENT_FIELD_OPAQUE:
        default:
            igTextDisabled("%s (opaque)", label);
            break;
    }
}

static void drawComponentField(struct world *world, entityId entity, void *data, struct componentField *field) {
    void *values = (char *)data + field->offset;

    igPushID_Str(field->name);

    if (field->count > 1) {
        if (field->count <= 16 && fieldTypeEditable(field->type)) {
            for (unsigned int i = 0; i < field->count; i++) {
                char label[160];
                snprintf(label, sizeof(label), "%s[%u]", field->name, i);

                igPushID_Int((int)i);
                drawFieldValue(world, entity, label, field->type, (char *)values + i * field->elementSize);
                igPopID();
            }
        }
        else {
            igTextDisabled("%s: %u elements, read-only", field->name, field->count);
        }
    }
    else {
        drawFieldValue(world, entity, field->name, field->type, values);
    }

    igPopID();
}

static const char *drawAddComponent(struct world *world, entityId entity) {
    const char *chosen = NULL;

    if (igBeginCombo("Add component", "Select...", 0)) {
        struct component *components = world->components.buffer;
        for (unsigned int i = 0; i < world->components.bufferCount; i++) {
            if (!world->components.valids[i]) continue;

            const char *name = components[i].name;
            if (getComponent(world, entity, (componentId)name) != NULL) continue;

            if (igSelectable_Bool(name, false, 0, (ImVec2) { 0, 0 })) {
                chosen = name;
            }
        }
        igEndCombo();
    }

    return chosen;
}

static void drawInspector(struct world *world) {
    if (!guiState.inspectorWindowOpened) return;

    if (!entityAlive(world, guiState.selectedEntity)) {
        guiState.selectedEntity[0] = '\0';
    }

    if (igBegin("Inspector", &guiState.inspectorWindowOpened, ImGuiWindowFlags_None)) {
        if (guiState.selectedEntity[0] == '\0') {
            igTextDisabled("Select an entity in the Entities window.");
        }
        else {
            igText("%s", guiState.selectedEntity);
            igSeparator();

            struct entity *entity = hashtableGet(&world->entities, guiState.selectedEntity);
            entityId selected = entity != NULL ? entity->name : NULL;
            const char *pendingRemove = NULL;
            const char *pendingAdd = NULL;

            if (entity != NULL && entity->table != INVALID_POSITION) {
                struct table *table = dynarrayGet(&world->tables, entity->table);

                for (int i = 0; i < table->recordsLength; i++) {
                    const char *componentName = table->records[i].componentType;
                    struct component *component = hashtableGet(&world->components, componentName);
                    void *data = getComponent(world, selected, (componentId)componentName);

                    if (data == NULL) continue;

                    igPushID_Str(componentName);

                    if (igTreeNodeEx_Str(componentName, ImGuiTreeNodeFlags_DefaultOpen)) {
                        if (component == NULL || component->fieldsLength == 0) {
                            igTextDisabled("%u bytes, no field metadata", component != NULL ? component->size : 0);
                        }
                        else {
                            for (unsigned int field = 0; field < component->fieldsLength; field++) {
                                drawComponentField(world, selected, data, &component->fields[field]);
                            }
                        }

                        if (igSmallButton("Remove")) {
                            pendingRemove = componentName;
                        }

                        igTreePop();
                    }

                    igPopID();
                }
            }
            else {
                igTextDisabled("No components.");
            }

            igSeparator();
            pendingAdd = drawAddComponent(world, selected);

            if (pendingRemove != NULL) {
                removeComponent(world, selected, (componentId)pendingRemove);
            }
            if (pendingAdd != NULL) {
                addComponent(world, selected, (componentId)pendingAdd);
            }
        }
    }
    igEnd();
}

static void drawHierarchy(struct world *world) {
    componentId transformComponentId = GET_COMPONENT_ID(transformComponent);
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
        struct transformComponent *t = GET_COMPONENT(world, queue[j], transformComponent);

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
                    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
                    if (strcmp(guiState.selectedEntity, drawEntities[i]) == 0) flags |= ImGuiTreeNodeFlags_Selected;

                    if (igTreeNodeEx_Str(drawEntities[i], flags)) {
                        currentLevel++;
                    }
                    if (igIsItemClicked(ImGuiMouseButton_Left)) {
                        selectEntity(drawEntities[i]);
                    }
                }
                else {
                    if (igSelectable_Bool(drawEntities[i], strcmp(guiState.selectedEntity, drawEntities[i]) == 0, 0, (ImVec2) { 0, 0 })) {
                        selectEntity(drawEntities[i]);
                    }
                }
            }
            for (int j = 0; j < currentLevel; j++) {
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
                    struct entity *entities = world->entities.buffer;
                    entityId id = entities[i].name;

                    if (igSelectable_Bool(id, strcmp(guiState.selectedEntity, id) == 0, 0, (ImVec2) { 0, 0 })) {
                        selectEntity(id);
                    }
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

static const char *inputActionTypeName(enum inputActionType type) {
    switch (type) {
        case INPUT_ACTION_BUTTON: return "button";
        case INPUT_ACTION_AXIS_1D: return "axis1d";
        case INPUT_ACTION_AXIS_2D: return "axis2d";
        default: return "";
    }
}

static const char *inputTargetName(enum inputAxisTarget target) {
    return target == INPUT_TARGET_Y ? "y" : "x";
}

static char inputStatus[256];

static void beginInputCapture(struct inputComponent *input, int actionIndex, int bindingIndex, const char *target) {
    struct inputAction *action = &input->actions[actionIndex];

    if (!inputBeginCapture(input, action->name, bindingIndex, target)) {
        snprintf(inputStatus, sizeof(inputStatus), "Capture failed: %s", inputLastError());
        return;
    }

    snprintf(inputStatus, sizeof(inputStatus),
        "Press a key, mouse button, gamepad button, stick, trigger, or wheel. Esc cancels.");
}

static void finishInputCapture(struct inputComponent *input) {
    struct inputCaptureState *capture = &input->capture;
    if (capture->actionIndex < 0) return;

    struct inputAction *action = &input->actions[capture->actionIndex];
    int device = capture->capturedDevice;

    if (action->name[0] == '\0' || device < 0) {
        capture->actionIndex = -1;
        capture->bindingIndex = -1;
        return;
    }

    enum inputDevice inputDevice = (enum inputDevice)device;
    const char *deviceName = inputDeviceName(inputDevice);
    const char *controlName = inputControlName(inputDevice, capture->capturedControl);
    const char *target = inputTargetName((enum inputAxisTarget)capture->capturedTarget);
    int player = capture->capturedPlayer;

    if (inputDevice == INPUT_DEVICE_GAMEPAD) {
        int mappedCount = 0;

        for (int i = 0; i < INPUT_MAX_GAMEPADS; i++) {
            if (input->gamepads[i].connected && input->gamepads[i].mapped) mappedCount++;
        }

        if (mappedCount == 1) player = -1;
    }

    int conflict = inputFindConflict(input, action->name, deviceName, controlName, target, player);
    if (conflict >= 0) {
        snprintf(inputStatus, sizeof(inputStatus),
            "%s %s is already bound to action '%s'.",
            deviceName, controlName, input->actions[conflict].name);
        capture->actionIndex = -1;
        capture->bindingIndex = -1;
        return;
    }

    bool ok;

    if (capture->bindingIndex >= 0) {
        struct inputBinding *binding = &action->bindings[capture->bindingIndex];
        float scale = binding->scale;
        float threshold = binding->threshold;
        target = inputTargetName(binding->target);

        ok = inputSetBinding(input, action->name, (unsigned int)capture->bindingIndex, deviceName,
            controlName, target, scale, player, threshold);
    }
    else {
        ok = inputBind(input, action->name, deviceName, controlName, target, 1.0f, player, 0.5f);
    }

    if (!ok) {
        snprintf(inputStatus, sizeof(inputStatus), "Bind failed: %s", inputLastError());
    }
    else {
        snprintf(inputStatus, sizeof(inputStatus), "Bound %s %s to action '%s'.",
            deviceName, controlName, action->name);
    }

    capture->actionIndex = -1;
    capture->bindingIndex = -1;
}

static void drawInput(struct inputComponent *input) {
    if (!guiState.inputWindowOpened) {
        if (input != NULL) {
            if (inputCaptureActive(input)) inputCancelCapture(input);
            input->capture.actionIndex = -1;
            input->capture.bindingIndex = -1;
        }
        return;
    }

    if (input == NULL) {
        if (igBegin("Input", &guiState.inputWindowOpened,
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize)) {
            igTextDisabled("The current scene has no inputComponent.");
        }
        igEnd();
        return;
    }

    if (inputCaptureActive(input) && inputCapturePoll(input)) finishInputCapture(input);

    if (igBegin("Input", &guiState.inputWindowOpened,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize)) {

        if (inputCaptureActive(input)) {
            igTextWrapped("Press any input. Esc cancels.");

            if (input->capture.actionIndex >= 0) {
                igText("Rebinding action: %s", input->actions[input->capture.actionIndex].name);
            }

            if (igSmallButton("Cancel capture")) {
                inputCancelCapture(input);
                input->capture.actionIndex = -1;
                input->capture.bindingIndex = -1;
                snprintf(inputStatus, sizeof(inputStatus), "Capture cancelled.");
            }

            igSeparator();
        }

        if (inputStatus[0] != '\0') {
            igTextWrapped("%s", inputStatus);
            igSeparator();
        }

        if (input->actionCount == 0) {
            igTextDisabled("No input actions in the current scene.");
        }

        for (unsigned int actionIndex = 0; actionIndex < input->actionCount; actionIndex++) {
            struct inputAction *action = &input->actions[actionIndex];

            char actionLabel[128];
            if (action->type == INPUT_ACTION_AXIS_2D) {
                snprintf(actionLabel, sizeof(actionLabel), "%s (%s, deadzone %.2f)",
                    action->name, inputActionTypeName(action->type), action->deadzone);
            }
            else {
                snprintf(actionLabel, sizeof(actionLabel), "%s (%s)",
                    action->name, inputActionTypeName(action->type));
            }

            igPushID_Int((int)actionIndex);

            if (igTreeNode_Str(actionLabel)) {
                for (unsigned int bindingIndex = 0; bindingIndex < action->bindingCount; bindingIndex++) {
                    struct inputBinding *binding = &action->bindings[bindingIndex];

                    const char *deviceName = inputDeviceName(binding->device);
                    const char *controlName = inputControlName(binding->device, binding->control);

                    char playerText[16];
                    if (binding->device == INPUT_DEVICE_GAMEPAD) {
                        if (binding->player < 0) snprintf(playerText, sizeof(playerText), "any");
                        else snprintf(playerText, sizeof(playerText), "%d", binding->player);
                    }
                    else {
                        snprintf(playerText, sizeof(playerText), "any");
                    }

                    char bindingText[192];
                    if (action->type == INPUT_ACTION_AXIS_2D) {
                        snprintf(bindingText, sizeof(bindingText), "%s %s -> %s (player %s)",
                            deviceName, controlName,
                            inputTargetName(binding->target), playerText);
                    }
                    else {
                        snprintf(bindingText, sizeof(bindingText), "%s %s (player %s)",
                            deviceName, controlName, playerText);
                    }

                    igPushID_Int((int)bindingIndex);
                    igText("%s", bindingText);
                    igSameLine(0, 8);

                    if (igSmallButton("Rebind")) {
                        beginInputCapture(input, (int)actionIndex, (int)bindingIndex, "x");
                    }

                    igSameLine(0, 4);

                    if (igSmallButton("Flip")) {
                        float scale = -binding->scale;
                        float threshold = binding->threshold;
                        const char *target = inputTargetName(binding->target);

                        if (!inputSetBinding(input, action->name, bindingIndex, deviceName,
                            controlName, target, scale, binding->player, threshold)) {
                            snprintf(inputStatus, sizeof(inputStatus), "Flip failed: %s", inputLastError());
                        }
                    }

                    igSameLine(0, 4);

                    if (igSmallButton("Remove")) {
                        if (!inputRemoveBinding(input, action->name, bindingIndex)) {
                            snprintf(inputStatus, sizeof(inputStatus), "Remove failed: %s", inputLastError());
                        }
                    }

                    igPopID();
                }

                if (action->type == INPUT_ACTION_AXIS_2D) {
                    if (igSmallButton("+ X")) beginInputCapture(input, (int)actionIndex, -1, "x");
                    igSameLine(0, 4);
                    if (igSmallButton("+ Y")) beginInputCapture(input, (int)actionIndex, -1, "y");
                }
                else {
                    if (igSmallButton("+ Add")) beginInputCapture(input, (int)actionIndex, -1, "x");
                }

                igTreePop();
            }

            igPopID();
        }

        igSeparatorText("Gamepads");

        int connectedPads = 0;
        for (int player = 0; player < INPUT_MAX_GAMEPADS; player++) {
            struct inputGamepad *pad = &input->gamepads[player];
            if (!pad->connected) continue;

            connectedPads++;
            igText("%d: %s (%s)", player, pad->name, pad->mapped ? "mapped" : "no mapping");
        }

        if (connectedPads == 0) {
            igTextDisabled("No gamepad connected.");
        }
    }

    igEnd();
}

void drawEngineUI(struct world *world) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();

    struct inputComponent *input = GET_SINGLETON_COMPONENT(world, inputComponent);
    ImGuiIO *imguiIO = igGetIO();
    inputSetUiCapture(input, imguiIO->WantCaptureKeyboard, imguiIO->WantCaptureMouse);

    drawMenuBar(world);
    drawSystems(world);
    drawComponents(world);
    drawTables(world);
    drawRendererData(world);
    drawHierarchy(world);
    drawInspector(world);
    drawFramebuffers();
    drawInput(input);

    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}
