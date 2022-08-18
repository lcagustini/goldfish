#ifndef SYSTEM_H
#define SYSTEM_H

struct world;

enum systemPhase {
    SYSTEM_ON_CREATE,
    SYSTEM_ON_UPDATE,
    SYSTEM_ON_RENDER,
    SYSTEM_ON_DELETE
};

struct system {
    int priority;
    enum systemPhase phase;
    void (*callback)(struct world *, unsigned int);
};

#endif
