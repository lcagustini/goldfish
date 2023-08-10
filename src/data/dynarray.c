#include <data/dynarray.h>

#include <string.h>
#include <stdlib.h>

#include <print.h>

struct dynarray dynarrayCreate(unsigned int capacity, unsigned int size) {
    if (capacity == 0) {
        print("[dynarray] Capacity can't be zero, creating with a single slot\n");
        capacity = 1;
    }

    struct dynarray array = {
        malloc(capacity * size),
        0,
        capacity,
        size
    };

    memset(array.buffer, 0, capacity * size);

    return array;
}

void dynarrayDestroy(struct dynarray *array) {
    free(array->buffer);

    memset(array, 0, sizeof(struct dynarray));
}

static void dynarrayResize(struct dynarray *array) {
}

void dynarrayAdd(struct dynarray *array, void *data) {
}

void dynarraySet(struct dynarray *array, unsigned int index, void *data) {
}

bool dynarrayRemove(struct dynarray *array, unsigned int index) {
    return false;
}

void *dynarrayGet(struct dynarray *array, unsigned int index) {
    return NULL;
}
