#ifndef DYNARRAY_H
#define DYNARRAY_H

#include <stdbool.h>

struct dynarray {
    void *buffer;

    unsigned int bufferCount;
    unsigned int capacityCount;
    unsigned int typeSize;
};

#define DYNARRAY_CREATE(c, t) dynarrayCreate((c), sizeof(t))
struct dynarray dynarrayCreate(unsigned int capacity, unsigned int size);
void dynarrayDestroy(struct dynarray *array);

void dynarrayAdd(struct dynarray *array, void *data);
bool dynarraySet(struct dynarray *array, unsigned int index, void *data);
bool dynarrayRemove(struct dynarray *array, unsigned int index);
void dynarrayClear(struct dynarray *array);
void *dynarrayGet(struct dynarray *array, unsigned int index);

#endif
