#include <stdlib.h>
#include "struct.h"
#include "util.h"

typedef struct robin_hood_entry {
        Val key;
        void *value;
        hash_t hash;
        size_t psi;
        struct robin_hood_entry *next;
        struct robin_hood_entry *prev;
} RobinHoodEntry;

typedef struct dict {
        size_t table_size;
        RobinHoodEntry **table;
        size_t entry_count;

        RobinHoodEntry *elements;
        RobinHoodEntry *elements_end;
        size_t psi_sum;
        size_t element_count;


        hash_t (* hashfunc)(Val);
        int (* cmpfunc)(Val, Val);

        void (* key_destroy)(Val);
        void (* value_destroy)(void *);
} Dict;
void dict_set(Dict *dict, Val key, void *value);

void dict_init(
                Dict *dict, size_t size,
                hash_t (* hashfunc)(Val), 
                int (* cmpfunc)(Val, Val),
                void (* key_destroy)(Val),
                void (* value_destroy)(void *)
                ) {
        if (size == 0) {
                size = 4096;
        }
        dict->psi_sum = 0;
        dict->element_count = 0;

        dict->table_size = size;
        dict->table = xcalloc(size, sizeof(RobinHoodEntry *));

        dict->hashfunc = hashfunc;
        dict->cmpfunc = cmpfunc;

        dict->key_destroy = key_destroy;
        dict->value_destroy = value_destroy;

        dict->elements = NULL;
        dict->elements_end = NULL;
        dict->entry_count = 0;
}

void dict_remove(Dict *dict, Val key){
        hash_t hash;
        hash_t h;
        hash_t i;
        hash_t j;
        hash_t k;

        hash = dict->hashfunc(key);
        h = hash % dict->table_size;
        dict->entry_count--;

        while (dict->table[h] != NULL) {
                if (dict->table[h]->hash == hash && !dict->cmpfunc(key, dict->table[h]->key)) {
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
        } else if (dict->table[h] == dict->elements_end) {
                dict->elements_end = dict->table[h]->prev;
        } else {
                /* easily exploitable pattern, but i couldn't care less */
                dict->table[h]->prev->next = dict->table[h]->next;
                dict->table[h]->next->prev = dict->table[h]->prev;
        }

        if (dict->key_destroy) { dict->key_destroy(dict->table[h]->key); }
        if (dict->value_destroy) { dict->value_destroy(dict->table[h]->value); }

        i = j = h;
        
        for(;;) {
                dict->table[i] = NULL;
                j = (j + 1) % dict->table_size;
                if (dict->table[j] == NULL || dict->table[j]->psi == 0) {
                        break;
                }
                k = dict->table[j]->hash % dict->table_size;
                if ((i<=j) ? ((i<k)&&(k<=j)) : ((i<k)||(k<=j))) {
                        continue;
                }
                dict->table[i] = dict->table[j];
                i = j;
        }
}


void dict_rebuild(Dict *dict, size_t size) {
        hash_t h;
        RobinHoodEntry *ep;
        RobinHoodEntry *tmp;

        if (size == 0) { size = 4096 > dict->entry_count ? 4096 : dict->entry_count; }
        if (size < dict->entry_count) {
                die(3, "Attempt to resize dict with %zu entries to size %zu.", dict->entry_count, size);
        }
        dict->table = xcalloc(size, sizeof(RobinHoodEntry *));
        dict->table_size = size;

        ep = dict->elements;
        dict->elements = NULL;
        dict->elements_end = NULL;
        while (ep) {
                dict_set(dict, ep->key, ep->value);
                tmp = ep;
                ep = ep->next;
                free(tmp);
        }
}


void dict_set(Dict *dict, Val key, void *value){
        hash_t hash;
        hash_t h;
        size_t psi;

        Val tmp_key;
        void *tmp_value;
        hash_t tmp_hash;
        size_t tmp_psi;

        hash = dict->hashfunc(key);
        h = hash % dict->table_size;
        dict->entry_count++;
        psi = 0;

        while (dict->table[h] != NULL) {
                if (dict->table[h]->hash == hash && !dict->cmpfunc(key, dict->table[h]->key)) {
                        if (dict->value_destroy) { dict->value_destroy(dict->table[h]->value); }
                        dict->table[h]->value = value;
                        dict->entry_count--;
                        return;
                }
                h = (h + 1) % dict->table_size;
                psi++;
                if (psi > dict->table[h]->psi) {
                        tmp_key = dict->table[h]->key;
                        tmp_value = dict->table[h]->value;
                        tmp_hash = dict->table[h]->hash;
                        tmp_hash = dict->table[h]->psi;
                        dict->table[h]->key = key;
                        dict->table[h]->value = value;
                        dict->table[h]->hash = hash;
                        key = tmp_key;
                        value = tmp_value;
                        hash = tmp_hash;
                        psi = tmp_psi;

                }
                if (h == hash % dict->table_size) {
                        dict_rebuild(dict, dict->table_size * 2);
                        dict_set(dict, key, value);
                        return;
                }
        }

        dict->table[h] = xcalloc(1, sizeof(RobinHoodEntry));

        dict->table[h]->psi = psi;
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

void **dict_get(Dict *dict, Val key){
        hash_t hash;
        hash_t h;
        size_t i;
        size_t avg_psi;
        RobinHoodEntry *entry;

        hash = dict->hashfunc(key);
        h = hash % dict->table_size;
        avg_psi = dict->psi_sum / (dict->element_count + 1);
        i = 0;



        while (dict->table[(h + avg_psi + i) % dict->table_size] || dict->table[((h + avg_psi - i) % dict->table_size) % dict->table_size]) {
                entry = dict->table[(h + avg_psi + i) % dict->table_size];
                if (entry && entry->hash == hash && !dict->cmpfunc(key, entry->key)) {
                        return &(entry->value);
                }

                entry = dict->table[((h + avg_psi - i) % dict->table_size) % dict->table_size];
                if (entry && entry->hash == hash && !dict->cmpfunc(key, entry->key)) {
                        return &(entry->value);
                }
                i = (i + 1) % dict->table_size;
                if ((i + avg_psi) % dict->table_size == 0) {
                        break;
                }
        }

        return NULL;
}


void dict_destroy(Dict *dict) {
        while (dict->elements != NULL) {
                dict_remove(dict, dict->elements->key);
        }
        free(dict->table);
}
