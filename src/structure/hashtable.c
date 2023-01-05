#include <structure/hashtable.h>

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
    struct hashtable table = {
        malloc(capacity * size),
        malloc(capacity * sizeof(uint32_t)),
        malloc(capacity * sizeof(bool)),
        capacity,
        size
    };

    return table;
}

void hashtableDestroy(struct hashtable *hashtable) {
    free(hashtable->buffer);
    free(hashtable->hashes);
    free(hashtable->valids);
}

static void hashtableSetById(struct hashtable *hashtable, uint32_t hash, void *data) {
    unsigned int position = hash % hashtable->bufferCount;

    if (hashtable->valids[position] && hashtable->hashes[position] != hash) {
        struct hashtable oldTable;
        memcpy(&oldTable, hashtable, sizeof(struct hashtable));

        unsigned int newLength = (unsigned int)(1.5f * hashtable->bufferCount);
        print("[hashtable] Collision! Increasing capacity from %d to %d\n", hashtable->bufferCount, newLength);
        hashtable->buffer = malloc(newLength * hashtable->typeSize);
        hashtable->hashes = malloc(newLength * sizeof(uint32_t));
        hashtable->valids = malloc(newLength * sizeof(bool));
        hashtable->bufferCount = newLength;

        memset(hashtable->buffer, 0, hashtable->bufferCount * hashtable->typeSize);
        memset(hashtable->hashes, 0, hashtable->bufferCount * sizeof(uint32_t));
        memset(hashtable->valids, 0, hashtable->bufferCount * sizeof(bool));

        for (int i = 0; i < oldTable.bufferCount; i++) {
            if (oldTable.valids[i]) {
                hashtableSetById(hashtable, oldTable.hashes[i], oldTable.buffer + i * oldTable.typeSize);
            }
        }

        hashtableDestroy(&oldTable);

        hashtableSetById(hashtable, hash, data);
    }
    else {
        hashtable->valids[position] = true;
        hashtable->hashes[position] = hash;
        memcpy(hashtable->buffer + position * hashtable->typeSize, data, hashtable->typeSize);
    }
}

hashtableId hashtableSet(struct hashtable *hashtable, const char *key, void *data) {
    uint32_t hash = hashString(key, strlen(key));

    hashtableSetById(hashtable, hash, data);

    return hash;
}

bool hashtableRemove(struct hashtable *hashtable, const char *key) {
    uint32_t hash = hashString(key, strlen(key));
    unsigned int position = hash % hashtable->bufferCount;

    if (hashtable->valids[position]) {
        hashtable->valids[position] = false;

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

    return hashtable->buffer + position * hashtable->typeSize;
}
