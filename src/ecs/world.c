#include <ecs/world.h>

#include <print.h>

#include <string.h>
#include <assert.h>
#include <stdlib.h>

struct world createWorld() {
    struct world world = {0};

    world.components = HASHTABLE_CREATE(MAX_COMPONENT_COUNT, struct component);
    world.entities = HASHTABLE_CREATE(MAX_ENTITY_COUNT, struct entity);

    world.singletonEntity = createEntity(&world, "Singleton");

    return world;
}

void destroyWorld(struct world *world) {
    for (int i = 0; i < world->tablesLength; i++) {
        for (int j = 0; j < world->tables[i].recordsLength; j++) {
            free(world->tables[i].records[j].components);
        }
        free(world->tables[i].records);
    }

    hashtableDestroy(&world->components);
}

void createComponent(struct world *world, const char *component, unsigned int componentSize) {
    struct component newComponent = {
        component,
        componentSize
    };

    return hashtableSet(&world->components, component, &newComponent);
}

unsigned int getAllTablesWithComponents(struct world *world, componentId *components, unsigned int componentsLength, tableId *tables, unsigned int tablesLength) {
    int l = 0;
    for (int j = 0; j < world->tablesLength; j++) {
        if (world->tables[j].recordsLength < componentsLength) continue;
        if (world->tables[j].componentsLength == 0) continue;

        int hits = 0;
        for (int k = 0; k < componentsLength; k++) {
            for (int i = 0; i < world->tables[j].recordsLength; i++) {
                hits += strcmp(world->tables[j].records[i].componentType, components[k]) == 0;
            }
        }
        if (hits == componentsLength) {
            tables[l] = j;
            l++;
            if (l == tablesLength) return l;
        }
    }
    return l;
}

static tableId getTableForComponents(struct world *world, componentId *components, unsigned int componentsLength) {
    for (int j = 0; j < world->tablesLength; j++) {
        if (world->tables[j].recordsLength != componentsLength) continue;

        int hits = 0;
        for (int k = 0; k < componentsLength; k++) {
            for (int i = 0; i < componentsLength; i++) {
                hits += strcmp(world->tables[j].records[i].componentType, components[k]) == 0;
            }
        }
        if (hits == componentsLength) return j;
    }

    tableId len = world->tablesLength;
    world->tables[len].records = malloc(componentsLength * sizeof(struct record));
    world->tables[len].recordsLength = componentsLength;
    world->tables[len].componentsLength = 0;

    for (int i = 0; i < componentsLength; i++) {
        world->tables[len].records[i].componentType = components[i];
        world->tables[len].records[i].components = malloc(MAX_ENTITY_COUNT * COMPONENT_SIZE(world, components[i]));
    }

    world->tablesLength++;

    return len;
}

static struct entity addEntityToTable(struct world *world, tableId table, struct entity *entity) {
    struct table *t = &world->tables[table];

    struct entity e = {
        entity->name,
        table,
        t->componentsLength
    };

    t->componentsLength++;

    return e;
}

static void removeEntityFromTable(struct world *world, struct entity *entity) {
    struct table *t = &world->tables[entity->table];

    for (int j = 0; j < t->recordsLength; j++) {
        unsigned int size = COMPONENT_SIZE(world, t->records[j].componentType);
        for (int i = entity->position; i < t->componentsLength - 1; i++) {
            memcpy(t->records[j].components + (i * size), t->records[j].components + ((i + 1) * size), size);
        }
    }

    struct entity *entities = world->entities.buffer;
    for (int i = 0; i < world->entities.bufferCount; i++) {
        if (!world->entities.valids[i]) continue;

        if (entities[i].table == entity->table && entities[i].position > entity->position) {
            entities[i].position--;
        }
    }

    t->componentsLength--;
}

// Assumes tableTo exists already
static struct entity copyEntityBetweenTables(struct world *world, struct entity *entity, tableId tableTo) {
    tableId tableFrom = entity->table;
    struct table *tFrom = &world->tables[tableFrom];
    struct table *tTo = &world->tables[tableTo];

    unsigned int len = tTo->componentsLength;

    for (int j = 0; j < tFrom->recordsLength; j++) {
        for (int i = 0; i < tTo->recordsLength; i++) {
            if (strcmp(tFrom->records[j].componentType, tTo->records[i].componentType) == 0) {
                unsigned int size = COMPONENT_SIZE(world, tTo->records[i].componentType);
                memcpy(tTo->records[i].components + (len * size), tFrom->records[j].components + (entity->position * size), size);
                break;
            }
        }
    }

    struct entity e = {
        entity->name,
        tableTo,
        len
    };

    tTo->componentsLength++;

    return e;
}

void addComponent(struct world *world, entityId entity, componentId component) {
    struct entity *e = hashtableGet(&world->entities, entity);

    if (e->table == INVALID_POSITION) {
        tableId t = getTableForComponents(world, &component, 1);
        struct entity newEntity = addEntityToTable(world, t, e);
        hashtableSet(&world->entities, entity, &newEntity);
    }
    else {
        componentId components[MAX_COMPONENT_COUNT];
        for (int i = 0; i < world->tables[e->table].recordsLength; i++) {
            components[i] = world->tables[e->table].records[i].componentType;
        }
        components[world->tables[e->table].recordsLength] = component;
        tableId t = getTableForComponents(world, components, world->tables[e->table].recordsLength + 1);
        struct entity newEntity = copyEntityBetweenTables(world, e, t);
        removeEntityFromTable(world, e);
        hashtableSet(&world->entities, entity, &newEntity);
    }

    for (int i = 0; i < world->systemsLength; i++) {
        if (world->systems[i].phase != SYSTEM_ON_CREATE) continue;
        if (world->systems[i].components[0] != component) continue;

        struct systemRunData data = {
            world,
            { .entity = entity },
            &world->systems[i]
        };

        world->systems[i].callback(data);
    }
}

void *getComponent(struct world *world, entityId entity, componentId component) {
    if (entity == NULL) return NULL;

    struct entity *e = hashtableGet(&world->entities, entity);

    for (int i = 0; i < world->tables[e->table].recordsLength; i++) {
        if (strcmp(world->tables[e->table].records[i].componentType, component) == 0) {
            return world->tables[e->table].records[i].components + (e->position * COMPONENT_SIZE(world, component));
        }
    }

    return NULL;
}

void *getComponentsFromTable(struct world *world, tableId table, componentId component) {
    struct table t = world->tables[table];

    for (int i = 0; i < t.recordsLength; i++) {
        if (strcmp(t.records[i].componentType, component) == 0) return t.records[i].components;
    }

    return NULL;
}

void removeComponent(struct world *world, entityId entity, componentId component) {
    struct entity *e = hashtableGet(&world->entities, entity);

    if (e->table == INVALID_POSITION) {
        return;
    }
    else {
        componentId components[MAX_COMPONENT_COUNT];
        for (int i = 0; i < world->tables[e->table].recordsLength; i++) {
            if (strcmp(world->tables[e->table].records[i].componentType, component) == 0) continue;
            components[i] = world->tables[e->table].records[i].componentType;
        }
        tableId t = getTableForComponents(world, components, world->tables[e->table].recordsLength - 1);
        struct entity newEntity = copyEntityBetweenTables(world, e, t);
        removeEntityFromTable(world, e);
        hashtableSet(&world->entities, entity, &newEntity);
    }

    for (int i = 0; i < world->systemsLength; i++) {
        if (world->systems[i].phase != SYSTEM_ON_DELETE) continue;
        if (world->systems[i].components[0] != component) continue;

        struct systemRunData data = {
            world,
            { .entity = entity },
            &world->systems[i]
        };

        world->systems[i].callback(data);
    }
}

void addSingletonComponent(struct world *world, componentId component) {
    addComponent(world, world->singletonEntity, component);
}

void *getSingletonComponent(struct world *world, componentId component) {
    return getComponent(world, world->singletonEntity, component);
}

void removeSingletonComponent(struct world *world, componentId component) {
    removeComponent(world, world->singletonEntity, component);
}

entityId createEntity(struct world *world, const char *name) {
    struct entity e = {
        malloc((strlen(name) + 50) * sizeof(char)),
        INVALID_POSITION,
        INVALID_POSITION
    };
    sprintf(e.name, "(%d) %s", world->entities.validCount, name);

    hashtableSet(&world->entities, e.name, &e);

    return e.name;
}

void deleteEntity(struct world *world, entityId id) {
    struct entity *e = hashtableGet(&world->entities, id);
    struct table *t = &world->tables[e->table];

    for (int j = 0; j < t->recordsLength; j++) {
        componentId component = t->records[j].componentType;

        for (int i = 0; i < world->systemsLength; i++) {
            if (world->systems[i].phase != SYSTEM_ON_DELETE) continue;
            if (world->systems[i].components[0] != component) continue;

            struct systemRunData data = {
                world,
                { .entity = id },
                &world->systems[i]
            };

            world->systems[i].callback(data);
        }
    }

    free(e->name);

    hashtableRemove(&world->entities, id);
}

void addSystem(struct world *world, struct system system) {
    world->systems[world->systemsLength] = system;
    world->systemsLength++;

    for (int i = 1; i < world->systemsLength; i++) {
        struct system x = world->systems[i];
        int j = i - 1;

        while (j >= 0 && world->systems[j].priority > x.priority) {
            world->systems[j + 1] = world->systems[j];
            j = j - 1;
        }

        world->systems[j + 1] = x;
    }
}

void runWorldPhase(struct world *world, enum systemPhase phase, float dt) {
    for (int i = 0; i < world->systemsLength; i++) {
        if (world->systems[i].phase != phase) continue;

        tableId tables[MAX_ARCHETYPE_COUNT];
        unsigned int tablesLength = getAllTablesWithComponents(world, world->systems[i].components, world->systems[i].componentsLength, tables, MAX_ARCHETYPE_COUNT);

        for (int j = 0; j < tablesLength; j++) {
            struct systemRunData data = {
                world,
                { .table = tables[j] },
                &world->systems[i],
                dt
            };

            world->systems[i].callback(data);
        }
    }
}

void printWorld(struct world *world) {
    print("[ECS World]\n");

    print("[ECS Systems]\n");
    for (int i = 0; i < world->systemsLength; i++) {
        print("(name: %s) (priority: %d) (phase: %d) (components:", world->systems[i].name, world->systems[i].priority, world->systems[i].phase);
        for (int j = 0; j < world->systems[i].componentsLength; j++) {
            print(" %s,", world->systems[i].components[j]);
        }
        print("\b)\n");
    }
    print("\n");

    print("[ECS Entities]\n");
    //for (int i = 0; i < world->entitiesLength; i++) {
        //print("(id: %d) (name: %s) (table: %u) (row: %u)\n", i, world->entities[i].name, world->entities[i].table, world->entities[i].position);
    //}
    print("\n");

    print("[ECS Components (%d)]\n", world->components.bufferCount);
    struct component *components = world->components.buffer;
    for (int i = 0; i < world->components.bufferCount; i++) {
        if (!world->components.valids[i]) continue;

        print("(name: %s) (key: %s) (size: %u)\n", components[i].name, world->components.keys[i], components[i].size);
    }
    print("\n");

    print("[ECS Tables]\n");
    for (int i = 0; i < world->tablesLength; i++) {
        print("(id: %d) (records: %u) (rows: %u)\n", i, world->tables[i].recordsLength, world->tables[i].componentsLength);
        for (int j = 0; j < world->tables[i].recordsLength; j++) {
            print("(component: %s -> 0x%p)\n", world->tables[i].records[j].componentType, world->tables[i].records[j].components);
        }
        print("\n");
    }
}
