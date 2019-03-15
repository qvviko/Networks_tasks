#ifndef NETWORKS_TASKS_HASHMAP_H
#define NETWORKS_TASKS_HASHMAP_H

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <unistd.h>

struct MapNode {
    void *key;
    void *value;
    struct MapNode *next;
    struct MapNode *previous;
};
struct HashMap {
    struct MapNode **inner_array;
    int max_len;
    int length;
    size_t key_size;
    size_t value_size;
};

int hash_function(struct HashMap *hashmap, void *key);

int init_map(struct HashMap *hashmap, size_t key, size_t value, int max_size);

int add_item(struct HashMap *hashmap, void *key, void *value);

void *remove_item(struct HashMap *hashmap, void *key);

void *find(struct HashMap *hashmap, void *key);

void get_all(struct HashMap *hashMap, void *items);

#endif //NETWORKS_TASKS_HASHMAP_H
