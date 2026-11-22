#ifndef GOLDFISH_INPUT_H
#define GOLDFISH_INPUT_H

#include <stdbool.h>

#define INPUT_MAX_ACTIONS 64
#define INPUT_MAX_BINDINGS 16
#define INPUT_MAX_GAMEPADS 16
#define INPUT_ACTION_NAME_LENGTH 32

#define INPUT_KEY_COUNT 512
#define INPUT_MOUSE_BUTTON_COUNT 8
#define INPUT_GAMEPAD_BUTTON_COUNT 15
#define INPUT_GAMEPAD_AXIS_COUNT 6

enum inputDevice {
    INPUT_DEVICE_KEYBOARD,
    INPUT_DEVICE_MOUSE,
    INPUT_DEVICE_GAMEPAD,

    INPUT_DEVICE_MAX,
};

enum inputActionType {
    INPUT_ACTION_BUTTON,
    INPUT_ACTION_AXIS_1D,
    INPUT_ACTION_AXIS_2D,

    INPUT_ACTION_TYPE_MAX,
};

enum inputAxisTarget {
    INPUT_TARGET_X,
    INPUT_TARGET_Y,
};

enum inputCursorMode {
    INPUT_CURSOR_NORMAL,
    INPUT_CURSOR_DISABLED,
    INPUT_CURSOR_RAW,
};

enum inputMouseControl {
    INPUT_MOUSE_LEFT = 0,
    INPUT_MOUSE_RIGHT = 1,
    INPUT_MOUSE_MIDDLE = 2,
    INPUT_MOUSE_X1 = 3,
    INPUT_MOUSE_X2 = 4,

    INPUT_MOUSE_DELTA_X = 100,
    INPUT_MOUSE_DELTA_Y = 101,
    INPUT_MOUSE_WHEEL_X = 102,
    INPUT_MOUSE_WHEEL_Y = 103,
};

#define INPUT_GAMEPAD_AXIS_BASE 100

struct inputBinding {
    enum inputDevice device;
    int control;
    enum inputAxisTarget target;
    float scale;
    float threshold;
    int player;
};

struct inputAction {
    char name[INPUT_ACTION_NAME_LENGTH];
    enum inputActionType type;
    float deadzone;

    struct inputBinding bindings[INPUT_MAX_BINDINGS];
    unsigned int bindingCount;

    float x;
    float y;

    bool down;
    bool pressed;
    bool released;
    bool previousDown;
};

struct inputGamepad {
    bool connected;
    bool mapped;
    char name[128];
    char guid[33];

    float axes[INPUT_GAMEPAD_AXIS_COUNT];
    unsigned char buttons[INPUT_GAMEPAD_BUTTON_COUNT];
    unsigned char previousButtons[INPUT_GAMEPAD_BUTTON_COUNT];
};

struct inputCaptureState {
    bool active;
    bool armed;
    int actionIndex;
    int bindingIndex;

    enum inputAxisTarget target;

    int capturedDevice;
    int capturedControl;
    int capturedTarget;
    int capturedPlayer;
};

struct inputComponent;

void inputInit(void);
void inputEnd(void);
void inputReset(struct inputComponent *input);
void inputSetUiCapture(struct inputComponent *input, bool keyboard, bool mouse);
void inputSetCursorMode(struct inputComponent *input, enum inputCursorMode mode);

bool inputAddAction(struct inputComponent *input, const char *name, enum inputActionType type, float deadzone);
bool inputBind(struct inputComponent *input, const char *action, const char *device, const char *control, const char *target, float scale, int player, float threshold);
bool inputSetBinding(struct inputComponent *input, const char *action, unsigned int index, const char *device, const char *control, const char *target, float scale, int player, float threshold);
bool inputRemoveBinding(struct inputComponent *input, const char *action, unsigned int index);

int inputActionIndex(const struct inputComponent *input, const char *name);
struct inputAction *inputActionPointer(struct inputComponent *input, const char *name);

bool inputBeginCapture(struct inputComponent *input, const char *action, int bindingIndex, const char *target);
void inputCancelCapture(struct inputComponent *input);
bool inputCaptureActive(const struct inputComponent *input);
bool inputCapturePoll(struct inputComponent *input);
int inputCapturedDevice(const struct inputComponent *input);
int inputCapturedControl(const struct inputComponent *input);
int inputCapturedTarget(const struct inputComponent *input);
int inputCapturedPlayer(const struct inputComponent *input);
int inputFindConflict(struct inputComponent *input, const char *action, const char *device, const char *control, const char *target, int player);

int inputDeviceCode(const char *name);
int inputControlCode(enum inputDevice device, const char *name);
const char *inputControlName(enum inputDevice device, int control);
const char *inputDeviceName(enum inputDevice device);
const char *inputLastError(void);

#endif
