#include <stdlib.h>
#include "struct.h"
#include "util.h"

static DictEntry dummie_entry;
static DictEntry *dummie = &dummie_entry;


void opendict_init(
                OpenDict *dict, size_t size,
                hash_t (* hashfunc)(Val), 
                int (* cmpfunc)(Val, Val),
                void (* key_destroy)(Val),
                void (* value_destroy)(void *)
                ) {
        if (size == 0) {
                size = 4096;
        }
        dict->table_size = size;
        dict->table = xcalloc(size, sizeof(DictEntry *));

        dict->hashfunc = hashfunc;
        dict->cmpfunc = cmpfunc;

        dict->key_destroy = key_destroy;
        dict->value_destroy = value_destroy;

        dict->elements = NULL;
        dict->elements_end = NULL;
        dict->entry_count = 0;
}

void opendict_remove(OpenDict *dict, Val key){
        hash_t hash;
        hash_t h;
        hash_t i;
        hash_t j;
        hash_t k;

        hash = dict->hashfunc(key);
        h = hash % dict->table_size;
        dict->entry_count--;

        while (dict->table[h] != NULL) {
                if (dict->table[h] != dummie && dict->table[h]->hash == hash && !dict->cmpfunc(key, dict->table[h]->key)) {
                        break;
                }
                h = (h + 1) % dict->table_size;
                if (h == hash % dict->table_size) {
                        return;
                }
        }
        if (dict->table[h] == NULL) { return; }

        if (dict->table[h]->prev == NULL) {
                dict->elements = dict->table[h]->next;
                if (dict->elements) {
                        dict->elements->prev = NULL;
                }
        } else if (dict->table[h] == dict->elements_end) {
                dict->elements_end = dict->table[h]->prev;
                if (dict->elements_end) {
                        dict->elements_end->next = NULL;
                }
        } else {
                /* easily exploitable pattern, but i couldn't care less */
                dict->table[h]->prev->next = dict->table[h]->next;
                dict->table[h]->next->prev = dict->table[h]->prev;
        }

        if (dict->key_destroy) { dict->key_destroy(dict->table[h]->key); }
        if (dict->value_destroy) { dict->value_destroy(dict->table[h]->value); }

        i = j = h;

        free(dict->table[h]);
        dict->table[h] = dummie;
        /* for(;;) { */
        /*         dict->table[i] = NULL; */
        /*         j = (j + 1) % dict->table_size; */
        /*         if (dict->table[j] == NULL) { */
        /*                 break; */
        /*         } */
        /*         k = dict->table[j]->hash % dict->table_size; */
        /*         if ((i<=j) ? ((i<k)&&(k<=j)) : ((i<k)||(k<=j))) { */
        /*                 continue; */
        /*         } */
        /*         dict->table[i] = dict->table[j]; */
        /*         i = j; */
        /* } */
}


void opendict_rebuild(OpenDict *dict, size_t size) {
        hash_t h;
        DictEntry *ep;

        if (size == 0) { size = 4096 > dict->entry_count ? 4096 : dict->entry_count; }
        if (size < dict->entry_count) {
                die(3, "Attempt to resize dict with %zu entries to size %zu.\n", dict->entry_count, size);
        }
        dict->table = xcalloc(size, sizeof(DictEntry *));
        dict->table_size = size;

        ep = dict->elements;
        while (ep) {
                h = ep->hash % dict->table_size;
                while (dict->table[h] != NULL) {
                        h = (h + 1) % dict->table_size;
                }
                dict->table[h] = ep;

                ep = ep->next;
        }
}


void opendict_set(OpenDict *dict, Val key, void *value){
        hash_t hash;
        hash_t h;

        hash = dict->hashfunc(key);
        h = hash % dict->table_size;
        dict->entry_count++;

        while (dict->table[h] != NULL && dict->table[h] != dummie) {
                if (dict->table[h]->hash == hash && !dict->cmpfunc(key, dict->table[h]->key)) {
                        if (dict->value_destroy) { dict->value_destroy(dict->table[h]->value); }
                        dict->table[h]->value = value;
                        dict->entry_count--;
                        return;
                }
                h = (h + 1) % dict->table_size;
                if (h == hash % dict->table_size) {
                        opendict_rebuild(dict, dict->table_size * 2);
                        opendict_set(dict, key, value);
                        return;
                }
        }

        dict->table[h] = xcalloc(1, sizeof(DictEntry));

        dict->table[h]->key = key;
        dict->table[h]->value = value;
        dict->table[h]->hash = hash;
        dict->table[h]->next = NULL;
        dict->table[h]->prev = dict->elements_end;
        if (dict->elements_end) { dict->elements_end->next = dict->table[h]; }
        dict->elements_end = dict->table[h];
        if (dict->elements == NULL) {
                dict->elements = dict->table[h];
        }

}

void **opendict_get(OpenDict *dict, Val key){
        hash_t hash;
        hash_t h;

        hash = dict->hashfunc(key);
        h = hash % dict->table_size;


        while (dict->table[h] != NULL) {
                if (dict->table[h] != dummie && dict->table[h]->hash == hash && !dict->cmpfunc(key, dict->table[h]->key)) {
                        return &(dict->table[h]->value);
                }
                h = (h + 1) % dict->table_size;
                if (h == hash % dict->table_size) {
                        break;
                }
        }

        return NULL;
}


void opendict_destroy(OpenDict *dict) {
        while (dict->elements != NULL) {
                opendict_remove(dict, dict->elements->key);
        }
        free(dict->table);
}
