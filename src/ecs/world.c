#include <ecs/world.h>

#include <print.h>

#include <string.h>
#include <assert.h>
#include <stdlib.h>

struct world createWorld() {
    struct world world = {0};

    world.components = HASHTABLE_CREATE(MAX_COMPONENT_COUNT, struct component);
    world.entities = HASHTABLE_CREATE(MAX_ENTITY_COUNT, struct entity);

    world.singletonEntity = createEntity(&world);

    return world;
}

componentId createComponent(struct world *world, const char *component, unsigned int componentSize) {
    struct component newComponent = {
        component,
        componentSize
    };

    return hashtableSet(&world->components, component, &newComponent);
}

componentId getComponentId(const char *component) {
    return hashString(component, strlen(component));
}

unsigned int getAllTablesWithComponents(struct world *world, componentId *components, unsigned int componentsLength, tableId *tables, unsigned int tablesLength) {
    int l = 0;
    for (int j = 0; j < world->tablesLength; j++) {
        if (world->tables[j].recordsLength < componentsLength) continue;
        if (world->tables[j].componentsLength == 0) continue;

        int hits = 0;
        for (int k = 0; k < componentsLength; k++) {
            for (int i = 0; i < world->tables[j].recordsLength; i++) {
                hits += world->tables[j].records[i].componentType == components[k];
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
                hits += world->tables[j].records[i].componentType == components[k];
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
        world->tables[len].records[i].components = malloc(MAX_ENTITY_COUNT * COMPONENT_SIZE_BY_ID(world, components[i]));
    }

    world->tablesLength++;

    return len;
}

static struct entity addEntityToTable(struct world *world, tableId table) {
    struct table *t = &world->tables[table];

    struct entity e = {
        table,
        t->componentsLength
    };

    t->componentsLength++;

    return e;
}

static void removeEntityFromTable(struct world *world, struct entity entity) {
    struct table *t = &world->tables[entity.table];

    for (int j = 0; j < t->recordsLength; j++) {
        unsigned int size = COMPONENT_SIZE_BY_ID(world, t->records[j].componentType);
        for (int i = entity.position; i < t->componentsLength - 1; i++) {
            memcpy(t->records[j].components + (i * size), t->records[j].components + ((i + 1) * size), size);
        }
    }

    t->componentsLength--;
}

// Assumes tableTo exists already
static struct entity copyEntityBetweenTables(struct world *world, struct entity entity, tableId tableTo) {
    tableId tableFrom = entity.table;
    struct table *tFrom = &world->tables[tableFrom];
    struct table *tTo = &world->tables[tableTo];

    unsigned int len = tTo->componentsLength;

    for (int j = 0; j < tFrom->recordsLength; j++) {
        for (int i = 0; i < tTo->recordsLength; i++) {
            if (tFrom->records[j].componentType == tTo->records[i].componentType) {
                unsigned int size = COMPONENT_SIZE_BY_ID(world, tTo->records[i].componentType);
                memcpy(tTo->records[i].components + (entity.position * size), tFrom->records[j].components + (len * size), size);
                break;
            }
        }
    }

    struct entity e = {
        tableTo,
        len
    };

    tTo->componentsLength++;

    return e;
}

void addComponent(struct world *world, entityId entity, componentId component) {
    struct entity *e = hashtableGetById(&world->entities, entity);

    if (e->table == INVALID_POSITION) {
        tableId t = getTableForComponents(world, &component, 1);
        struct entity newEntity = addEntityToTable(world, t);
        hashtableSetById(&world->entities, entity, &newEntity);
    }
    else {
        componentId components[MAX_COMPONENT_COUNT];
        for (int i = 0; i < world->tables[e->table].recordsLength; i++) {
            components[i] = world->tables[e->table].records[i].componentType;
        }
        components[world->tables[e->table].recordsLength] = component;
        tableId t = getTableForComponents(world, components, world->tables[e->table].recordsLength + 1);
        struct entity newEntity = copyEntityBetweenTables(world, *e, t);
        removeEntityFromTable(world, *e);
        hashtableSetById(&world->entities, entity, &newEntity);
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
    struct entity *e = hashtableGetById(&world->entities, entity);

    for (int i = 0; i < world->tables[e->table].recordsLength; i++) {
        if (world->tables[e->table].records[i].componentType == component) {
            return world->tables[e->table].records[i].components + (e->position * COMPONENT_SIZE_BY_ID(world, component));
        }
    }

    return NULL;
}

void *getComponentsFromTable(struct world *world, tableId table, componentId component) {
    struct table t = world->tables[table];

    for (int i = 0; i < t.recordsLength; i++) {
        if (t.records[i].componentType == component) return t.records[i].components;
    }

    return NULL;
}

void removeComponent(struct world *world, entityId entity, componentId component) {
    struct entity *e = hashtableGetById(&world->entities, entity);

    if (e->table == INVALID_POSITION) {
        return;
    }
    else {
        componentId components[MAX_COMPONENT_COUNT];
        for (int i = 0; i < world->tables[e->table].recordsLength; i++) {
            if (world->tables[e->table].records[i].componentType == component) continue;
            components[i] = world->tables[e->table].records[i].componentType;
        }
        tableId t = getTableForComponents(world, components, world->tables[e->table].recordsLength - 1);
        struct entity newEntity = copyEntityBetweenTables(world, *e, t);
        removeEntityFromTable(world, *e);
        hashtableSetById(&world->entities, entity, &newEntity);
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

entityId createEntity(struct world *world) {
    world->counter++;

    char key[500];
    sprintf(key, "%d", world->counter);

    struct entity e = {
        INVALID_POSITION,
        INVALID_POSITION
    };

    return hashtableSet(&world->entities, key, &e);
}

void deleteEntity(struct world *world, entityId id) {
    struct entity *e = hashtableGetById(&world->entities, id);
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

    hashtableRemoveById(&world->entities, id);
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

void runWorldPhase(struct world *world, enum systemPhase phase) {
    for (int i = 0; i < world->systemsLength; i++) {
        if (world->systems[i].phase != phase) continue;

        tableId tables[MAX_ARCHETYPE_COUNT];
        unsigned int tablesLength = getAllTablesWithComponents(world, world->systems[i].components, world->systems[i].componentsLength, tables, MAX_ARCHETYPE_COUNT);

        for (int j = 0; j < tablesLength; j++) {
            struct systemRunData data = {
                world,
                { .table = tables[j] },
                &world->systems[i]
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
            print(" %u", world->systems[i].components[j]);
        }
        print(")\n");
    }
    print("\n");

    print("[ECS Entities]\n");
    struct entity *entities = world->entities.buffer;
    for (int i = 0; i < world->entities.bufferCount; i++) {
        if (!world->entities.valids[i]) continue;

        print("(id: 0x%X) (table: %u) (row: %u)\n", world->entities.hashes[i], entities[i].table, entities[i].position);
    }
    print("\n");

    print("[ECS Components (%d)]\n", world->components.bufferCount);
    struct component *components = world->components.buffer;
    for (int i = 0; i < world->components.bufferCount; i++) {
        if (!world->components.valids[i]) continue;

        print("(name: %s) (hash: 0x%X) (size: %u)\n", components[i].name, world->components.hashes[i], components[i].size);
    }
    print("\n");

    print("[ECS Tables]\n");
    for (int i = 0; i < world->tablesLength; i++) {
        print("(id: %d) (records: %u) (rows: %u)\n", i, world->tables[i].recordsLength, world->tables[i].componentsLength);
        for (int j = 0; j < world->tables[i].recordsLength; j++) {
            print("(component: %u -> %p)\n", world->tables[i].records[j].componentType, world->tables[i].records[j].components);
        }
        print("\n");
    }
}
