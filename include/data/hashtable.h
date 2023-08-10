#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdint.h>
#include <stdbool.h>

struct hashtable {
    void *buffer;
    char **keys;
    bool *valids;

    unsigned int validCount;
    unsigned int bufferCount;
    unsigned int typeSize;
};

uint32_t hashString(const char *name, uint32_t len);

#define HASHTABLE_CREATE(c, t) hashtableCreate((c), sizeof(t))
struct hashtable hashtableCreate(unsigned int capacity, unsigned int size);
void hashtableDestroy(struct hashtable *hashtable);

void hashtableSet(struct hashtable *hashtable, const char *key, void *data);
bool hashtableRemove(struct hashtable *hashtable, const char *key);
void *hashtableGet(struct hashtable *hashtable, const char *key);

#endif
