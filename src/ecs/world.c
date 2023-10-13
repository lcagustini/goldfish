#include <ecs/world.h>

#include <print.h>

#include <string.h>
#include <assert.h>
#include <stdlib.h>

struct world createWorld() {
    struct world world = {0};

    world.tables = DYNARRAY_CREATE(MAX_ARCHETYPE_COUNT, struct table);

    world.filters = HASHTABLE_CREATE(MAX_SYSTEM_COUNT, struct filter);
    world.components = HASHTABLE_CREATE(MAX_COMPONENT_COUNT, struct component);
    world.entities = HASHTABLE_CREATE(MAX_ENTITY_COUNT, struct entity);

    world.singletonEntity = createEntity(&world, "Singleton");

    for (int i = 0; i < SYSTEM_PHASE_MAX; i++) world.phaseSystems[i] = DYNARRAY_CREATE(MAX_SYSTEM_COUNT, struct system);
    for (int i = 0; i < SYSTEM_EVENT_MAX; i++) world.eventSystems[i] = DYNARRAY_CREATE(MAX_SYSTEM_COUNT, struct system);

    return world;
}

void destroyWorld(struct world *world) {
    for (int i = 0; i < world->tables.bufferCount; i++) {
        struct table *table = dynarrayGet(&world->tables, i);

        for (int j = 0; j < table->recordsLength; j++) {
            free(table->records[j].components);
        }
        free(table->records);
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
    for (int j = 0; j < world->tables.bufferCount; j++) {
        struct table *table = dynarrayGet(&world->tables, j);

        if (table->recordsLength < componentsLength) continue;
        if (table->componentsLength == 0) continue;

        int hits = 0;
        for (int k = 0; k < componentsLength; k++) {
            for (int i = 0; i < table->recordsLength; i++) {
                hits += strcmp(table->records[i].componentType, components[k]) == 0;
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
    for (int j = 0; j < world->tables.bufferCount; j++) {
        struct table *table = dynarrayGet(&world->tables, j);

        if (table->recordsLength != componentsLength) continue;

        int hits = 0;
        for (int k = 0; k < componentsLength; k++) {
            for (int i = 0; i < componentsLength; i++) {
                hits += strcmp(table->records[i].componentType, components[k]) == 0;
            }
        }
        if (hits == componentsLength) return j;
    }

    struct table newTable = {0};
    newTable.records = malloc(componentsLength * sizeof(struct record));
    newTable.recordsLength = componentsLength;
    newTable.componentsLength = 0;

    for (int i = 0; i < componentsLength; i++) {
        newTable.records[i].componentType = components[i];
        newTable.records[i].components = malloc(MAX_ENTITY_COUNT * COMPONENT_SIZE(world, components[i]));
    }

    dynarrayAdd(&world->tables, &newTable);

    return world->tables.bufferCount - 1;
}

static struct entity addEntityToTable(struct world *world, tableId table, struct entity *entity) {
    struct table *t = dynarrayGet(&world->tables, table);

    struct entity e = {
        entity->name,
        table,
        t->componentsLength
    };

    t->componentsLength++;

    return e;
}

static void removeEntityFromTable(struct world *world, struct entity *entity) {
    struct table *t = dynarrayGet(&world->tables, entity->table);

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
    struct table *tFrom = dynarrayGet(&world->tables, tableFrom);
    struct table *tTo = dynarrayGet(&world->tables, tableTo);

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

static void updateFilter(struct world *world, filterId filterToUpdate) {
    struct filter *filter = hashtableGet(&world->filters, filterToUpdate);
	filter->resultsLength = getAllTablesWithComponents(world, filter->components, filter->componentsLength, filter->results, MAX_ARCHETYPE_COUNT);
}

void addComponent(struct world *world, entityId entity, componentId component) {
    struct entity *e = hashtableGet(&world->entities, entity);

    if (e->table == INVALID_POSITION) {
        tableId t = getTableForComponents(world, &component, 1);
        struct entity newEntity = addEntityToTable(world, t, e);
        hashtableSet(&world->entities, entity, &newEntity);
    }
    else {
        struct table *table = dynarrayGet(&world->tables, e->table);
        componentId components[MAX_COMPONENT_COUNT];
        for (int i = 0; i < table->recordsLength; i++) {
            components[i] = table->records[i].componentType;
        }
        components[table->recordsLength] = component;
        tableId t = getTableForComponents(world, components, table->recordsLength + 1);
        struct entity newEntity = copyEntityBetweenTables(world, e, t);
        removeEntityFromTable(world, e);
        hashtableSet(&world->entities, entity, &newEntity);
    }

    // Update phase systems filters
    for (int j = 0; j < SYSTEM_PHASE_MAX; j++) {
		for (int i = 0; i < world->phaseSystems[j].bufferCount; i++) {
			struct system *system = dynarrayGet(&world->phaseSystems[j], i);

            for (int k = 0; k < system->filtersLength; k++) {
				updateFilter(world, system->filters[k]);
            }
		}
    }

    // Check if any event system is expecting this component
	for (int i = 0; i < world->eventSystems[SYSTEM_ON_COMPONENT_ADD].bufferCount; i++) {
		struct system *system = dynarrayGet(&world->eventSystems[SYSTEM_ON_COMPONENT_ADD], i);
		struct filter *filter = hashtableGet(&world->filters, system->filters[0]);

		if (filter->components[0] == component) {
			struct systemRunData data = {
				world,
				{ { .entity = entity } },
                1,
				system,
				0
			};

			system->callback(data);
        }
	}
}

void *getComponent(struct world *world, entityId entity, componentId component) {
    if (entity == NULL) return NULL;

    struct entity *e = hashtableGet(&world->entities, entity);
    struct table *table = dynarrayGet(&world->tables, e->table);

    for (int i = 0; i < table->recordsLength; i++) {
        if (strcmp(table->records[i].componentType, component) == 0) {
            return table->records[i].components + (e->position * COMPONENT_SIZE(world, component));
        }
    }

    return NULL;
}

void *getComponentsFromTable(struct world *world, tableId table, componentId component) {
    struct table *t = dynarrayGet(&world->tables, table);

    for (int i = 0; i < t->recordsLength; i++) {
        if (strcmp(t->records[i].componentType, component) == 0) return t->records[i].components;
    }

    return NULL;
}

void removeComponent(struct world *world, entityId entity, componentId component) {
    struct entity *e = hashtableGet(&world->entities, entity);

    if (e->table == INVALID_POSITION) {
        return;
    }
    else {
        struct table *table = dynarrayGet(&world->tables, e->table);
        componentId components[MAX_COMPONENT_COUNT];
        for (int i = 0; i < table->recordsLength; i++) {
            if (strcmp(table->records[i].componentType, component) == 0) continue;
            components[i] = table->records[i].componentType;
        }
        tableId t = getTableForComponents(world, components, table->recordsLength - 1);
        struct entity newEntity = copyEntityBetweenTables(world, e, t);
        removeEntityFromTable(world, e);
        hashtableSet(&world->entities, entity, &newEntity);
    }

    // Update phase systems filters
    for (int j = 0; j < SYSTEM_PHASE_MAX; j++) {
		for (int i = 0; i < world->phaseSystems[j].bufferCount; i++) {
			struct system *system = dynarrayGet(&world->phaseSystems[j], i);

            for (int k = 0; k < system->filtersLength; k++) {
				updateFilter(world, system->filters[k]);
            }
		}
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

    free(e->name);
    hashtableRemove(&world->entities, id);
}

void addPhaseSystem(struct world *world, enum systemPhase phase, struct system system) {
    dynarrayAdd(&world->phaseSystems[phase], &system);
	for (int k = 0; k < system.filtersLength; k++) {
		updateFilter(world, system.filters[k]);
	}
}

void addEventSystem(struct world *world, enum systemEvent event, struct system system) {
    dynarrayAdd(&world->eventSystems[event], &system);
	for (int k = 0; k < system.filtersLength; k++) {
		updateFilter(world, system.filters[k]);
	}
}

void addFilter(struct world *world, const char *name, struct filter filter) {
    hashtableSet(&world->filters, name, &filter);
}

void runWorldPhase(struct world *world, enum systemPhase phase, float dt) {
    for (int i = 0; i < world->phaseSystems[phase].bufferCount; i++) {
        struct system *system = dynarrayGet(&world->phaseSystems[phase], i);

        int runTimes = 1;
        for (int k = 0; k < system->filtersLength; k++) {
			struct filter *filter = hashtableGet(&world->filters, system->filters[k]);
            runTimes *= filter->resultsLength;
        }

        if (runTimes == 0) continue;

		struct systemRunData *runDatas = malloc(runTimes * sizeof(struct systemRunData));

		int incrementAt = runTimes;
		for (int j = 0; j < system->filtersLength; j++) {
			struct filter *filter = hashtableGet(&world->filters, system->filters[j]);

			incrementAt /= filter->resultsLength;

			int l = -1;
			for (int k = 0; k < runTimes; k++) {
				if (k % incrementAt == 0) l++;
				runDatas[k].filterResults[j] = (union genericId) { .table = filter->results[l] };
			}
		}

		for (int k = 0; k < runTimes; k++) {
			runDatas[k].world = world;
			runDatas[k].filterResultsLength = system->filtersLength;
			runDatas[k].system = system;
			runDatas[k].dt = dt;

			system->callback(runDatas[k]);
		}

        free(runDatas);
    }
}

void printWorld(struct world *world) {
    print("[ECS World]\n");

    print("[ECS Systems]\n");
    for (int j = 0; j < SYSTEM_PHASE_MAX; j++) {
		for (int i = 0; i < world->phaseSystems[j].bufferCount; i++) {
			struct system *system = dynarrayGet(&world->phaseSystems[j], i);

			print("(name: %s) (phase: %d) ", system->name, j);

            for (int l = 0; l < system->filtersLength; l++) {
				struct filter *filter = hashtableGet(&world->filters, system->filters[l]);

				print("(components [%d]:", l);
				for (int k = 0; k < filter->componentsLength; k++) {
					print(" %s,", filter->components[k]);
				}
				print("\b)");
            }
			print("\n");
		}
    }
    print("\n");

    print("[ECS Entities (%d)]\n", world->entities.validCount);
    struct entity *entities = world->entities.buffer;
    for (int i = 0; i < world->entities.bufferCount; i++) {
        if (!world->entities.valids[i]) continue;

        print("(name: %s) (table: %u) (row: %u)\n", entities[i].name, entities[i].table, entities[i].position);
    }
    print("\n");

    print("[ECS Components (%d)]\n", world->components.validCount);
    struct component *components = world->components.buffer;
    for (int i = 0; i < world->components.bufferCount; i++) {
        if (!world->components.valids[i]) continue;

        print("(name: %s) (key: %s) (size: %u)\n", components[i].name, world->components.keys[i], components[i].size);
    }
    print("\n");

    print("[ECS Tables]\n");
    for (int i = 0; i < world->tables.bufferCount; i++) {
        struct table *table = dynarrayGet(&world->tables, i);
        print("(id: %d) (records: %u) (rows: %u)\n", i, table->recordsLength, table->componentsLength);
        for (int j = 0; j < table->recordsLength; j++) {
            print("(component: %s -> 0x%p)\n", table->records[j].componentType, table->records[j].components);
        }
        print("\n");
    }
}
