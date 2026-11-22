#include <goldfish/input.h>
#include <goldfish/global.h>
#include <ecs/systems.h>

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

_Static_assert(GLFW_KEY_LAST < INPUT_KEY_COUNT, "INPUT_KEY_COUNT is too small");
_Static_assert(GLFW_MOUSE_BUTTON_LAST < INPUT_MOUSE_BUTTON_COUNT, "INPUT_MOUSE_BUTTON_COUNT is too small");
_Static_assert(GLFW_GAMEPAD_BUTTON_LAST < INPUT_GAMEPAD_BUTTON_COUNT, "INPUT_GAMEPAD_BUTTON_COUNT is too small");
_Static_assert(GLFW_GAMEPAD_AXIS_LAST < INPUT_GAMEPAD_AXIS_COUNT, "INPUT_GAMEPAD_AXIS_COUNT is too small");

struct inputControlName {
    const char *name;
    int code;
};

static char inputError[256];
static double pendingScrollX;
static double pendingScrollY;

static void resolveActions(struct inputComponent *input);

static const struct inputControlName keyboardNames[] = {
    { "space", GLFW_KEY_SPACE },
    { "apostrophe", GLFW_KEY_APOSTROPHE },
    { "comma", GLFW_KEY_COMMA },
    { "minus", GLFW_KEY_MINUS },
    { "period", GLFW_KEY_PERIOD },
    { "slash", GLFW_KEY_SLASH },
    { "0", GLFW_KEY_0 },
    { "1", GLFW_KEY_1 },
    { "2", GLFW_KEY_2 },
    { "3", GLFW_KEY_3 },
    { "4", GLFW_KEY_4 },
    { "5", GLFW_KEY_5 },
    { "6", GLFW_KEY_6 },
    { "7", GLFW_KEY_7 },
    { "8", GLFW_KEY_8 },
    { "9", GLFW_KEY_9 },
    { "semicolon", GLFW_KEY_SEMICOLON },
    { "equal", GLFW_KEY_EQUAL },
    { "a", GLFW_KEY_A },
    { "b", GLFW_KEY_B },
    { "c", GLFW_KEY_C },
    { "d", GLFW_KEY_D },
    { "e", GLFW_KEY_E },
    { "f", GLFW_KEY_F },
    { "g", GLFW_KEY_G },
    { "h", GLFW_KEY_H },
    { "i", GLFW_KEY_I },
    { "j", GLFW_KEY_J },
    { "k", GLFW_KEY_K },
    { "l", GLFW_KEY_L },
    { "m", GLFW_KEY_M },
    { "n", GLFW_KEY_N },
    { "o", GLFW_KEY_O },
    { "p", GLFW_KEY_P },
    { "q", GLFW_KEY_Q },
    { "r", GLFW_KEY_R },
    { "s", GLFW_KEY_S },
    { "t", GLFW_KEY_T },
    { "u", GLFW_KEY_U },
    { "v", GLFW_KEY_V },
    { "w", GLFW_KEY_W },
    { "x", GLFW_KEY_X },
    { "y", GLFW_KEY_Y },
    { "z", GLFW_KEY_Z },
    { "leftBracket", GLFW_KEY_LEFT_BRACKET },
    { "backslash", GLFW_KEY_BACKSLASH },
    { "rightBracket", GLFW_KEY_RIGHT_BRACKET },
    { "grave", GLFW_KEY_GRAVE_ACCENT },
    { "world1", GLFW_KEY_WORLD_1 },
    { "world2", GLFW_KEY_WORLD_2 },
    { "escape", GLFW_KEY_ESCAPE },
    { "enter", GLFW_KEY_ENTER },
    { "tab", GLFW_KEY_TAB },
    { "backspace", GLFW_KEY_BACKSPACE },
    { "insert", GLFW_KEY_INSERT },
    { "delete", GLFW_KEY_DELETE },
    { "right", GLFW_KEY_RIGHT },
    { "left", GLFW_KEY_LEFT },
    { "down", GLFW_KEY_DOWN },
    { "up", GLFW_KEY_UP },
    { "pageUp", GLFW_KEY_PAGE_UP },
    { "pageDown", GLFW_KEY_PAGE_DOWN },
    { "home", GLFW_KEY_HOME },
    { "end", GLFW_KEY_END },
    { "capsLock", GLFW_KEY_CAPS_LOCK },
    { "scrollLock", GLFW_KEY_SCROLL_LOCK },
    { "numLock", GLFW_KEY_NUM_LOCK },
    { "printScreen", GLFW_KEY_PRINT_SCREEN },
    { "pause", GLFW_KEY_PAUSE },
    { "f1", GLFW_KEY_F1 },
    { "f2", GLFW_KEY_F2 },
    { "f3", GLFW_KEY_F3 },
    { "f4", GLFW_KEY_F4 },
    { "f5", GLFW_KEY_F5 },
    { "f6", GLFW_KEY_F6 },
    { "f7", GLFW_KEY_F7 },
    { "f8", GLFW_KEY_F8 },
    { "f9", GLFW_KEY_F9 },
    { "f10", GLFW_KEY_F10 },
    { "f11", GLFW_KEY_F11 },
    { "f12", GLFW_KEY_F12 },
    { "f13", GLFW_KEY_F13 },
    { "f14", GLFW_KEY_F14 },
    { "f15", GLFW_KEY_F15 },
    { "f16", GLFW_KEY_F16 },
    { "f17", GLFW_KEY_F17 },
    { "f18", GLFW_KEY_F18 },
    { "f19", GLFW_KEY_F19 },
    { "f20", GLFW_KEY_F20 },
    { "f21", GLFW_KEY_F21 },
    { "f22", GLFW_KEY_F22 },
    { "f23", GLFW_KEY_F23 },
    { "f24", GLFW_KEY_F24 },
    { "f25", GLFW_KEY_F25 },
    { "kp0", GLFW_KEY_KP_0 },
    { "kp1", GLFW_KEY_KP_1 },
    { "kp2", GLFW_KEY_KP_2 },
    { "kp3", GLFW_KEY_KP_3 },
    { "kp4", GLFW_KEY_KP_4 },
    { "kp5", GLFW_KEY_KP_5 },
    { "kp6", GLFW_KEY_KP_6 },
    { "kp7", GLFW_KEY_KP_7 },
    { "kp8", GLFW_KEY_KP_8 },
    { "kp9", GLFW_KEY_KP_9 },
    { "kpDecimal", GLFW_KEY_KP_DECIMAL },
    { "kpDivide", GLFW_KEY_KP_DIVIDE },
    { "kpMultiply", GLFW_KEY_KP_MULTIPLY },
    { "kpSubtract", GLFW_KEY_KP_SUBTRACT },
    { "kpAdd", GLFW_KEY_KP_ADD },
    { "kpEnter", GLFW_KEY_KP_ENTER },
    { "kpEqual", GLFW_KEY_KP_EQUAL },
    { "leftShift", GLFW_KEY_LEFT_SHIFT },
    { "leftControl", GLFW_KEY_LEFT_CONTROL },
    { "leftAlt", GLFW_KEY_LEFT_ALT },
    { "leftSuper", GLFW_KEY_LEFT_SUPER },
    { "rightShift", GLFW_KEY_RIGHT_SHIFT },
    { "rightControl", GLFW_KEY_RIGHT_CONTROL },
    { "rightAlt", GLFW_KEY_RIGHT_ALT },
    { "rightSuper", GLFW_KEY_RIGHT_SUPER },
    { "menu", GLFW_KEY_MENU },
    { NULL, 0 },
};

static const struct inputControlName mouseNames[] = {
    { "left", INPUT_MOUSE_LEFT },
    { "right", INPUT_MOUSE_RIGHT },
    { "middle", INPUT_MOUSE_MIDDLE },
    { "x1", INPUT_MOUSE_X1 },
    { "x2", INPUT_MOUSE_X2 },
    { "deltaX", INPUT_MOUSE_DELTA_X },
    { "deltaY", INPUT_MOUSE_DELTA_Y },
    { "wheelX", INPUT_MOUSE_WHEEL_X },
    { "wheelY", INPUT_MOUSE_WHEEL_Y },
    { NULL, 0 },
};

static const struct inputControlName gamepadNames[] = {
    { "a", GLFW_GAMEPAD_BUTTON_A },
    { "b", GLFW_GAMEPAD_BUTTON_B },
    { "x", GLFW_GAMEPAD_BUTTON_X },
    { "y", GLFW_GAMEPAD_BUTTON_Y },
    { "lb", GLFW_GAMEPAD_BUTTON_LEFT_BUMPER },
    { "rb", GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER },
    { "back", GLFW_GAMEPAD_BUTTON_BACK },
    { "start", GLFW_GAMEPAD_BUTTON_START },
    { "guide", GLFW_GAMEPAD_BUTTON_GUIDE },
    { "ls", GLFW_GAMEPAD_BUTTON_LEFT_THUMB },
    { "rs", GLFW_GAMEPAD_BUTTON_RIGHT_THUMB },
    { "up", GLFW_GAMEPAD_BUTTON_DPAD_UP },
    { "right", GLFW_GAMEPAD_BUTTON_DPAD_RIGHT },
    { "down", GLFW_GAMEPAD_BUTTON_DPAD_DOWN },
    { "left", GLFW_GAMEPAD_BUTTON_DPAD_LEFT },
    { "south", GLFW_GAMEPAD_BUTTON_A },
    { "east", GLFW_GAMEPAD_BUTTON_B },
    { "west", GLFW_GAMEPAD_BUTTON_X },
    { "north", GLFW_GAMEPAD_BUTTON_Y },
    { "leftBumper", GLFW_GAMEPAD_BUTTON_LEFT_BUMPER },
    { "rightBumper", GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER },
    { "leftThumb", GLFW_GAMEPAD_BUTTON_LEFT_THUMB },
    { "rightThumb", GLFW_GAMEPAD_BUTTON_RIGHT_THUMB },
    { "dpadUp", GLFW_GAMEPAD_BUTTON_DPAD_UP },
    { "dpadRight", GLFW_GAMEPAD_BUTTON_DPAD_RIGHT },
    { "dpadDown", GLFW_GAMEPAD_BUTTON_DPAD_DOWN },
    { "dpadLeft", GLFW_GAMEPAD_BUTTON_DPAD_LEFT },
    { "leftX", INPUT_GAMEPAD_AXIS_BASE + GLFW_GAMEPAD_AXIS_LEFT_X },
    { "leftY", INPUT_GAMEPAD_AXIS_BASE + GLFW_GAMEPAD_AXIS_LEFT_Y },
    { "rightX", INPUT_GAMEPAD_AXIS_BASE + GLFW_GAMEPAD_AXIS_RIGHT_X },
    { "rightY", INPUT_GAMEPAD_AXIS_BASE + GLFW_GAMEPAD_AXIS_RIGHT_Y },
    { "leftTrigger", INPUT_GAMEPAD_AXIS_BASE + GLFW_GAMEPAD_AXIS_LEFT_TRIGGER },
    { "rightTrigger", INPUT_GAMEPAD_AXIS_BASE + GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER },
    { "lt", INPUT_GAMEPAD_AXIS_BASE + GLFW_GAMEPAD_AXIS_LEFT_TRIGGER },
    { "rt", INPUT_GAMEPAD_AXIS_BASE + GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER },
    { NULL, 0 },
};

static const struct inputControlName *controlTable(enum inputDevice device) {
    switch (device) {
        case INPUT_DEVICE_KEYBOARD: return keyboardNames;
        case INPUT_DEVICE_MOUSE: return mouseNames;
        case INPUT_DEVICE_GAMEPAD: return gamepadNames;
        default: return NULL;
    }
}

static void setError(const char *format, ...) {
    va_list arguments;
    va_start(arguments, format);
    vsnprintf(inputError, sizeof(inputError), format, arguments);
    va_end(arguments);
}

static void clearError(void) {
    inputError[0] = '\0';
}

const char *inputLastError(void) {
    return inputError;
}

const char *inputDeviceName(enum inputDevice device) {
    switch (device) {
        case INPUT_DEVICE_KEYBOARD: return "key";
        case INPUT_DEVICE_MOUSE: return "mouse";
        case INPUT_DEVICE_GAMEPAD: return "pad";
        default: return "";
    }
}

int inputControlCode(enum inputDevice device, const char *name) {
    const struct inputControlName *table = controlTable(device);
    if (table == NULL || name == NULL) return -1;

    for (int i = 0; table[i].name != NULL; i++) {
        if (strcmp(table[i].name, name) == 0) return table[i].code;
    }
    return -1;
}

const char *inputControlName(enum inputDevice device, int control) {
    const struct inputControlName *table = controlTable(device);
    if (table == NULL) return "";

    for (int i = 0; table[i].name != NULL; i++) {
        if (table[i].code == control) return table[i].name;
    }
    return "";
}

static int parseDeviceName(const char *name) {
    if (name == NULL) return -1;
    if (strcmp(name, "key") == 0 || strcmp(name, "keyboard") == 0) return INPUT_DEVICE_KEYBOARD;
    if (strcmp(name, "mouse") == 0) return INPUT_DEVICE_MOUSE;
    if (strcmp(name, "pad") == 0 || strcmp(name, "gamepad") == 0) return INPUT_DEVICE_GAMEPAD;
    return -1;
}

int inputDeviceCode(const char *name) {
    return parseDeviceName(name);
}

static void scrollCallback(GLFWwindow *window, double x, double y) {
    (void)window;
    pendingScrollX += x;
    pendingScrollY += y;
}

void inputInit(void) {
    pendingScrollX = 0.0;
    pendingScrollY = 0.0;

    clearError();

    glfwSetScrollCallback(globalState.window, scrollCallback);
}

void inputEnd(void) {
    pendingScrollX = 0.0;
    pendingScrollY = 0.0;
}

void inputReset(struct inputComponent *input) {
    if (input == NULL) return;

    memset(input, 0, sizeof(*input));
    input->firstPoll = true;

    input->capture.actionIndex = -1;
    input->capture.bindingIndex = -1;
    input->capture.capturedDevice = -1;
    input->capture.capturedControl = -1;
    input->capture.capturedTarget = -1;
    input->capture.capturedPlayer = -1;

    clearError();
}

static void pollGamepad(struct inputComponent *input, int player) {
    struct inputGamepad *pad = &input->gamepads[player];
    bool present = glfwJoystickPresent(player);

    if (!present) {
        memset(pad, 0, sizeof(*pad));
        return;
    }

    if (!pad->connected) {
        memset(pad, 0, sizeof(*pad));
    }

    pad->connected = true;
    pad->mapped = glfwJoystickIsGamepad(player);

    const char *name = glfwGetJoystickName(player);
    const char *guid = glfwGetJoystickGUID(player);
    snprintf(pad->name, sizeof(pad->name), "%s", name == NULL ? "" : name);
    snprintf(pad->guid, sizeof(pad->guid), "%s", guid == NULL ? "" : guid);

    memcpy(pad->previousButtons, pad->buttons, sizeof(pad->buttons));

    if (!pad->mapped) {
        memset(pad->axes, 0, sizeof(pad->axes));
        memset(pad->buttons, 0, sizeof(pad->buttons));
        return;
    }

    GLFWgamepadstate state;
    if (!glfwGetGamepadState(player, &state)) {
        pad->mapped = false;
        memset(pad->axes, 0, sizeof(pad->axes));
        memset(pad->buttons, 0, sizeof(pad->buttons));
        return;
    }

    for (int button = 0; button < INPUT_GAMEPAD_BUTTON_COUNT; button++) {
        pad->buttons[button] = state.buttons[button];
    }

    for (int axis = 0; axis < INPUT_GAMEPAD_AXIS_COUNT; axis++) {
        float value = state.axes[axis];

        if (axis == GLFW_GAMEPAD_AXIS_LEFT_Y || axis == GLFW_GAMEPAD_AXIS_RIGHT_Y) {
            value = -value;
        }
        else if (axis == GLFW_GAMEPAD_AXIS_LEFT_TRIGGER || axis == GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER) {
            value = (value + 1.0f) * 0.5f;
        }

        pad->axes[axis] = value;
    }
}

static void pollDevices(struct inputComponent *input) {
    memcpy(input->previousKeys, input->keys, sizeof(input->keys));
    memcpy(input->previousMouseButtons, input->mouseButtons, sizeof(input->mouseButtons));

    input->focused = globalState.headless
        || glfwGetWindowAttrib(globalState.window, GLFW_FOCUSED) == GLFW_TRUE;

    if (input->focused) {
        for (int key = 0; key <= GLFW_KEY_LAST; key++) {
            input->keys[key] = glfwGetKey(globalState.window, key) == GLFW_PRESS;
        }

        for (int button = 0; button <= GLFW_MOUSE_BUTTON_LAST; button++) {
            input->mouseButtons[button] = glfwGetMouseButton(globalState.window, button) == GLFW_PRESS;
        }
    }
    else {
        memset(input->keys, 0, sizeof(input->keys));
        memset(input->mouseButtons, 0, sizeof(input->mouseButtons));
    }

    double mouseX = input->previousMouseX;
    double mouseY = input->previousMouseY;

    if (globalState.window != NULL) {
        glfwGetCursorPos(globalState.window, &mouseX, &mouseY);
    }

    if (input->firstPoll || input->resetMouse) {
        input->previousMouseX = mouseX;
        input->previousMouseY = mouseY;
        input->firstPoll = false;
        input->resetMouse = false;
    }

    input->mouseDeltaX = input->focused ? mouseX - input->previousMouseX : 0.0;
    input->mouseDeltaY = input->focused ? mouseY - input->previousMouseY : 0.0;
    input->mouseX = mouseX;
    input->mouseY = mouseY;
    input->previousMouseX = mouseX;
    input->previousMouseY = mouseY;

    input->scrollX = pendingScrollX;
    input->scrollY = pendingScrollY;
    pendingScrollX = 0.0;
    pendingScrollY = 0.0;

    for (int player = 0; player < INPUT_MAX_GAMEPADS; player++) {
        pollGamepad(input, player);
    }
}

static bool deviceSuppressed(const struct inputComponent *input, enum inputDevice device) {
    if (!input->focused || input->capture.active) return true;
    if (device == INPUT_DEVICE_KEYBOARD && input->uiCaptureKeyboard) return true;
    if (device == INPUT_DEVICE_MOUSE && input->uiCaptureMouse) return true;
    return false;
}

static float gamepadAnyAxisValue(const struct inputComponent *input, int axis) {
    float best = 0.0f;

    for (int player = 0; player < INPUT_MAX_GAMEPADS; player++) {
        const struct inputGamepad *pad = &input->gamepads[player];
        if (!pad->connected || !pad->mapped) continue;

        float value = pad->axes[axis];
        if (fabsf(value) > fabsf(best)) best = value;
    }

    return best;
}

static bool gamepadAnyButtonDown(const struct inputComponent *input, int button) {
    for (int player = 0; player < INPUT_MAX_GAMEPADS; player++) {
        const struct inputGamepad *pad = &input->gamepads[player];
        if (!pad->connected || !pad->mapped) continue;
        if (pad->buttons[button] != 0) return true;
    }

    return false;
}

static float sampleBinding(const struct inputComponent *input, const struct inputBinding *binding) {
    if (deviceSuppressed(input, binding->device)) return 0.0f;

    switch (binding->device) {
        case INPUT_DEVICE_KEYBOARD:
            return input->keys[binding->control] ? binding->scale : 0.0f;

        case INPUT_DEVICE_MOUSE:
            if (binding->control <= GLFW_MOUSE_BUTTON_LAST) {
                return input->mouseButtons[binding->control] ? binding->scale : 0.0f;
            }

            switch (binding->control) {
                case INPUT_MOUSE_DELTA_X: return (float)input->mouseDeltaX * binding->scale;
                case INPUT_MOUSE_DELTA_Y: return (float)input->mouseDeltaY * binding->scale;
                case INPUT_MOUSE_WHEEL_X: return (float)input->scrollX * binding->scale;
                case INPUT_MOUSE_WHEEL_Y: return (float)input->scrollY * binding->scale;
                default: return 0.0f;
            }

        case INPUT_DEVICE_GAMEPAD:
            if (binding->control >= INPUT_GAMEPAD_AXIS_BASE) {
                int axis = binding->control - INPUT_GAMEPAD_AXIS_BASE;
                if (axis < 0 || axis >= INPUT_GAMEPAD_AXIS_COUNT) return 0.0f;

                if (binding->player >= 0) {
                    const struct inputGamepad *pad = &input->gamepads[binding->player];
                    if (!pad->connected || !pad->mapped) return 0.0f;
                    return pad->axes[axis] * binding->scale;
                }
                return gamepadAnyAxisValue(input, axis) * binding->scale;
            }
            else {
                if (binding->control < 0 || binding->control >= INPUT_GAMEPAD_BUTTON_COUNT) return 0.0f;

                if (binding->player >= 0) {
                    const struct inputGamepad *pad = &input->gamepads[binding->player];
                    if (!pad->connected || !pad->mapped) return 0.0f;
                    return pad->buttons[binding->control] != 0 ? binding->scale : 0.0f;
                }
                return gamepadAnyButtonDown(input, binding->control) ? binding->scale : 0.0f;
            }

        default:
            return 0.0f;
    }
}

static float clampValue(float value, float low, float high) {
    if (value < low) return low;
    if (value > high) return high;
    return value;
}

static float applyDeadzone1D(float value, float deadzone) {
    float magnitude = fabsf(value);
    if (magnitude <= deadzone) return 0.0f;

    float scaled = (magnitude - deadzone) / (1.0f - deadzone);
    return value < 0.0f ? -scaled : scaled;
}

static void applyDeadzone2D(float *x, float *y, float deadzone) {
    float magnitude = sqrtf((*x) * (*x) + (*y) * (*y));
    if (magnitude <= deadzone) {
        *x = 0.0f;
        *y = 0.0f;
        return;
    }

    float scale = ((magnitude - deadzone) / (1.0f - deadzone)) / magnitude;
    *x *= scale;
    *y *= scale;
}

// Mouse motion and wheel are deltas, not directions. They bypass the axis
// clamp, the deadzone, and the 2D normalization, so their scale stays linear.
static bool relativeBinding(const struct inputBinding *binding) {
    return binding->device == INPUT_DEVICE_MOUSE && binding->control >= INPUT_MOUSE_DELTA_X;
}

static void resolveActions(struct inputComponent *input) {
    for (unsigned int actionIndex = 0; actionIndex < input->actionCount; actionIndex++) {
        struct inputAction *action = &input->actions[actionIndex];

        float x = 0.0f;
        float y = 0.0f;
        float relativeX = 0.0f;
        float relativeY = 0.0f;
        bool buttonDown = false;

        for (unsigned int bindingIndex = 0; bindingIndex < action->bindingCount; bindingIndex++) {
            const struct inputBinding *binding = &action->bindings[bindingIndex];
            float value = sampleBinding(input, binding);
            bool relative = relativeBinding(binding);

            if (action->type == INPUT_ACTION_BUTTON) {
                if (fabsf(value) >= binding->threshold) buttonDown = true;
            }
            else if (action->type == INPUT_ACTION_AXIS_1D) {
                if (relative) relativeX += value;
                else x += value;
            }
            else if (binding->target == INPUT_TARGET_Y) {
                if (relative) relativeY += value;
                else y += value;
            }
            else {
                if (relative) relativeX += value;
                else x += value;
            }
        }

        if (action->type == INPUT_ACTION_BUTTON) {
            x = buttonDown ? 1.0f : 0.0f;
            y = 0.0f;
        }
        else if (action->type == INPUT_ACTION_AXIS_1D) {
            x = clampValue(x, -1.0f, 1.0f);
            x = applyDeadzone1D(x, action->deadzone);
            x += relativeX;
        }
        else {
            float magnitude = sqrtf(x * x + y * y);
            if (magnitude > 1.0f) {
                x /= magnitude;
                y /= magnitude;
            }
            applyDeadzone2D(&x, &y, action->deadzone);
            x += relativeX;
            y += relativeY;
        }

        bool down = action->type == INPUT_ACTION_BUTTON
            ? buttonDown
            : fabsf(x) > 0.0001f || fabsf(y) > 0.0001f;

        action->x = x;
        action->y = y;
        action->pressed = down && !action->previousDown;
        action->released = !down && action->previousDown;
        action->down = down;
        action->previousDown = down;
    }
}

int inputActionIndex(const struct inputComponent *input, const char *name) {
    if (input == NULL || name == NULL) return -1;

    for (unsigned int i = 0; i < input->actionCount; i++) {
        if (strcmp(input->actions[i].name, name) == 0) return (int)i;
    }

    return -1;
}

struct inputAction *inputActionPointer(struct inputComponent *input, const char *name) {
    int index = inputActionIndex(input, name);
    if (index < 0) return NULL;
    return &input->actions[index];
}

static bool validActionName(const char *name) {
    if (name == NULL) return false;

    size_t length = strlen(name);
    if (length == 0 || length >= INPUT_ACTION_NAME_LENGTH) return false;

    for (size_t i = 0; i < length; i++) {
        char c = name[i];
        bool valid = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
            || (c >= '0' && c <= '9') || c == '_';
        if (!valid) return false;
    }

    return true;
}

static int parseTargetName(const char *name) {
    if (name == NULL || strcmp(name, "x") == 0) return INPUT_TARGET_X;
    if (strcmp(name, "y") == 0) return INPUT_TARGET_Y;
    return -1;
}

static bool validateBinding(const struct inputAction *action, const struct inputBinding *binding) {
    switch (binding->device) {
        case INPUT_DEVICE_KEYBOARD:
            if (binding->control < 0 || binding->control > GLFW_KEY_LAST) {
                setError("keyboard control is out of range");
                return false;
            }
            break;

        case INPUT_DEVICE_MOUSE:
            if (!((binding->control >= 0 && binding->control <= GLFW_MOUSE_BUTTON_LAST)
                || (binding->control >= INPUT_MOUSE_DELTA_X && binding->control <= INPUT_MOUSE_WHEEL_Y))) {
                setError("mouse control is out of range");
                return false;
            }
            if (action->type == INPUT_ACTION_BUTTON && binding->control >= INPUT_MOUSE_DELTA_X) {
                setError("mouse axes cannot drive button actions");
                return false;
            }
            break;

        case INPUT_DEVICE_GAMEPAD:
            if (!((binding->control >= 0 && binding->control < INPUT_GAMEPAD_BUTTON_COUNT)
                || (binding->control >= INPUT_GAMEPAD_AXIS_BASE
                    && binding->control < INPUT_GAMEPAD_AXIS_BASE + INPUT_GAMEPAD_AXIS_COUNT))) {
                setError("gamepad control is out of range");
                return false;
            }
            break;

        default:
            setError("unknown input device");
            return false;
    }

    if (binding->player < -1 || binding->player >= INPUT_MAX_GAMEPADS) {
        setError("player must be -1 or 0..%d", INPUT_MAX_GAMEPADS - 1);
        return false;
    }

    if (!isfinite(binding->scale)) {
        setError("binding scale is not a number");
        return false;
    }

    if (!isfinite(binding->threshold) || binding->threshold < 0.0f || binding->threshold > 1.0f) {
        setError("binding threshold must be in 0..1");
        return false;
    }

    if (action->type == INPUT_ACTION_AXIS_2D
        && binding->target != INPUT_TARGET_X && binding->target != INPUT_TARGET_Y) {
        setError("2D binding target must be x or y");
        return false;
    }

    return true;
}

static enum inputAxisTarget effectiveTarget(const struct inputAction *action, const struct inputBinding *binding) {
    return action->type == INPUT_ACTION_AXIS_2D ? binding->target : INPUT_TARGET_X;
}

static int findDuplicateBinding(const struct inputAction *action, const struct inputBinding *binding, int skipIndex) {
    for (unsigned int i = 0; i < action->bindingCount; i++) {
        if ((int)i == skipIndex) continue;

        const struct inputBinding *existing = &action->bindings[i];
        if (existing->device != binding->device || existing->control != binding->control) continue;
        if (effectiveTarget(action, existing) != effectiveTarget(action, binding)) continue;

        if (binding->device == INPUT_DEVICE_GAMEPAD) {
            bool overlap = existing->player == -1 || binding->player == -1
                || existing->player == binding->player;
            if (!overlap) continue;
        }

        return (int)i;
    }

    return -1;
}

static bool appendBinding(struct inputAction *action, const struct inputBinding *binding) {
    if (action->bindingCount >= INPUT_MAX_BINDINGS) {
        setError("action '%s' has no free binding slots", action->name);
        return false;
    }

    if (!validateBinding(action, binding)) return false;

    int duplicate = findDuplicateBinding(action, binding, -1);
    if (duplicate >= 0) {
        setError("action '%s' already has this binding", action->name);
        return false;
    }

    action->bindings[action->bindingCount] = *binding;
    action->bindingCount++;
    return true;
}

bool inputAddAction(struct inputComponent *input, const char *name, enum inputActionType type, float deadzone) {
    clearError();

    if (input == NULL) {
        setError("input component is missing");
        return false;
    }

    if (!validActionName(name)) {
        setError("action name must use 1..%d letters, digits, or underscore", INPUT_ACTION_NAME_LENGTH - 1);
        return false;
    }

    if (inputActionIndex(input, name) >= 0) {
        setError("action '%s' already exists", name);
        return false;
    }

    if (type < 0 || type >= INPUT_ACTION_TYPE_MAX) {
        setError("action '%s' has an unknown type", name);
        return false;
    }

    if (type != INPUT_ACTION_BUTTON && (!isfinite(deadzone) || deadzone < 0.0f || deadzone >= 1.0f)) {
        setError("action '%s' deadzone must be in 0..1", name);
        return false;
    }

    if (input->actionCount >= INPUT_MAX_ACTIONS) {
        setError("no free action slots (max %d)", INPUT_MAX_ACTIONS);
        return false;
    }

    struct inputAction *action = &input->actions[input->actionCount];
    memset(action, 0, sizeof(*action));
    snprintf(action->name, sizeof(action->name), "%s", name);
    action->type = type;
    action->deadzone = type == INPUT_ACTION_BUTTON ? 0.0f : deadzone;
    input->actionCount++;

    return true;
}

static bool parseBindingArguments(const char *deviceName, const char *controlName, const char *targetName,
    float scale, int player, float threshold, struct inputBinding *binding) {

    int device = parseDeviceName(deviceName);
    if (device < 0) {
        setError("unknown device '%s'", deviceName == NULL ? "" : deviceName);
        return false;
    }

    int control = inputControlCode((enum inputDevice)device, controlName);
    if (control < 0) {
        setError("unknown %s control '%s'", inputDeviceName((enum inputDevice)device),
            controlName == NULL ? "" : controlName);
        return false;
    }

    int target = parseTargetName(targetName);
    if (target < 0) {
        setError("target must be x or y");
        return false;
    }

    binding->device = (enum inputDevice)device;
    binding->control = control;
    binding->target = (enum inputAxisTarget)target;
    binding->scale = scale;
    binding->threshold = threshold;
    binding->player = player;

    return true;
}

bool inputBind(struct inputComponent *input, const char *actionName, const char *deviceName,
    const char *controlName, const char *targetName, float scale, int player, float threshold) {

    clearError();

    int index = inputActionIndex(input, actionName);
    if (index < 0) {
        setError("unknown action '%s'", actionName == NULL ? "" : actionName);
        return false;
    }

    struct inputAction *action = &input->actions[index];
    struct inputBinding binding;

    if (!parseBindingArguments(deviceName, controlName, targetName, scale, player, threshold, &binding)) {
        return false;
    }

    if (action->type != INPUT_ACTION_AXIS_2D) binding.target = INPUT_TARGET_X;

    return appendBinding(action, &binding);
}

bool inputSetBinding(struct inputComponent *input, const char *actionName, unsigned int index,
    const char *deviceName, const char *controlName, const char *targetName,
    float scale, int player, float threshold) {

    clearError();

    int actionIndex = inputActionIndex(input, actionName);
    if (actionIndex < 0) {
        setError("unknown action '%s'", actionName == NULL ? "" : actionName);
        return false;
    }

    struct inputAction *action = &input->actions[actionIndex];
    if (index >= action->bindingCount) {
        setError("action '%s' binding index %u is out of range", action->name, index);
        return false;
    }

    struct inputBinding binding;
    if (!parseBindingArguments(deviceName, controlName, targetName, scale, player, threshold, &binding)) {
        return false;
    }

    if (action->type != INPUT_ACTION_AXIS_2D) binding.target = INPUT_TARGET_X;

    if (!validateBinding(action, &binding)) return false;

    int duplicate = findDuplicateBinding(action, &binding, (int)index);
    if (duplicate >= 0) {
        setError("action '%s' already has this binding", action->name);
        return false;
    }

    action->bindings[index] = binding;
    return true;
}

bool inputRemoveBinding(struct inputComponent *input, const char *actionName, unsigned int index) {
    clearError();

    int actionIndex = inputActionIndex(input, actionName);
    if (actionIndex < 0) {
        setError("unknown action '%s'", actionName == NULL ? "" : actionName);
        return false;
    }

    struct inputAction *action = &input->actions[actionIndex];
    if (index >= action->bindingCount) {
        setError("action '%s' binding index %u is out of range", action->name, index);
        return false;
    }

    for (unsigned int i = index; i + 1 < action->bindingCount; i++) {
        action->bindings[i] = action->bindings[i + 1];
    }

    action->bindingCount--;
    return true;
}

static bool keyPressed(const struct inputComponent *input, int key) {
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    return input->keys[key] && !input->previousKeys[key];
}

static bool mouseButtonPressed(const struct inputComponent *input, int button) {
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
    return input->mouseButtons[button] && !input->previousMouseButtons[button];
}

static bool gamepadButtonPressed(const struct inputComponent *input, int player, int button) {
    const struct inputGamepad *pad = &input->gamepads[player];
    if (!pad->connected || !pad->mapped) return false;
    return pad->buttons[button] != 0 && pad->previousButtons[button] == 0;
}

static bool anyInputHeld(const struct inputComponent *input) {
    for (int key = 0; key <= GLFW_KEY_LAST; key++) {
        if (input->keys[key]) return true;
    }

    for (int button = 0; button <= GLFW_MOUSE_BUTTON_LAST; button++) {
        if (input->mouseButtons[button]) return true;
    }

    for (int player = 0; player < INPUT_MAX_GAMEPADS; player++) {
        const struct inputGamepad *pad = &input->gamepads[player];
        if (!pad->connected || !pad->mapped) continue;

        for (int button = 0; button < INPUT_GAMEPAD_BUTTON_COUNT; button++) {
            if (pad->buttons[button] != 0) return true;
        }

        for (int axis = 0; axis < INPUT_GAMEPAD_AXIS_COUNT; axis++) {
            if (fabsf(pad->axes[axis]) > 0.3f) return true;
        }
    }

    return false;
}

bool inputBeginCapture(struct inputComponent *input, const char *actionName, int bindingIndex, const char *targetName) {
    clearError();

    int actionIndex = inputActionIndex(input, actionName);
    if (actionIndex < 0) {
        setError("unknown action '%s'", actionName == NULL ? "" : actionName);
        return false;
    }

    struct inputAction *action = &input->actions[actionIndex];

    if (bindingIndex >= (int)action->bindingCount) {
        setError("action '%s' binding index %d is out of range", action->name, bindingIndex);
        return false;
    }

    enum inputAxisTarget target = INPUT_TARGET_X;

    if (bindingIndex >= 0) {
        target = action->bindings[bindingIndex].target;
    }
    else if (action->type == INPUT_ACTION_AXIS_2D) {
        int parsedTarget = parseTargetName(targetName);
        if (parsedTarget < 0) {
            setError("2D action '%s' target must be x or y", action->name);
            return false;
        }
        target = (enum inputAxisTarget)parsedTarget;
    }

    input->capture.active = true;
    input->capture.armed = false;
    input->capture.actionIndex = actionIndex;
    input->capture.bindingIndex = bindingIndex;
    input->capture.target = target;
    input->capture.capturedDevice = -1;
    input->capture.capturedControl = -1;
    input->capture.capturedTarget = -1;
    input->capture.capturedPlayer = -1;

    return true;
}

void inputCancelCapture(struct inputComponent *input) {
    if (input == NULL) return;

    input->capture.active = false;
    input->capture.armed = false;
}

bool inputCaptureActive(const struct inputComponent *input) {
    return input != NULL && input->capture.active;
}

static bool storeCapture(struct inputComponent *input, int device, int control, int player) {
    input->capture.capturedDevice = device;
    input->capture.capturedControl = control;
    input->capture.capturedTarget = input->capture.target;
    input->capture.capturedPlayer = player;
    input->capture.active = false;
    input->capture.armed = false;
    return true;
}

bool inputCapturePoll(struct inputComponent *input) {
    if (input == NULL || !input->capture.active) return false;

    if (!input->capture.armed) {
        if (anyInputHeld(input)) return false;
        input->capture.armed = true;
        return false;
    }

    if (keyPressed(input, GLFW_KEY_ESCAPE)) {
        inputCancelCapture(input);
        return false;
    }

    for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; key++) {
        if (keyPressed(input, key)) return storeCapture(input, INPUT_DEVICE_KEYBOARD, key, -1);
    }

    for (int button = 0; button <= GLFW_MOUSE_BUTTON_LAST; button++) {
        if (mouseButtonPressed(input, button)) return storeCapture(input, INPUT_DEVICE_MOUSE, button, -1);
    }

    for (int player = 0; player < INPUT_MAX_GAMEPADS; player++) {
        const struct inputGamepad *pad = &input->gamepads[player];
        if (!pad->connected || !pad->mapped) continue;

        for (int button = 0; button < INPUT_GAMEPAD_BUTTON_COUNT; button++) {
            if (gamepadButtonPressed(input, player, button)) {
                return storeCapture(input, INPUT_DEVICE_GAMEPAD, button, player);
            }
        }

        for (int axis = 0; axis < INPUT_GAMEPAD_AXIS_COUNT; axis++) {
            if (fabsf(pad->axes[axis]) > 0.6f) {
                return storeCapture(input, INPUT_DEVICE_GAMEPAD, INPUT_GAMEPAD_AXIS_BASE + axis, player);
            }
        }
    }

    if (input->scrollX != 0.0) {
        return storeCapture(input, INPUT_DEVICE_MOUSE, INPUT_MOUSE_WHEEL_X, -1);
    }
    if (input->scrollY != 0.0) {
        return storeCapture(input, INPUT_DEVICE_MOUSE, INPUT_MOUSE_WHEEL_Y, -1);
    }

    return false;
}

int inputCapturedDevice(const struct inputComponent *input) {
    return input == NULL ? -1 : input->capture.capturedDevice;
}

int inputCapturedControl(const struct inputComponent *input) {
    return input == NULL ? -1 : input->capture.capturedControl;
}

int inputCapturedTarget(const struct inputComponent *input) {
    return input == NULL ? -1 : input->capture.capturedTarget;
}

int inputCapturedPlayer(const struct inputComponent *input) {
    return input == NULL ? -1 : input->capture.capturedPlayer;
}

int inputFindConflict(struct inputComponent *input, const char *actionName, const char *deviceName,
    const char *controlName, const char *targetName, int player) {

    clearError();

    int sourceIndex = inputActionIndex(input, actionName);
    if (sourceIndex < 0) {
        setError("unknown action '%s'", actionName == NULL ? "" : actionName);
        return -1;
    }

    int device = parseDeviceName(deviceName);
    if (device < 0) {
        setError("unknown device '%s'", deviceName == NULL ? "" : deviceName);
        return -1;
    }

    int control = inputControlCode((enum inputDevice)device, controlName);
    if (control < 0) {
        setError("unknown %s control '%s'", inputDeviceName((enum inputDevice)device),
            controlName == NULL ? "" : controlName);
        return -1;
    }

    struct inputAction *source = &input->actions[sourceIndex];
    enum inputAxisTarget sourceTarget = INPUT_TARGET_X;

    if (source->type == INPUT_ACTION_AXIS_2D) {
        int parsedTarget = parseTargetName(targetName);
        if (parsedTarget < 0) {
            setError("2D action '%s' target must be x or y", source->name);
            return -1;
        }
        sourceTarget = (enum inputAxisTarget)parsedTarget;
    }

    for (unsigned int actionIndex = 0; actionIndex < input->actionCount; actionIndex++) {
        if ((int)actionIndex == sourceIndex) continue;

        struct inputAction *action = &input->actions[actionIndex];

        for (unsigned int bindingIndex = 0; bindingIndex < action->bindingCount; bindingIndex++) {
            struct inputBinding *binding = &action->bindings[bindingIndex];

            if (binding->device != (enum inputDevice)device || binding->control != control) continue;
            if (effectiveTarget(action, binding) != sourceTarget) continue;

            if (device == INPUT_DEVICE_GAMEPAD) {
                bool overlap = binding->player == -1 || player == -1 || binding->player == player;
                if (!overlap) continue;
            }

            return (int)actionIndex;
        }
    }

    return -1;
}

void inputSetUiCapture(struct inputComponent *input, bool keyboard, bool mouse) {
    if (input == NULL) return;

    input->uiCaptureKeyboard = keyboard;
    input->uiCaptureMouse = mouse;
}

void inputSetCursorMode(struct inputComponent *input, enum inputCursorMode mode) {
    int glfwMode = mode == INPUT_CURSOR_NORMAL ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
    bool raw = mode == INPUT_CURSOR_RAW && glfwRawMouseMotionSupported();

    glfwSetInputMode(globalState.window, GLFW_RAW_MOUSE_MOTION, raw ? GLFW_TRUE : GLFW_FALSE);
    glfwSetInputMode(globalState.window, GLFW_CURSOR, glfwMode);

    if (input != NULL) input->resetMouse = true;
}

void updateInput(struct systemRunData data) {
    struct inputComponent *input = GET_SYSTEM_COMPONENTS(data, 0, 0);

    pollDevices(&input[0]);
    resolveActions(&input[0]);
}
