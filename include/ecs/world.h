#ifndef WORLD_H
#define WORLD_H

#define MAX_ENTITY_COUNT 50
#define MAX_COMPONENT_COUNT 50
#define MAX_SYSTEM_COUNT 20
#define MAX_ARCHETYPE_COUNT 50
#define MAX_FILTER_COUNT 10

#define INVALID_POSITION (UINT_MAX)

#include <data/hashtable.h>
#include <data/dynarray.h>

#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

typedef char * componentId;
typedef char * entityId;
typedef unsigned int tableId;
typedef char * filterId;

union genericId {
    filterId filter;
	tableId table;
	entityId entity;
	componentId component;
};

enum systemEvent {
    SYSTEM_ON_COMPONENT_ADD,

    SYSTEM_EVENT_MAX,
};

enum systemPhase {
    SYSTEM_ON_PRE_UPDATE,
    SYSTEM_ON_UPDATE,
    SYSTEM_ON_POST_UPDATE,

    SYSTEM_ON_RENDER_SETUP,
    SYSTEM_ON_RENDER_OPAQUE,
    SYSTEM_ON_RENDER_SKYBOX,
    SYSTEM_ON_RENDER_TRANSPARENT,
    SYSTEM_ON_RENDER_POST,
    
    SYSTEM_PHASE_MAX,
};

struct systemRunData {
    struct world *world;

    union genericId filterResults[MAX_FILTER_COUNT];
    unsigned int filterResultsLength;

    struct system *system;
    float dt;
};

struct filter {
    componentId components[MAX_COMPONENT_COUNT];
    unsigned int componentsLength;

    tableId results[MAX_ARCHETYPE_COUNT];
    unsigned int resultsLength;
};

struct system {
    const char *name;

    filterId filters[MAX_FILTER_COUNT];
    unsigned int filtersLength;

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
    struct dynarray tables;

    struct hashtable components;

    struct hashtable entities;
    entityId singletonEntity;

    struct dynarray phaseSystems[SYSTEM_PHASE_MAX];
    struct dynarray eventSystems[SYSTEM_EVENT_MAX];

    struct hashtable filters;
};

#include <macros.h>

#define COMPONENT_SIZE(w, c) ((struct component*)hashtableGet(&(w)->components, c))->size
#define GET_COMPONENT_ID(c) STRINGIFY(c)

#define CREATE_COMPONENT(w, c) createComponent((w), STRINGIFY(c), sizeof(c))
#define GET_COMPONENT(w, e, c) getComponent((w), (e), STRINGIFY(c))
#define ADD_COMPONENT(w, e, c) addComponent((w), (e), STRINGIFY(c))

#define GET_SINGLETON_COMPONENT(w, c) getSingletonComponent((w), STRINGIFY(c))
#define ADD_SINGLETON_COMPONENT(w, c) addSingletonComponent((w), STRINGIFY(c))

#define GET_FILTER_COMPONENT(w, f, i) ((struct filter *)hashtableGet(&(w)->filters, (f)))->components[i]

#define GET_SYSTEM_COMPONENT(d) getComponent((d).world, (d).filterResults[0].entity, GET_FILTER_COMPONENT((d).world, (d).system->filters[0], 0))
#define GET_SYSTEM_COMPONENTS(d, i) getComponentsFromTable((d).world, (d).filterResults[0].table, GET_FILTER_COMPONENT((d).world, (d).system->filters[0], i))
#define GET_SYSTEM_COMPONENTS_LENGTH(d) (((struct table *)dynarrayGet(&(d).world->tables, (d).filterResults[0].table))->componentsLength)

//#define COMPONENT_PACKAGE(...) ({ __VA_ARGS__, VARIADIC_COUNT(__VA_ARGS__)})
//#define COMPONENT_LIST(...) { __VA_ARGS__, VARIADIC_COUNT(__VA_ARGS__) }

#define ADD_PHASE_SYSTEM(w, ph, callback, ...) do { \
	char *name = STRINGIFY(callback); \
    struct filter f = { \
        { __VA_ARGS__ }, \
        VARIADIC_COUNT(char *, __VA_ARGS__), \
        { 0 }, \
        0 \
    }; \
    addFilter(w, name, f); \
    struct system s = { \
        name, \
        { name }, \
        1, \
        callback \
    }; \
    addPhaseSystem(w, ph, s); \
} while (0); \

#define ADD_EVENT_SYSTEM(w, ev, callback, component) do { \
	char *name = STRINGIFY(callback); \
    struct filter f = { \
        { component }, \
        1, \
        { 0 }, \
        0 \
    }; \
    addFilter(w, name, f); \
    struct system s = { \
        name, \
        { name }, \
        1, \
        callback \
    }; \
    addEventSystem(w, ev, s); \
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

void addPhaseSystem(struct world *world, enum systemPhase phase, struct system system);
void addEventSystem(struct world *world, enum systemEvent event, struct system system);
void addFilter(struct world *world, const char *name, struct filter filter);

void runWorldPhase(struct world *world, enum systemPhase phase, float dt);
void printWorld(struct world *world);

#endif
