#include <ecs/world.h>

#include <print.h>

#include <string.h>
#include <assert.h>
#include <stdlib.h>

#define membersize(type, member) sizeof(((type *)0)->member)

uint32_t hash(const char *name, uint32_t len) {
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
        case 3: k ^= (tail[2] << 16);
        case 2: k ^= (tail[1] << 8);
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

#define COMPONENT_NAME(c) #c

#define CREATE_COMPONENT(w, c) createComponent(w, COMPONENT_NAME(c), sizeof(c))
componentId createComponent(struct world *world, const char *component, unsigned int componentSize) {
    for (int i = 0; i < MAX_COMPONENT_COUNT; i++) {
        if (!world->validComponents[i]) {
            world->components[i].hash = hash(component, strlen(component));
            world->components[i].name = component;
            world->components[i].size = componentSize;
            world->validComponents[i] = true;

            return i;
        }
    }

    return INVALID_POSITION;
}

#define COMPONENT_SIZE(w, c) (w->components[c].size)

static unsigned int getTableForComponents(struct world *world, componentId *components, unsigned int componentsLength) {
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

    unsigned int len = world->tablesLength;
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

static struct entity addEntityToTable(struct world *world, unsigned int table) {
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
        unsigned int size = COMPONENT_SIZE(world, t->records[j].componentType);
        for (int i = entity.position; i < t->componentsLength - 1; i++) {
            memcpy(t->records[j].components + (i * size), t->records[j].components + ((i + 1) * size), size);
        }
    }

    t->componentsLength--;
}

static void copyEntityBetweenTables(struct world *world, struct entity entity, unsigned int tableFrom, unsigned int tableTo) {
    struct table *tFrom = &world->tables[tableFrom];
    struct table *tTo = &world->tables[tableTo];
}

void addComponent(struct world *world, entityId entity, componentId component) {
    if (world->entities[entity].table == INVALID_POSITION) {
        unsigned int t = getTableForComponents(world, &component, 1);
        world->entities[entity] = addEntityToTable(world, t);
    }
    else {
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

void removeComponent(struct world *world, entityId entityId, componentId component) {
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
        if (world->systems[i].phase == phase) {
            world->systems[i].callback(world, 0);
        }
    }
}
