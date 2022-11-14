#ifndef WORLD_H
#define WORLD_H

#define MAX_ENTITY_COUNT 50
#define MAX_COMPONENT_COUNT 50
#define MAX_SYSTEM_COUNT 20
#define MAX_ARCHETYPE_COUNT 50

#define INVALID_POSITION (UINT_MAX)

#include <ecs/components.h>

#include <stdbool.h>
#include <stdint.h>

typedef uint32_t componentId;
typedef unsigned int entityId;
typedef unsigned int tableId;

enum systemPhase {
    SYSTEM_ON_CREATE,
    SYSTEM_ON_UPDATE,
    SYSTEM_ON_RENDER,
    SYSTEM_ON_DELETE
};

struct systemRunData {
    struct world *world;

    union {
        tableId table;
        entityId entity;
    };

    struct system *system;
};

struct system {
    const char *name;

    int priority;
    enum systemPhase phase;

    componentId components[MAX_COMPONENT_COUNT];
    unsigned int componentsLength;

    void (*callback)(struct systemRunData);
};

struct entity {
    unsigned int table;
    unsigned int position;
};

struct component {
    bool valid;
    uint32_t hash;
    const char *name;
    unsigned int size;
};

struct record {
    componentId componentType;
    void *components;
};

struct table {
    struct record *records;
    unsigned int recordsLength;
    unsigned int componentsLength;
};

struct world {
    struct table tables[MAX_ARCHETYPE_COUNT];
    unsigned int tablesLength;

    struct component *components;
    unsigned int componentsLength;

    struct entity entities[MAX_ENTITY_COUNT];
    bool validEntities[MAX_ENTITY_COUNT];

    struct system systems[MAX_SYSTEM_COUNT];
    unsigned int systemsLength;
};

#define STRINGIFY(c) #c
#define VARIADIC_COUNT(...) ((int)(sizeof((int[]){__VA_ARGS__})/sizeof(int)))
#define UNPARENTHESES(...) __VA_ARGS__

#define COMPONENT_SIZE(w, c) (w->components[c % w->componentsLength].size)

#define CREATE_COMPONENT(w, c) createComponent(w, STRINGIFY(c), sizeof(STRINGIFY(c)), sizeof(c))
#define GET_COMPONENT_ID(c) getComponentId(STRINGIFY(c), sizeof(STRINGIFY(c)))
#define GET_SYSTEM_COMPONENT(d) getComponent(d.world, d.entity, d.system->components[0])
#define GET_SYSTEM_COMPONENTS(d, i) getComponentFromTable(d.world, d.table, d.system->components[i])
#define GET_SYSTEM_COMPONENTS_LENGTH(d) (d.world->tables[d.table].componentsLength)

//#define COMPONENT_PACKAGE(...) ({ __VA_ARGS__, VARIADIC_COUNT(__VA_ARGS__)})
//#define COMPONENT_LIST(...) { __VA_ARGS__, VARIADIC_COUNT(__VA_ARGS__) }

#define ADD_SYSTEM(w, pr, ph, callback, ...) do { \
    struct system s = { \
        STRINGIFY(callback), \
        pr, \
        ph, \
        { __VA_ARGS__ }, \
        VARIADIC_COUNT(__VA_ARGS__), \
        callback \
    }; \
    addSystem(w, s); \
} while (0); \

struct world createWorld();

componentId createComponent(struct world *world, const char *component, unsigned int componentLength, unsigned int componentSize);
componentId getComponentId(const char *component, unsigned int componentLength);

void addComponent(struct world *world, entityId entity, componentId component);
void *getComponent(struct world *world, entityId entity, componentId component);
void *getComponentFromTable(struct world *world, tableId table, componentId component);
unsigned int getAllTablesWithComponents(struct world *world, componentId *components, unsigned int componentsLength, tableId *tables, unsigned int tablesLength);
void removeComponent(struct world *world, entityId entity, componentId component);

void addSingletonComponent(struct world *world, componentId component);
void *getSingletonComponent(struct world *world, componentId component);
void removeSingletonComponent(struct world *world, componentId component);

entityId createEntity(struct world *world);
void deleteEntity(struct world *world, entityId id);
void addSystem(struct world *world, struct system system);

void runWorldPhase(struct world *world, enum systemPhase phase);
void printWorld(struct world *world);

#endif
