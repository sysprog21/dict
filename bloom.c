#include "bloom.h"
#include "stdint.h"
#include "stdlib.h"

struct bloom_hash {
    hash_function func;
    struct bloom_hash *next;
};

struct bloom_filter {
    struct bloom_hash *func;
    void *bits;
    size_t size;
};

static unsigned int djb2(const void *_str)
{
    const char *str = _str;
    unsigned int hash = 5381;
    char c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

static unsigned int jenkins(const void *_str)
{
    const char *key = _str;
    unsigned int hash = 0;
    while (*key) {
        hash += *key;
        hash += (hash << 10);
        hash ^= (hash >> 6);
        key++;
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

bloom_t bloom_create(size_t size)
{
    bloom_t res = calloc(1, sizeof(struct bloom_filter));
    res->size = size;
    res->bits = calloc((size + 7) >> 3, 1);

    bloom_add_hash(res, djb2);
    bloom_add_hash(res, jenkins);

    return res;
}

void bloom_free(bloom_t filter)
{
    if (filter) {
        while (filter->func) {
            struct bloom_hash *h = filter->func;
            filter->func = h->next;
            free(h);
        }
        free(filter->bits);
        free(filter);
    }
}

void bloom_add_hash(bloom_t filter, hash_function func)
{
    struct bloom_hash *h = calloc(1, sizeof(struct bloom_hash));
    h->func = func;
    struct bloom_hash *last = filter->func;
    while (last && last->next) {
        last = last->next;
    }
    if (last) {
        last->next = h;
    } else {
        filter->func = h;
    }
}

void bloom_add(bloom_t filter, const void *item)
{
    struct bloom_hash *h = filter->func;
    uint8_t *bits = filter->bits;
    while (h) {
        unsigned int hash = h->func(item);
        hash %= filter->size;
        bits[hash >> 3] |= 0x80 >> (hash & 7);
        h = h->next;
    }
}

bool bloom_test(bloom_t filter, const void *item)
{
    struct bloom_hash *h = filter->func;
    uint8_t *bits = filter->bits;
    while (h) {
        unsigned int hash = h->func(item);
        hash %= filter->size;
        if (!(bits[hash >> 3] & (0x80 >> (hash & 7)))) {
            return false;
        }
        h = h->next;
    }
    return true;
}
