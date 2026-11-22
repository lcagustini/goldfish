#include <scripting/luaInternal.h>

#include <goldfish/input.h>

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

struct bindingArguments {
    char device[32];
    char control[64];
    char target[8];

    float scale;
    int player;
    float threshold;
};

static struct inputComponent *requireInput(lua_State *L) {
    struct inputComponent *input = GET_SINGLETON_COMPONENT(activeWorld, inputComponent);
    if (input == NULL) luaL_error(L, "the current scene has no inputComponent");
    return input;
}

static int absoluteIndex(lua_State *L, int index) {
    if (index > 0 || index <= LUA_REGISTRYINDEX) return index;
    return lua_gettop(L) + index + 1;
}

static void getStringField(lua_State *L, int table, const char *key,
    char *buffer, size_t size, bool required, const char *fallback) {

    lua_getfield(L, table, key);

    if (lua_isnil(L, -1)) {
        if (required) luaL_error(L, "input binding is missing field '%s'", key);
        snprintf(buffer, size, "%s", fallback == NULL ? "" : fallback);
    }
    else {
        const char *value = luaL_checkstring(L, -1);
        snprintf(buffer, size, "%s", value);
    }

    lua_pop(L, 1);
}

static float getNumberField(lua_State *L, int table, const char *key, float fallback) {
    lua_getfield(L, table, key);

    float value = fallback;
    if (!lua_isnil(L, -1)) {
        if (!lua_isnumber(L, -1)) luaL_error(L, "input field '%s' must be a number", key);
        value = (float)lua_tonumber(L, -1);
    }

    lua_pop(L, 1);
    return value;
}

static int getIntegerField(lua_State *L, int table, const char *key, int fallback) {
    lua_getfield(L, table, key);

    int value = fallback;
    if (!lua_isnil(L, -1)) {
        if (!lua_isnumber(L, -1)) luaL_error(L, "input field '%s' must be a number", key);
        value = (int)lua_tointeger(L, -1);
    }

    lua_pop(L, 1);
    return value;
}

static void readBindingArguments(lua_State *L, int table, struct bindingArguments *arguments) {
    table = absoluteIndex(L, table);
    luaL_checktype(L, table, LUA_TTABLE);

    getStringField(L, table, "device", arguments->device, sizeof(arguments->device), true, NULL);
    getStringField(L, table, "control", arguments->control, sizeof(arguments->control), true, NULL);
    getStringField(L, table, "target", arguments->target, sizeof(arguments->target), false, "x");

    arguments->scale = getNumberField(L, table, "scale", 1.0f);
    arguments->player = getIntegerField(L, table, "player", -1);
    arguments->threshold = getNumberField(L, table, "threshold", 0.5f);
}

static void applyBinding(lua_State *L, struct inputComponent *input, const char *action, int table) {
    struct bindingArguments arguments;
    readBindingArguments(L, table, &arguments);

    if (!inputBind(input, action, arguments.device, arguments.control,
        arguments.target, arguments.scale, arguments.player, arguments.threshold)) {
        luaL_error(L, "input action '%s': %s", action, inputLastError());
    }
}

static enum inputActionType parseActionType(lua_State *L, const char *action, const char *type) {
    if (strcmp(type, "button") == 0) return INPUT_ACTION_BUTTON;
    if (strcmp(type, "axis1d") == 0 || strcmp(type, "axis") == 0) return INPUT_ACTION_AXIS_1D;
    if (strcmp(type, "axis2d") == 0) return INPUT_ACTION_AXIS_2D;

    luaL_error(L, "input action '%s' has unknown type '%s'", action, type);
    return INPUT_ACTION_BUTTON;
}

static void applyAction(lua_State *L, struct inputComponent *input, const char *name, int spec) {
    spec = absoluteIndex(L, spec);
    luaL_checktype(L, spec, LUA_TTABLE);

    char typeName[16];
    getStringField(L, spec, "type", typeName, sizeof(typeName), true, NULL);

    enum inputActionType type = parseActionType(L, name, typeName);
    float deadzone = getNumberField(L, spec, "deadzone", 0.2f);

    if (!inputAddAction(input, name, type, deadzone)) {
        luaL_error(L, "input action '%s': %s", name, inputLastError());
    }

    lua_getfield(L, spec, "bindings");
    if (!lua_isnil(L, -1)) {
        luaL_checktype(L, -1, LUA_TTABLE);

        int count = (int)lua_objlen(L, -1);
        for (int i = 1; i <= count; i++) {
            lua_rawgeti(L, -1, i);
            applyBinding(L, input, name, -1);
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
}

static int lApplyScene(lua_State *L) {
    struct inputComponent *input = requireInput(L);
    inputReset(input);

    if (lua_isnoneornil(L, 1)) return 0;

    luaL_checktype(L, 1, LUA_TTABLE);
    int scene = absoluteIndex(L, 1);

    lua_getfield(L, scene, "actions");
    if (!lua_isnil(L, -1)) {
        luaL_checktype(L, -1, LUA_TTABLE);
        int actions = lua_gettop(L);

        lua_pushnil(L);
        while (lua_next(L, actions) != 0) {
            const char *name = luaL_checkstring(L, -2);
            applyAction(L, input, name, -1);
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);

    return 0;
}

// action(name) -> lightuserdata into inputComponent
static int lAction(lua_State *L) {
    const char *name = luaL_checkstring(L, 1);

    struct inputComponent *input = requireInput(L);
    struct inputAction *action = inputActionPointer(input, name);
    if (action == NULL) luaL_error(L, "unknown input action '%s'", name);

    lua_pushlightuserdata(L, action);
    return 1;
}

// controlCode(device, name) -> integer code
static int lControlCode(lua_State *L) {
    const char *deviceName = luaL_checkstring(L, 1);
    const char *controlName = luaL_checkstring(L, 2);

    int device = inputDeviceCode(deviceName);
    if (device < 0) luaL_error(L, "unknown input device '%s'", deviceName);

    int code = inputControlCode((enum inputDevice)device, controlName);
    if (code < 0) {
        luaL_error(L, "unknown %s control '%s'", inputDeviceName((enum inputDevice)device), controlName);
    }

    lua_pushinteger(L, code);
    return 1;
}

static int lSetCursor(lua_State *L) {
    const char *mode = luaL_checkstring(L, 1);

    struct inputComponent *input = requireInput(L);

    if (strcmp(mode, "normal") == 0) inputSetCursorMode(input, INPUT_CURSOR_NORMAL);
    else if (strcmp(mode, "disabled") == 0) inputSetCursorMode(input, INPUT_CURSOR_DISABLED);
    else if (strcmp(mode, "raw") == 0) inputSetCursorMode(input, INPUT_CURSOR_RAW);
    else luaL_error(L, "cursor mode must be 'normal', 'disabled', or 'raw'");

    return 0;
}

const luaL_Reg luaInputBindings[] = {
    { "applyScene", lApplyScene },
    { "action0", lAction },
    { "controlCode", lControlCode },
    { "setCursor", lSetCursor },
    { NULL, NULL },
};
