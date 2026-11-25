#include <scripting/luaInternal.h>

#include <ecs/systems.h>
#include <goldfish/print.h>

#include <string.h>

struct nativeSystem {
    const char *name;
    void (*callback)(struct systemRunData);
};

// The native systems scripts can bind by name; findNativeSystem resolves them.
static const struct nativeSystem nativeSystems[] = {
    { "updateInput", updateInput },
    { "updateCameraView", updateCameraView },
    { "updateTransformMatrix", updateTransformMatrix },
    { "rendererGetModels", rendererGetModels },
    { "rendererGetCameras", rendererGetCameras },
    { "rendererGetSkybox", rendererGetSkybox },
    { "rendererGetDirLights", rendererGetDirLights },
    { "rendererGetSpotLights", rendererGetSpotLights },
    { "rendererGetPointLights", rendererGetPointLights },
    { "rendererDirShadowPass", rendererDirShadowPass },
    { "rendererSpotShadowPass", rendererSpotShadowPass },
    { "rendererPointShadowPass", rendererPointShadowPass },
    { "rendererOpaqueRender", rendererOpaqueRender },
    { "renderSkybox", renderSkybox },
    { "rendererTransparentRender", rendererTransparentRender },
    { "finishRender", finishRender },
    { NULL, NULL },
};

void (*findNativeSystem(const char *name))(struct systemRunData) {
    for (int i = 0; nativeSystems[i].name != NULL; i++) {
        if (strcmp(nativeSystems[i].name, name) == 0) return nativeSystems[i].callback;
    }
    return NULL;
}

void luaSystemCallback(struct systemRunData data) {
    lua_State *L = luaState;
    if (L == NULL) return;

    lua_getglobal(L, data.system->name);
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        print("[Lua system error] no global function '%s'\n", data.system->name);
        return;
    }

    lua_createtable(L, 0, 3);
    lua_pushliteral(L, "dt");
    lua_pushnumber(L, (lua_Number)data.dt);
    lua_settable(L, -3);

    lua_pushliteral(L, "world");
    lua_pushlightuserdata(L, data.world);
    lua_settable(L, -3);

    lua_pushliteral(L, "filterResults");
    lua_createtable(L, (int)data.filterResultsLength, 0);
    for (unsigned int i = 0; i < data.filterResultsLength; i++) {
        lua_pushinteger(L, (lua_Integer)data.filterResults[i].table);
        lua_rawseti(L, -2, (int)(i + 1)); // 1-based; engine.view converts to 0-based
    }
    lua_settable(L, -3);

    if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
        print("[Lua system error] %s:\n%s\n", data.system->name, errorTraceback(L));
        lua_pop(L, 1); // error string
    }
}
