#ifndef WORLD_H
#define WORLD_H

#define MAX_ENTITY_COUNT 50
#define MAX_COMPONENT_COUNT 50
#define MAX_SYSTEM_COUNT 20
#define MAX_ARCHETYPE_COUNT 50

#define INVALID_POSITION (UINT_MAX)

#include <data/hashtable.h>

#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

typedef char * componentId;
typedef char * entityId;
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
    float dt;
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
    char *name;
    unsigned int table;
    unsigned int position;
};

struct component {
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

    struct hashtable components;

    struct hashtable entities;
    entityId singletonEntity;

    struct system systems[MAX_SYSTEM_COUNT];
    unsigned int systemsLength;
};

#include <macros.h>

#define COMPONENT_SIZE(w, c) ((struct component*)hashtableGet(&(w)->components, c))->size
#define GET_COMPONENT_ID(c) STRINGIFY(c)

#define CREATE_COMPONENT(w, c) createComponent((w), STRINGIFY(c), sizeof(c))
#define GET_COMPONENT(w, e, c) getComponent((w), (e), STRINGIFY(c))
#define ADD_COMPONENT(w, e, c) addComponent((w), (e), STRINGIFY(c))

#define GET_SINGLETON_COMPONENT(w, c) getSingletonComponent((w), STRINGIFY(c))
#define ADD_SINGLETON_COMPONENT(w, c) addSingletonComponent((w), STRINGIFY(c))

#define GET_SYSTEM_COMPONENT(d) getComponent((d).world, (d).entity, (d).system->components[0])
#define GET_SYSTEM_COMPONENTS(d, i) getComponentsFromTable((d).world, (d).table, (d).system->components[i])
#define GET_SYSTEM_COMPONENTS_LENGTH(d) ((d).world->tables[(d).table].componentsLength)

//#define COMPONENT_PACKAGE(...) ({ __VA_ARGS__, VARIADIC_COUNT(__VA_ARGS__)})
//#define COMPONENT_LIST(...) { __VA_ARGS__, VARIADIC_COUNT(__VA_ARGS__) }

#define ADD_SYSTEM(w, pr, ph, callback, ...) do { \
    struct system s = { \
        STRINGIFY(callback), \
        pr, \
        ph, \
        { __VA_ARGS__ }, \
        VARIADIC_COUNT(char *, __VA_ARGS__), \
        callback \
    }; \
    addSystem(w, s); \
} while (0); \

struct world createWorld();
void destroyWorld(struct world *world);

void createComponent(struct world *world, const char *component, unsigned int componentSize);

void addComponent(struct world *world, entityId entity, componentId component);
void *getComponent(struct world *world, entityId entity, componentId component);
void *getComponentsFromTable(struct world *world, tableId table, componentId component);
unsigned int getAllTablesWithComponents(struct world *world, componentId *components, unsigned int componentsLength, tableId *tables, unsigned int tablesLength);
void removeComponent(struct world *world, entityId entity, componentId component);

void addSingletonComponent(struct world *world, componentId component);
void *getSingletonComponent(struct world *world, componentId component);
void removeSingletonComponent(struct world *world, componentId component);

entityId createEntity(struct world *world, const char *name);
void deleteEntity(struct world *world, entityId id);
void addSystem(struct world *world, struct system system);

void runWorldPhase(struct world *world, enum systemPhase phase, float dt);
void printWorld(struct world *world);

#endif
