#include <scripting/luaInternal.h>

#include <render/model.h>
#include <render/skybox.h>
#include <goldfish/global.h>
#include <goldfish/print.h>

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

void luaFatal(const char *what, const char *detail) {
    if (detail == NULL) detail = "no detail";
    print("[Lua fatal] %s: %s\n", what, detail);
    exit(1);
}

// No strdup: ISO C11 hides it, and one malloc+memcpy is the same amount of code.
char *persistentString(const char *source) {
    size_t length = strlen(source);
    char *copy = malloc(length + 1);
    if (copy == NULL) {
        luaFatal("out of memory", "persistentString");
    }
    memcpy(copy, source, length + 1);
    return copy;
}

// Errors route through print(); a pending error value becomes a traceback string.
const char *errorTraceback(lua_State *L) {
    const char *message = lua_tostring(L, -1);
    if (message == NULL) message = "non-string error";

    luaL_traceback(L, L, message, 2);
    const char *text = (const char *)lua_tostring(L, -1);
    lua_pop(L, 1); // traceback string
    return text;
}

// declare(name, size): register a Lua-only component from its FFI sizeof.
static int lDeclare(lua_State *L) {
    const char *name = (const char *)luaL_checkstring(L, 1);
    unsigned int size = (unsigned int)luaL_checkinteger(L, 2);

    createComponent(activeWorld, name, size);
    return 0;
}

static int lAddFilter(lua_State *L) {
    const char *name = persistentString(luaL_checkstring(L, 1));

    struct filter f = {0};
    luaL_checktype(L, 2, LUA_TTABLE);
    int count = (int)lua_objlen(L, 2);
    if (count > MAX_COMPONENT_COUNT) {
        luaL_error(L, "filter '%s': too many components (max %d)", name, MAX_COMPONENT_COUNT);
    }
    for (int i = 0; i < count; i++) {
        lua_rawgeti(L, 2, i + 1);
        f.components[i] = persistentString(luaL_checkstring(L, -1));
        lua_pop(L, 1);
    }
    f.componentsLength = (unsigned int)count;

    addFilter(activeWorld, name, f);
    return 0;
}

// addPhaseSystem(phase, fnName, filters): native names resolve to C, others stay Lua.
static int lAddPhaseSystem(lua_State *L) {
    int phase = (int)luaL_checkinteger(L, 1);
    if (phase < 0 || phase >= SYSTEM_PHASE_MAX) {
        luaL_error(L, "unknown phase %d", phase);
    }
    const char *fnName = persistentString(luaL_checkstring(L, 2));

    filterId filters[MAX_FILTER_COUNT];
    int count = 0;
    luaL_checktype(L, 3, LUA_TTABLE);
    count = (int)lua_objlen(L, 3);
    if (count > MAX_FILTER_COUNT) {
        luaL_error(L, "system '%s': too many filters (max %d)", fnName, MAX_FILTER_COUNT);
    }
    for (int i = 0; i < count; i++) {
        lua_rawgeti(L, 3, i + 1);
        filters[i] = persistentString(luaL_checkstring(L, -1));
        lua_pop(L, 1);
    }

    struct system s = {
        .name = fnName,
        .filtersLength = (unsigned int)count,
        .kind = SYSTEM_LUA,
        .callback = luaSystemCallback,
    };
    memcpy(s.filters, filters, count * sizeof(filterId));

    void (*native)(struct systemRunData) = findNativeSystem(fnName);
    if (native != NULL) {
        s.kind = SYSTEM_C;
        s.callback = native;
    } else {
        // Fail fast at registration, not per frame in the loop.
        lua_getglobal(L, fnName);
        if (!lua_isfunction(L, -1)) {
            luaL_error(L, "system '%s' is no native system and has no Lua global", fnName);
        }
        lua_pop(L, 1);
    }

    addPhaseSystem(activeWorld, (enum systemPhase)phase, s);
    return 0;
}

// addNativeEventSystem(event, fnName, component): mirrors ADD_EVENT_SYSTEM.
static int lAddNativeEventSystem(lua_State *L) {
    int event = (int)luaL_checkinteger(L, 1);
    if (event < 0 || event >= SYSTEM_EVENT_MAX) {
        luaL_error(L, "unknown event %d", event);
    }
    char *name = persistentString(luaL_checkstring(L, 2));
    componentId component = persistentString((const char *)luaL_checkstring(L, 3));

    struct filter f = {0};
    f.components[0] = component;
    f.componentsLength = 1;
    addFilter(activeWorld, name, f);

    struct system s = {
        .name = name,
        .filtersLength = 1,
        .kind = SYSTEM_C,
        .callback = findNativeSystem(name),
    };
    s.filters[0] = name;
    if (s.callback == NULL) {
        luaL_error(L, "event system '%s' is not a native system", name);
    }

    addEventSystem(activeWorld, (enum systemEvent)event, s);
    return 0;
}

static int lCreateEntity(lua_State *L) {
    const char *name = (const char *)luaL_checkstring(L, 1);
    entityId id = createEntity(activeWorld, name);
    lua_pushstring(L, id);
    return 1;
}

// Boundary check: a missing declaration is a loud Lua error, not a segfault.
static void checkComponentId(lua_State *L, const char *component) {
    if (hashtableGet(&activeWorld->components, component) == NULL) {
        luaL_error(L, "component '%s' is not registered: declare its ffi type and call engine.declare first", component);
    }
}

static void checkEntityId(lua_State *L, entityId entity) {
    if (hashtableGet(&activeWorld->entities, entity) == NULL) {
        luaL_error(L, "unknown entity id '%s'", entity);
    }
}

static int lAddComponent(lua_State *L) {
    entityId entity = (entityId)luaL_checkstring(L, 1);
    componentId component = persistentString((const char *)luaL_checkstring(L, 2));
    checkEntityId(L, entity);
    checkComponentId(L, component);

    addComponent(activeWorld, entity, component); // native event systems fire here
    return 0;
}

// getComponent(entity, ctype) -> lightuserdata or nil
static int lGetComponent(lua_State *L) {
    entityId entity = (entityId)luaL_checkstring(L, 1);
    componentId component = (componentId)luaL_checkstring(L, 2);
    checkEntityId(L, entity);
    checkComponentId(L, component);

    void *value = getComponent(activeWorld, entity, component);
    if (value == NULL) {
        lua_pushnil(L);
        return 1;
    }
    lua_pushlightuserdata(L, value);
    return 1;
}

// getSingleton(ctype) -> lightuserdata or nil
static int lGetSingleton(lua_State *L) {
    componentId component = (componentId)luaL_checkstring(L, 1);
    checkComponentId(L, component);

    void *value = getSingletonComponent(activeWorld, component);
    if (value == NULL) {
        lua_pushnil(L);
        return 1;
    }
    lua_pushlightuserdata(L, value);
    return 1;
}

// getColumn(run, filterIdx, ctype) -> (lightuserdata, rows).
// run: lightuserdata world pointer + 1-based table of tableIds.
static int lGetColumn(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, 1, "world");
    struct world *world = (struct world *)lua_touserdata(L, -1);
    lua_pop(L, 1);
    if (world == NULL) {
        luaL_error(L, "run has no world pointer");
    }
    lua_getfield(L, 1, "filterResults");
    if (!lua_istable(L, -1)) {
        luaL_error(L, "run has no filterResults table");
    }
    componentId component = (componentId)luaL_checkstring(L, 3);

    int filterIdx = (int)luaL_checkinteger(L, 2); // 0-based, matching the script API
    if (filterIdx < 0) {
        luaL_error(L, "filter index %d < 0", filterIdx);
    }
    size_t runCount = lua_objlen(L, -1);
    if ((size_t)(filterIdx + 1) > runCount) {
        luaL_error(L, "filter index %d out of range (run has %zu)", filterIdx, runCount);
    }

    lua_rawgeti(L, -1, filterIdx + 1);
    tableId table = (tableId)luaL_checkinteger(L, -1);
    lua_pop(L, 2); // tableId + filterResults table

    unsigned int rows = 0;
    void *column = getComponentsFromTable(world, table, component, &rows);
    if (column == NULL) {
        lua_pushnil(L);
        lua_pushinteger(L, 0);
        return 2;
    }
    lua_pushlightuserdata(L, column);
    lua_pushinteger(L, (lua_Integer)rows);
    return 2;
}

// Paths are copied: the renderer keeps the pointers, Lua strings do not.
static int lLoadModel(lua_State *L) {    const char *modelPath = persistentString(luaL_checkstring(L, 1));
    const char *diffusePath = lua_isnil(L, 2) ? NULL : persistentString(luaL_checkstring(L, 2));
    const char *normalPath = lua_isnil(L, 3) ? NULL : persistentString(luaL_checkstring(L, 3));
    const char *specularPath = lua_isnil(L, 4) ? NULL : persistentString(luaL_checkstring(L, 4));
    const char *reflectancePath = lua_isnil(L, 5) ? NULL : persistentString(luaL_checkstring(L, 5));
    bool transparent = lua_toboolean(L, 6) != 0;

    entityId id = loadModel(activeWorld, modelPath, diffusePath, normalPath, specularPath, reflectancePath, transparent);
    lua_pushstring(L, id);
    return 1;
}

static int lLoadSkybox(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    if (lua_objlen(L, 1) != 6) {
        luaL_error(L, "skybox needs exactly 6 face paths");
    }

    const char *facePaths[6];
    for (int i = 0; i < 6; i++) {
        lua_rawgeti(L, 1, i + 1);
        facePaths[i] = persistentString(luaL_checkstring(L, -1));
        lua_pop(L, 1);
    }

    entityId skybox = createEntity(activeWorld, "Skybox");
    addComponent(activeWorld, skybox, "skyboxComponent");
    struct skyboxComponent *data = (struct skyboxComponent *)getComponent(activeWorld, skybox, "skyboxComponent");
    if (data == NULL) {
        luaL_error(L, "skybox component missing after addComponent");
    }
    loadSkybox(facePaths, data);
    lua_pushstring(L, skybox);
    return 1;
}

static int lUniformLocation(lua_State *L) {
    luaL_argcheck(L, lua_type(L, 1) == 10, 1, "struct meshRenderData pointer expected");
    const void *payload = lua_topointer(L, 1);
    luaL_argcheck(L, payload != NULL, 1, "mesh pointer is nil");
    struct meshRenderData *mesh = *(struct meshRenderData **)payload;
    luaL_argcheck(L, mesh != NULL, 1, "mesh pointer is nil");
    const char *name = luaL_checkstring(L, 2);

    lua_pushinteger(L, shaderUniformLocation(&mesh->shader, name));
    return 1;
}

static bool isDescendantOf(struct world *world, entityId entity, entityId ancestor) {
    entityId current = entity;
    while (current != NULL) {
        if (strcmp(current, ancestor) == 0) return true;

        struct transformComponent *transform = (struct transformComponent *)getComponent(world, current, "transformComponent");
        if (transform == NULL || transform->parent == NULL) return false;
        current = transform->parent;
    }
    return false;
}

static int lSetShader(lua_State *L) {
    entityId entity = (entityId)luaL_checkstring(L, 1);
    checkEntityId(L, entity);
    if (hashtableGet(&activeWorld->components, "modelComponent") == NULL) {
        luaL_error(L, "entity '%s': modelComponent is not registered", entity);
    }
    const char *vertexPath = persistentString(luaL_checkstring(L, 2));
    const char *fragmentPath = persistentString(luaL_checkstring(L, 3));

    struct shader *replaced[64];
    unsigned int replacedLength = 0;

    for (unsigned int t = 0; t < activeWorld->tables.bufferCount; t++) {
        unsigned int rows = 0;
        struct modelComponent *models = getComponentsFromTable(activeWorld, t, "modelComponent", &rows);
        if (models == NULL) continue;

        for (unsigned int row = 0; row < rows; row++) {
            entityId entityAtRow = searchForEntity(activeWorld, t, row);
            if (strcmp(entityAtRow, entity) != 0 && !isDescendantOf(activeWorld, entityAtRow, entity)) continue;

            struct modelComponent *model = &models[row];
            if (model->meshesLength == 0 || model->meshes[0].material.shader == NULL) continue;

            struct shader *shader = model->meshes[0].material.shader;
            bool seen = false;
            for (unsigned int r = 0; r < replacedLength; r++) seen = seen || replaced[r] == shader;
            if (seen) continue;

            if (replacedLength == 64) {
                luaL_error(L, "setShader: more than 64 shader blocks under entity '%s'", entity);
            }
            if (!shaderReplace(shader, vertexPath, fragmentPath)) {
                luaL_error(L, "shader %s/%s failed to build; the previous program stays active", vertexPath, fragmentPath);
            }
            replaced[replacedLength++] = shader;
        }
    }

    if (replacedLength == 0) {
        luaL_error(L, "entity '%s' has no loaded shader", entity);
    }
    return 0;
}

static int lDeleteEntity(lua_State *L) {
    entityId entity = (entityId)luaL_checkstring(L, 1);
    checkEntityId(L, entity);
    deleteEntity(activeWorld, entity);
    return 0;
}

static int lRemoveFilter(lua_State *L) {
    const char *name = luaL_checkstring(L, 1);
    if (!hashtableRemove(&activeWorld->filters, name)) {
        luaL_error(L, "filter '%s' is not registered", name);
    }
    return 0;
}

static int lRemovePhaseSystem(lua_State *L) {
    int phase = (int)luaL_checkinteger(L, 1);
    if (phase < 0 || phase >= SYSTEM_PHASE_MAX) {
        luaL_error(L, "unknown phase %d", phase);
    }
    removePhaseSystem(activeWorld, (enum systemPhase)phase, luaL_checkstring(L, 2));
    return 0;
}

static int lRemoveSingleton(lua_State *L) {
    const char *component = (const char *)luaL_checkstring(L, 1);
    checkComponentId(L, component);
    removeSingletonComponent(activeWorld, persistentString(component));
    return 0;
}

static int lAddSingleton(lua_State *L) {
    const char *component = (const char *)luaL_checkstring(L, 1);
    checkComponentId(L, component);
    addSingletonComponent(activeWorld, persistentString(component));
    return 0;
}

static int lLog(lua_State *L) {
    print("%s\n", (const char *)luaL_checkstring(L, 1));
    return 0;
}

// entityIdPointer(entity) -> engine-owned id string, for engine.rawEntity casts.
static int lEntityIdPointer(lua_State *L) {
    entityId entity = (entityId)luaL_checkstring(L, 1);
    struct entity *e = (struct entity *)hashtableGet(&activeWorld->entities, entity);
    luaL_argcheck(L, e != NULL, 1, "unknown entity id");
    lua_pushlightuserdata(L, e->name);
    return 1;
}

// world() -> lightuserdata of the active world, for selftest run DTOs.
static int lWorld(lua_State *L) {
    lua_pushlightuserdata(L, activeWorld);
    return 1;
}

// filterResults(name) -> 1-based tableIds, like a run DTO, for boot selftests.
static int lFilterResults(lua_State *L) {
    const char *name = (const char *)luaL_checkstring(L, 1);
    struct filter *filter = (struct filter *)hashtableGet(&activeWorld->filters, name);
    luaL_argcheck(L, filter != NULL, 1, "unknown filter name");

    lua_createtable(L, (int)filter->resultsLength, 0);
    for (unsigned int i = 0; i < filter->resultsLength; i++) {
        lua_pushinteger(L, (lua_Integer)filter->results[i]);
        lua_rawseti(L, -2, (int)(i + 1));
    }
    return 1;
}

// registry() -> {{name = id, size = bytes}, ...}: feeds the size-parity sweep.
static int lRegistry(lua_State *L) {
    struct component *components = activeWorld->components.buffer;

    lua_createtable(L, activeWorld->components.validCount, 0);
    int entry = 0;
    for (unsigned int i = 0; i < activeWorld->components.bufferCount; i++) {
        if (!activeWorld->components.valids[i]) continue;

        entry++;
        lua_createtable(L, 0, 2);
        lua_pushstring(L, activeWorld->components.keys[i]);
        lua_setfield(L, -2, "name");
        lua_pushinteger(L, (lua_Integer)components[i].size);
        lua_setfield(L, -2, "size");
        lua_rawseti(L, -2, entry);
    }
    return 1;
}

static int lQuit(lua_State *L) {
    globalState.quitRequested = true;
    return 0;
}

// Binding table consumed by lua.c's engine table registration.
const luaL_Reg luaBindings[] = {
    { "declare0", lDeclare },
    { "addFilter", lAddFilter },
    { "removeFilter", lRemoveFilter },
    { "addPhaseSystem", lAddPhaseSystem },
    { "removePhaseSystem", lRemovePhaseSystem },
    { "addNativeEventSystem", lAddNativeEventSystem },
    { "createEntity", lCreateEntity },
    { "deleteEntity", lDeleteEntity },
    { "addComponent", lAddComponent },
    { "getComponent0", lGetComponent },
    { "getSingleton0", lGetSingleton },
    { "addSingleton", lAddSingleton },
    { "removeSingleton", lRemoveSingleton },
    { "getColumn", lGetColumn },
    { "loadModel", lLoadModel },
    { "loadSkybox", lLoadSkybox },
    { "uniformLocation0", lUniformLocation },
    { "setShader", lSetShader },
    { "entityIdPointer", lEntityIdPointer },
    { "world", lWorld },
    { "filterResults", lFilterResults },
    { "log0", lLog },
    { "quit", lQuit },
    { "registry", lRegistry },
    { NULL, NULL },
};
