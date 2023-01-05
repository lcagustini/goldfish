#include <ecs/world.h>

#include <print.h>

#include <string.h>
#include <assert.h>
#include <stdlib.h>

#define membersize(type, member) sizeof(((type *)0)->member)

uint32_t hashString(const char *name, uint32_t len) {
    // murmurhash3
    uint32_t c1 = 0xcc9e2d51;
    uint32_t c2 = 0x1b873593;
    uint32_t r1 = 15;
    uint32_t r2 = 13;
    uint32_t m = 5;
    uint32_t n = 0xe6546b64;
    uint32_t h = 0;
    uint32_t k = 0;
    uint8_t *d = (uint8_t *) name; // 32 bit extract from `name'
    const uint32_t *chunks = NULL;
    const uint8_t *tail = NULL; // tail - last 8 bytes
    int i = 0;
    int l = len / 4; // chunk length

    h = 0xb6d99cf8; // seed

    chunks = (const uint32_t *) (d + l * 4); // body
    tail = (const uint8_t *) (d + l * 4); // last 8 byte chunk of `name'

    // for each 4 byte chunk of `name'
    for (i = -l; i != 0; i++) {
        // next 4 byte chunk of `name'
        k = chunks[i];

        // encode next 4 byte chunk of `name'
        k *= c1;
        k = (k << r1) | (k >> (32 - r1));
        k *= c2;

        // append to hash
        h ^= k;
        h = (h << r2) | (h >> (32 - r2));
        h = h * m + n;
    }

    k = 0;

    // remainder
    switch (len & 3) { // `len % 4'
        case 3:
            k ^= (tail[2] << 16);
        case 2:
            k ^= (tail[1] << 8);
        case 1:
            k ^= tail[0];
            k *= c1;
            k = (k << r1) | (k >> (32 - r1));
            k *= c2;
            h ^= k;
    }

    h ^= len;

    h ^= (h >> 16);
    h *= 0x85ebca6b;
    h ^= (h >> 13);
    h *= 0xc2b2ae35;
    h ^= (h >> 16);

    return h;
}

entityId singletonEntity;

struct world createWorld() {
    struct world world = {0};

    world.components = malloc(MAX_COMPONENT_COUNT * sizeof(struct component));
    world.componentsLength = MAX_COMPONENT_COUNT;
    memset(world.components, 0, world.componentsLength * sizeof(struct component));

    singletonEntity = createEntity(&world);

    return world;
}

componentId createComponent(struct world *world, const char *component, unsigned int componentLength, unsigned int componentSize) {
    uint32_t hash = hashString(component, componentLength);
    unsigned int position = hash % world->componentsLength;

    if (world->components[position].valid) {
        struct component *oldBuffer = world->components;
        unsigned int oldLength = world->componentsLength;

        unsigned int newLength = (unsigned int)(1.5f * world->componentsLength);
        print("new length: %u\n", newLength);
        world->components = malloc(newLength * sizeof(struct component));
        world->componentsLength = newLength;
        memset(world->components, 0, world->componentsLength * sizeof(struct component));

        for (int i = 0; i < oldLength; i++) {
            if (oldBuffer[i].valid) {
                createComponent(world, oldBuffer[i].name, strlen(oldBuffer[i].name) + 1, oldBuffer[i].size);
            }
        }

        free(oldBuffer);

        return createComponent(world, component, componentLength, componentSize);
    }

    world->components[position].valid = true;
    world->components[position].hash = hash;
    world->components[position].name = component;
    world->components[position].size = componentSize;

    return hash;
}

componentId getComponentId(const char *component, unsigned int componentLength) {
    return hashString(component, componentLength);
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
        world->tables[len].records[i].components = malloc(MAX_ENTITY_COUNT * COMPONENT_SIZE(world, components[i]));
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

static entityId getIdOfEntity(struct world *world, struct entity entity) {
    for (int i = 0; i < MAX_ENTITY_COUNT; i++) {
        if (world->validEntities[i] && world->entities[i].table == entity.table && world->entities[i].position == entity.position) {
            return i;
        }
    }

    return INVALID_POSITION;
}

static void removeEntityFromTable(struct world *world, struct entity entity) {
    struct table *t = &world->tables[entity.table];

    for (int j = 0; j < t->recordsLength; j++) {
        unsigned int size = COMPONENT_SIZE(world, t->records[j].componentType);
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
                unsigned int size = COMPONENT_SIZE(world, tTo->records[i].componentType);
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
    if (world->entities[entity].table == INVALID_POSITION) {
        tableId t = getTableForComponents(world, &component, 1);
        world->entities[entity] = addEntityToTable(world, t);
    }
    else {
        componentId components[MAX_COMPONENT_COUNT];
        for (int i = 0; i < world->tables[world->entities[entity].table].recordsLength; i++) {
            components[i] = world->tables[world->entities[entity].table].records[i].componentType;
        }
        components[world->tables[world->entities[entity].table].recordsLength] = component;
        tableId t = getTableForComponents(world, components, world->tables[world->entities[entity].table].recordsLength + 1);
        struct entity e = copyEntityBetweenTables(world, world->entities[entity], t);
        removeEntityFromTable(world, world->entities[entity]);
        world->entities[entity] = e;
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
    struct entity e = world->entities[entity];

    for (int i = 0; i < world->tables[e.table].recordsLength; i++) {
        if (world->tables[e.table].records[i].componentType == component) {
            return world->tables[e.table].records[i].components + (e.position * COMPONENT_SIZE(world, component));
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
    if (world->entities[entity].table == INVALID_POSITION) {
        return;
    }
    else {
        componentId components[MAX_COMPONENT_COUNT];
        for (int i = 0; i < world->tables[world->entities[entity].table].recordsLength; i++) {
            if (world->tables[world->entities[entity].table].records[i].componentType == component) continue;
            components[i] = world->tables[world->entities[entity].table].records[i].componentType;
        }
        tableId t = getTableForComponents(world, components, world->tables[world->entities[entity].table].recordsLength - 1);
        struct entity e = copyEntityBetweenTables(world, world->entities[entity], t);
        removeEntityFromTable(world, world->entities[entity]);
        world->entities[entity] = e;
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
    addComponent(world, singletonEntity, component);
}

void *getSingletonComponent(struct world *world, componentId component) {
    return getComponent(world, singletonEntity, component);
}

void removeSingletonComponent(struct world *world, componentId component) {
    removeComponent(world, singletonEntity, component);
}

entityId createEntity(struct world *world) {
    for (int i = 0; i < MAX_ENTITY_COUNT; i++) {
        if (!world->validEntities[i]) {
            world->entities[i].table = INVALID_POSITION;
            world->entities[i].position = INVALID_POSITION;
            world->validEntities[i] = true;

            return i;
        }
    }

    return INVALID_POSITION;
}

void deleteEntity(struct world *world, entityId id) {
    struct table *t = &world->tables[world->entities[id].table];

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

    world->entities[id].table = INVALID_POSITION;
    world->entities[id].position = INVALID_POSITION;
    world->validEntities[id] = false;
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
    for (int i = 0; i < MAX_ENTITY_COUNT; i++) {
        if (!world->validEntities[i]) continue;

        print("(id: %d) (table: %u) (row: %u)\n", i, world->entities[i].table, world->entities[i].position);
    }
    print("\n");

    print("[ECS Components (%d)]\n", world->componentsLength);
    for (int i = 0; i < world->componentsLength; i++) {
        if (!world->components[i].valid) continue;

        print("(name: %s) (hash: %u) (size: %u)\n", world->components[i].name, world->components[i].hash, world->components[i].size);
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
