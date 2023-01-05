#ifndef STRUCTURE_H
#define STRUCTURE_H

#include <stdint.h>
#include <stdbool.h>

typedef uint32_t hashtableId;

struct hashtable {
    void *buffer;
    uint32_t *hashes;
    bool *valids;

    unsigned int bufferCount;
    unsigned int typeSize;
};

uint32_t hashString(const char *name, uint32_t len);

#define HASHTABLE_CREATE(c, t) hashtableCreate((c), sizeof(t))
struct hashtable hashtableCreate(unsigned int capacity, unsigned int size);
void hashtableDestroy(struct hashtable *hashtable);

hashtableId hashtableSet(struct hashtable *hashtable, const char *key, void *data);
bool hashtableRemove(struct hashtable *hashtable, const char *key);
void *hashtableGet(struct hashtable *hashtable, const char *key);

#endif
