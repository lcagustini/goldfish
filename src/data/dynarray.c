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
    unsigned int newCapacity = (unsigned int)(1.5f * array->capacityCount + 0.5f);
    void *newBuffer = malloc(newCapacity * array->typeSize);

    memcpy(newBuffer, array->buffer, array->bufferCount * array->typeSize);
    free(array->buffer);

    array->capacityCount = newCapacity;
    array->buffer = newBuffer;
}

void dynarrayAdd(struct dynarray *array, void *data) {
    if (array->bufferCount >= array->capacityCount) dynarrayResize(array);

    memcpy(array->buffer + array->bufferCount * array->typeSize, data, array->typeSize);
    array->bufferCount++;
}

bool dynarraySet(struct dynarray *array, unsigned int index, void *data) {
    if (index >= array->bufferCount) {
        print("[dynarray] Index out of range! (%d >= %d)", index, array->bufferCount);
        return false;
    }

    memcpy(array->buffer + index * array->typeSize, data, array->typeSize);

    return true;
}

bool dynarrayRemove(struct dynarray *array, unsigned int index) {
    if (index >= array->bufferCount) {
        print("[dynarray] Index out of range! (%d >= %d)", index, array->bufferCount);
        return false;
    }

    memcpy(array->buffer + index * array->typeSize, array->buffer + (index + 1) * array->typeSize, (array->bufferCount - index + 1) * array->typeSize);

    array->bufferCount--;

    return true;
}

void dynarrayClear(struct dynarray *array) {
    array->bufferCount = 0;
}

void *dynarrayGet(struct dynarray *array, unsigned int index) {
    if (index >= array->bufferCount) {
        print("[dynarray] Index out of range! (%d >= %d)", index, array->bufferCount);
        return NULL;
    }

    return array->buffer + index * array->typeSize;
}

unsigned int dynarrayCount(struct dynarray *array) {
    return array->bufferCount;
}
