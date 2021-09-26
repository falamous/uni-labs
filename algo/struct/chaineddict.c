#include <stdlib.h>
#include "struct.h"
#include "util.h"

void chaineddict_init(
                ChainedDict *dict, size_t size,
                hash_t (* hashfunc)(Val), 
                int (* cmpfunc)(Val, Val),
                void (* key_destroy)(Val),
                void (* value_destroy)(void *)
                ) {
        if (size == 0) {
                size = 4096;
        }
        dict->table_size = size;
        dict->table = xcalloc(size, sizeof(ChainedDictLink *));

        dict->hashfunc = hashfunc;
        dict->cmpfunc = cmpfunc;

        dict->key_destroy = key_destroy;
        dict->value_destroy = value_destroy;

        dict->elements = NULL;
        dict->elements_end = NULL;
}

void chaineddict_remove(ChainedDict *dict, Val key){
        hash_t hash;
        ChainedDictLink **cp;
        ChainedDictLink *tmp;

        hash = dict->hashfunc(key);

        cp = &dict->table[hash % dict->table_size];

        if (*cp == NULL){ return; }

        while (*cp != NULL){
                if ((*cp)->entry->hash == hash && !dict->cmpfunc(key, (*cp)->entry->key)) {
                        if ((*cp)->entry->prev == NULL) {
                                dict->elements = (*cp)->entry->next;
                                if (dict->elements) {
                                        dict->elements->prev = NULL;
                                }
                        } else if ((*cp)->entry == dict->elements_end) {
                                dict->elements_end = (*cp)->entry->prev;
                                if (dict->elements_end) {
                                        dict->elements_end->next = NULL;
                                }
                        } else {
                                /* easily exploitable pattern, but i couldn't care less */
                                (*cp)->entry->prev->next = (*cp)->entry->next;
                                (*cp)->entry->next->prev = (*cp)->entry->prev;
                        }

                        if (dict->key_destroy) { dict->key_destroy((*cp)->entry->key); }
                        if (dict->value_destroy) { dict->value_destroy((*cp)->entry->value); }
                        tmp = *cp;
                        *cp = (*cp)->next;
                        free(tmp->entry);
                        free(tmp);

                        return;
                }
                cp = &((*cp)->next);
        }
}

/* static void hashchain_swap(struct hashchain_link *node1, struct hashchain_link *node2){ */
/*         void *tkey = node1->key; */
/*         void *tvalue = node1->value; */
/*         hash_t thash = node1->hash; */

/*         node1->key = node2->key; */
/*         node1->value = node2->value; */
/*         node1->hash = node2->hash; */

/*         node2->key = tkey; */
/*         node2->value = tvalue; */
/*         node2->hash = thash; */
/* } */

void chaineddict_set(ChainedDict *dict, Val key, void *value){
        hash_t hash;
        ChainedDictLink **cp;

        hash = dict->hashfunc(key);

        cp = &dict->table[hash % dict->table_size];

        if (*cp != NULL){
                while (*cp != NULL){
                        if ((*cp)->entry->hash == hash && !dict->cmpfunc(key, (*cp)->entry->key)) {
                                if (dict->value_destroy) { dict->value_destroy((*cp)->entry->value); }
                                (*cp)->entry->value = value;
                                /* hashchain_swap(*cp, hashmap->table_entries[hash % hashmap->table_size].chain); */
                                return;
                        }
                        cp = &((*cp)->next);
                }
        }

        *cp = xcalloc(1, sizeof(ChainedDictLink));
        (*cp)->entry = xcalloc(1, sizeof(DictEntry));

        (*cp)->entry->key = key;
        (*cp)->entry->value = value;
        (*cp)->entry->hash = hash;
        (*cp)->entry->next = NULL;
        (*cp)->entry->prev = dict->elements_end;
        if (dict->elements_end) { dict->elements_end->next = (*cp)->entry; }
        dict->elements_end = (*cp)->entry;
        if (dict->elements == NULL) {
                dict->elements = (*cp)->entry;
        }

        /* hashchain_swap(*cp, hashmap->table_entries[hash % hashmap->table_size].chain); */
}

void **chaineddict_get(ChainedDict *dict, Val key){
        hash_t hash;
        ChainedDictLink **cp;

        hash = dict->hashfunc(key);

        cp = &dict->table[hash % dict->table_size];

        while (*cp != NULL){
                if ((*cp)->entry->hash == hash && !dict->cmpfunc(key, (*cp)->entry->key)) {
                        return &((*cp)->entry->value);
                }
                cp = &((*cp)->next);
        }

        return NULL;
}


void chaineddict_destroy(ChainedDict *dict) {
        while (dict->elements != NULL) {
                chaineddict_remove(dict, dict->elements->key);
        }
        free(dict->table);
}
