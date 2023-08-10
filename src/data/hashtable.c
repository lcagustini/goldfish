#include <data/hashtable.h>

#include <string.h>
#include <stdlib.h>

#include <print.h>

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

struct hashtable hashtableCreate(unsigned int capacity, unsigned int size) {
    if (capacity == 0) {
        print("[hashtable] Capacity can't be zero, creating with a single slot\n");
        capacity = 1;
    }

    struct hashtable table = {
        malloc(capacity * size),
        malloc(capacity * sizeof(char *)),
        malloc(capacity * sizeof(bool)),
        0,
        capacity,
        size
    };

    memset(table.buffer, 0, capacity * size);
    memset(table.keys, 0, capacity * sizeof(char *));
    memset(table.valids, 0, capacity * sizeof(bool));

    return table;
}

void hashtableDestroy(struct hashtable *hashtable) {
    free(hashtable->buffer);

    for (int i = 0; i < hashtable->bufferCount; i++) {
        if (hashtable->keys[i] != NULL) free(hashtable->keys[i]);
    }
    free(hashtable->keys);

    free(hashtable->valids);
}

static void hashtableResize(struct hashtable *hashtable) {
    struct hashtable oldTable;
    memcpy(&oldTable, hashtable, sizeof(struct hashtable));

    unsigned int newLength = (unsigned int)(1.5f * hashtable->bufferCount + 0.5f);
    print("[hashtable] Increasing capacity from %d to %d\n", hashtable->bufferCount, newLength);
    hashtable->buffer = malloc(newLength * hashtable->typeSize);
    hashtable->keys = malloc(newLength * sizeof(char *));
    hashtable->valids = malloc(newLength * sizeof(bool));
    hashtable->validCount = 0;
    hashtable->bufferCount = newLength;

    memset(hashtable->buffer, 0, hashtable->bufferCount * hashtable->typeSize);
    memset(hashtable->keys, 0, hashtable->bufferCount * sizeof(char *));
    memset(hashtable->valids, 0, hashtable->bufferCount * sizeof(bool));

    for (int i = 0; i < oldTable.bufferCount; i++) {
        if (oldTable.valids[i]) {
            hashtableSet(hashtable, oldTable.keys[i], oldTable.buffer + i * oldTable.typeSize);
        }
    }

    hashtableDestroy(&oldTable);
}

void hashtableSet(struct hashtable *hashtable, const char *key, void *data) {
    uint32_t hash = hashString(key, strlen(key));
    unsigned int position = hash % hashtable->bufferCount;

    while (hashtable->valids[position] && strcmp(hashtable->keys[position], key) != 0) {
        position++;
        if (position == hashtable->bufferCount) position = 0;
    }

    hashtable->valids[position] = true;
    hashtable->validCount++;

    hashtable->keys[position] = malloc((strlen(key) + 1) * sizeof(char));
    strcpy(hashtable->keys[position], key);
    memcpy(hashtable->buffer + position * hashtable->typeSize, data, hashtable->typeSize);

    if (((float)hashtable->validCount / hashtable->bufferCount) > 0.7f) {
        hashtableResize(hashtable);
    }
}

bool hashtableRemove(struct hashtable *hashtable, const char *key) {
    uint32_t hash = hashString(key, strlen(key));
    unsigned int position = hash % hashtable->bufferCount;

    if (hashtable->valids[position]) {
        hashtable->valids[position] = false;
        hashtable->validCount--;

        free(hashtable->keys[position]);
        hashtable->keys[position] = NULL;

        return true;
    }
    else {
        return false;
    }
}

void *hashtableGet(struct hashtable *hashtable, const char *key) {
    uint32_t hash = hashString(key, strlen(key));
    unsigned int position = hash % hashtable->bufferCount;

    if (!hashtable->valids[position]) return NULL;

    while (strcmp(hashtable->keys[position], key) != 0) {
        position++;
        if (position == hashtable->bufferCount) position = 0;
    }

    return hashtable->buffer + position * hashtable->typeSize;
}
