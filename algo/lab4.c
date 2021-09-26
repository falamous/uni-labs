#include <stdio.h>
#include <stdbool.h>
#include "struct.h"
#include "util.h"


struct cache_entry {
        int key;
        char *value;
        bool occupied;
};

typedef struct cache {
        struct cache_entry *cache;
        size_t size;
} Cache;

void cache_init(Cache *cache, size_t size) {
        if (size == 0) {
                size = 4096;
        }
        cache->size = size;
        cache->cache = xcalloc(size, sizeof(struct cache_entry));
}

void cache_set(Cache *cache, int key, char *value) {
        hash_t hash;
        hash = key % cache->size;

        cache->cache[hash].occupied = true;
        cache->cache[hash].key = key;
        cache->cache[hash].value = value;
}

char **cache_get(Cache *cache, int key) {
        hash_t hash;
        hash = key % cache->size;
        if (cache->cache[hash].occupied && key == cache->cache[hash].key) {
                return &(cache->cache[hash].value);
        }
        return NULL;
}

void cache_remove(Cache *cache, int key) {
        hash_t hash;
        hash = key % cache->size;
        if (cache->cache[hash].occupied && key == cache->cache[hash].key) {
                cache->cache[hash].occupied = false;
        }
}

void cache_destroy(Cache *cache) {
        free(cache->cache);
}

void menu() {
        puts("1. Get element by key.");
        puts("2. Set element by key.");
        puts("3. Delete element.");
        puts("4. Max element.");
        puts("5. Sorted table.");
        puts("6. Load from file.");
        puts("7. Exit.");
        puts("8. Print tree.");
        printf("cmd: ");
}

void print_inorder_traversal(RBTree *tree) {
        RBTreeNode *node;
        node = rbtree_min(tree);
        puts("{");
        while (node) {
                printf("%lu=%s\n", node->key.i, node->value);
                node = rbtree_next(node);
        }
        puts("}");
}

char *table_get(RBTree *tree, Cache *cache, int key) {
        char **res;
        res = cache_get(cache, key) ?: (char**)rbtree_get(tree, (Val){.i = key});
        if (res) {
                return *res;
        }
        return NULL;
}

void table_set(RBTree *tree, Cache *cache, int key, char *value) {

        if (table_get(tree, cache, key) != NULL) {
                eprintf("Key is already in table.\n");
                return;
        }

        cache_set(cache, key, value);
        rbtree_set(tree, (Val){.i = key}, value);
}

char *table_remove(RBTree *tree, Cache *cache, int key) {
        cache_remove(cache, key);
        /* order here is critical because rbtree actually frees */
        rbtree_remove(tree, (Val){.i = key});
}

void table_from_file(RBTree *tree, Cache *cache, FILE *stream, char *filename) {
        int res;
        int key;
        char *s;
        char tmp[4];
        int linenum;


        linenum = 0;
        while((res = fscanf(stream, "%i", &key)) != EOF) {
                linenum++;
                if (res != 1 || fscanf(stream, "%1[=]", tmp) != 1) {
                        eprintf("Error reading from file %s on line %i.\n", filename, linenum);
                        fscanf(stream, "%*[^\n]");
                        fscanf(stream, "%*1[\n]");
                }
                s = inputline(stream);
                cache_set(cache, key, s);
                rbtree_set(tree, (Val){.i = key}, s);
        }
}

size_t print_node(FILE *stream, struct rbtree_node *node) {
    size_t res;
    if (node->color == RBTREE_RED) {
        fprintf(stream, "\e[31m");
    }
    res = fprintf(stream, "%i", node->key.i);
    if (node->color == RBTREE_RED) {
        fprintf(stream, "\e[0m");
    }
    return res;
}

int main(int argc, char *argv[]) {
        Cache cache;
        RBTree tree;
        int choice;
        int key;
        char *s;
        int i;
        RBTreeNode *maxnode;
        setvbuf(stdin, NULL, _IONBF, 0);
        setvbuf(stdout, NULL, _IONBF, 0);

        FILE *file;

        cache_init(&cache, 0);
        rbtree_init(&tree, valcmp, NULL, free);

        for(i = 1; i < argc; i++) {
                file = fopen(argv[i], "r");
                if (file) {
                        table_from_file(&tree, &cache, file, argv[i]);
                        fclose(file);
                } else {
                        eprintf("Could not open file %s.\n", argv[i]);
                }
        }

        while (true) {
                menu();
                choice = -1;
                choice = input_int();
                switch(choice) {
                        case 1:
                                printf("key: ");
                                key = input_int();
                                s = table_get(&tree, &cache, key);
                                if (s == NULL) {
                                        puts("No such key");
                                } else {
                                        printf("string: %s\n", s);
                                }
                                break;
                        case 2:
                                printf("key: ");
                                key = input_int();
                                scanf("%*[^\n]");
                                scanf("%*1[\n]");
                                printf("string: ");
                                s = inputline(stdin);
                                table_set(&tree, &cache, key, s);
                                break;
                        case 3:
                                printf("key: ");
                                key = input_int();
                                table_remove(&tree, &cache, key);
                                break;
                        case 4:
                                maxnode = rbtree_max(&tree);
                                if (maxnode == NULL) {
                                        puts("Empty &tree");
                                } else {
                                        printf("Key: %lu\n", maxnode->key.i);
                                        printf("string: %s\n", maxnode->value);
                                }
                                break;
                        case 5:
                                print_inorder_traversal(&tree);
                                break;
                        case 6:
                                printf("filename: ");
                                s = inputline(stdin);
                                file = fopen(s, "r");
                                if (file) {
                                        table_from_file(&tree, &cache, file, s);
                                        fclose(file);
                                } else {
                                        eprintf("Could not open file %s.\n", s);
                                }
                                break;
                        case 7:
                                cache_destroy(&cache);
                                rbtree_destroy(&tree);
                                exit(0);
                        case 8:
                                rbtree_print(&tree, stdout, print_node);
                                break;
                        default:
                                eprintf("Unknown command %i.", choice);
                }
        }
}
