#include "hashmap.h"

int hash_function(struct HashMap *hashmap, void *key) {
    int running_sum = 0, acc = 1;
    for (size_t i = 0; i < hashmap->key_size; i++) {
        running_sum += acc * (((char *) key)[i]);
        acc++;
    }
    return running_sum % hashmap->max_len;
}

int init_map(struct HashMap *hashmap, size_t key, size_t value, int max_size) {
    hashmap->length = 0;
    hashmap->value_size = value;
    hashmap->key_size = key;
    hashmap->max_len = max_size;
    hashmap->inner_array = malloc(hashmap->max_len * sizeof(struct MapNode *));

    if (hashmap->inner_array == NULL) {
        return -1;
    }

    for (int i = 0; i < hashmap->max_len; i++) {
        hashmap->inner_array[i] = NULL;
    }
    return 0;
}

int add_item(struct HashMap *hashmap, void *key, void *value) {
    int index = hash_function(hashmap, key);
    struct MapNode *node = hashmap->inner_array[index];
    if (node == NULL) {
        hashmap->inner_array[hash_function(hashmap, key)] = malloc(sizeof(struct MapNode));
        node = hashmap->inner_array[hash_function(hashmap, key)];
        node->key = malloc(sizeof(hashmap->key_size));
        memcpy(node->key, key, sizeof(hashmap->key_size));
        node->value = malloc(sizeof(hashmap->value_size));
        memcpy(node->value, value, sizeof(hashmap->value_size));
        node->next = NULL;
        node->previous = NULL;
    } else {
        struct MapNode *cur_node = node;
        while (cur_node != NULL) {
            node = cur_node;
            if (memcmp(cur_node->key, key, hashmap->key_size) == 0) {
                return -1;
            }
            cur_node = cur_node->next;
        }
        cur_node = malloc(sizeof(struct MapNode));
        cur_node->key = malloc(sizeof(hashmap->key_size));
        memcpy(cur_node->key, key, sizeof(hashmap->key_size));
        cur_node->value = malloc(sizeof(hashmap->value_size));
        memcpy(cur_node->value, value, sizeof(hashmap->value_size));
        printf("%c %d\n", *(char *) cur_node->key, *(int *) cur_node->value);
        cur_node->next = NULL;
        cur_node->previous = node;
        node->next = cur_node;
    }
    hashmap->length++;
    return 0;
}

void *remove_item(struct HashMap *hashmap, void *key) {
    int index = hash_function(hashmap, key);
    struct MapNode *node = hashmap->inner_array[index];
    while (node != NULL && memcmp(node->key, key, hashmap->key_size) != 0) {
        node = node->next;
    }
    if (node == NULL) {
        return NULL;
    } else {
        void *to_return = node->value;
        struct MapNode *next = node->next;
        free(node->key);
        if (next != NULL) {
            next->previous = node->previous;
        }
        if (node->previous != NULL) {
            node->previous->next = node->next;
        } else {
            free(hashmap->inner_array[index]);
            hashmap->inner_array[index] = next;
        }
        free(node);
        hashmap->length--;
        return to_return;
    }
}

void *find(struct HashMap *hashmap, void *key) {
    struct MapNode *node = hashmap->inner_array[hash_function(hashmap, key)];
    printf("%c %d\n", *(char *) node->key, *(int *) node->value);
    while (node != NULL && memcmp(node->key, key, hashmap->key_size) != 0) {
        node = node->next;
    }
    if (node == NULL) {
        return NULL;
    } else {
        struct MapNode *to_return = malloc(sizeof(hashmap->value_size));
        memcpy(to_return, node->value, sizeof(hashmap->value_size));
        return to_return;
    }
}

void get_all(struct HashMap *hashMap, void *items) {
    for (int i = 0; i < hashMap->length; i++) {

    }
}

int main(void) {
    struct HashMap *m = malloc(sizeof(struct HashMap));
    free(NULL);
    init_map(m, sizeof(char), sizeof(int), 2);
    char *a = malloc(sizeof(char));
    int *b = malloc(sizeof(int));
    int *res;
    *a = 'a';
    *b = 5;
    add_item(m, a, b);
    add_item(m, a, b);
    add_item(m, a, b);
    *a = 'b';
    *b = 6;
    add_item(m, a, b);
    *a = 'c';
    *b = 7;
    add_item(m, a, b);

    res = find(m, a);
    printf("%d\n", *res);
    free(res);
    res = remove_item(m, a);
    printf("%d\n", *res);
    free(res);

    *a = 'c';
    *b = 7;
    add_item(m, a, b);

    *a = 'a';
    res = find(m, a);
    printf("%d\n", *res);
    free(res);
    res = remove_item(m, a);
    printf("%d\n", *res);
    free(res);

    *a = 'b';
    res = find(m, a);
    printf("%d\n", *res);
    free(res);
    res = remove_item(m, a);
    printf("%d\n", *res);
    free(res);

    *a = 'c';
    res = find(m, a);
    printf("%d\n", *res);
    free(res);
    res = remove_item(m, a);
    printf("%d\n", *res);
    free(res);

    *a = 'c';
    *b = 7;
    add_item(m, a, b);

}



