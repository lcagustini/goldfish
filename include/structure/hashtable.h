#ifndef STRUCTURE_H
#define STRUCTURE_H

#include <macros.h>

typedef uint32_t hashtableId;

struct hashtable {
    void *buffer;
    bool *valid;

    unsigned int bufferCount;
    unsigned int typeSize;
};

#define HASHTABLE_CREATE(c, t) hashtableCreate((c), sizeof(t))
struct hashtable hashtableCreate(unsigned int capacity, unsigned int size);

void hashtableDestroy(struct hashtable *hashtable);

#define HASHTABLE_ADD(t, k, d) hashtableAdd((t), STRINGIFY(k), (d))
hashtableId hashtableAdd(struct hashtable *hashtable, const char *key, void *data);

#define HASHTABLE_REMOVE(t, k) hashtableRemove((t), STRINGIFY(k))
bool hashtableRemove(struct hashtable *hashtable, const char *key);

#define HASHTABLE_GET(t, k) hashtableGet((t), STRINGIFY(k))
void *hashtableGet(struct hashtable *hashtable, const char *key);

#endif
