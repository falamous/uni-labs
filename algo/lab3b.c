#include <stdlib.h>
#include "struct.h"
#include "util.h"



OpenDict keyspace1;
ChainedDict keyspace2;
FILE *logfile;
FILE *keyfile;

struct keyspace_entry {
        struct keyspace_entry *next;
        int key1;
        int key2;
        struct file_mapped info;
        int version;
};

void keyspace1_remove(void *v) {
        struct keyspace_entry *k1p;
        struct keyspace_entry **k2p;
        struct keyspace_entry *tmp;
        int key2;

        k1p = v;
        while (k1p) {
                k2p = (struct keyspace_entry **)chaineddict_get(&keyspace2, (Val) {.i = k1p->key2});
                while (*k2p) {
                        if ((*k2p)->key1 == k1p->key1) {
                                tmp = *k2p;
                                *k2p = (*k2p)->next;
                                if (logfile) {
                                        ffree(&tmp->info);
                                }
                                free(tmp);
                        } else {
                                k2p = &((*k2p)->next);
                        }
                }
                tmp = k1p;
                k1p = k1p->next;
                free(tmp);
        }
}

void keyspace2_remove(void *v) {
        struct keyspace_entry **k1p;
        struct keyspace_entry *k2p;
        struct keyspace_entry *tmp;
        int key2;

        k2p = v;
        while (k2p) {
                k1p = (struct keyspace_entry **)opendict_get(&keyspace1, (Val){.i = k2p->key1});
                while (*k1p) {
                        if ((*k1p)->key2 == k2p->key2) {
                                tmp = *k1p;
                                *k1p = (*k1p)->next;
                                if (logfile) {
                                        ffree(&tmp->info);
                                }
                                free(tmp);
                        }
                        k1p = &((*k1p)->next);
                }
                tmp = k2p;
                k2p = k2p->next;
                free(tmp);
        }
}


void keyspace_remove(int key1, int key2, int version) {
        struct keyspace_entry **k1p;
        struct keyspace_entry **k2p;
        struct keyspace_entry *tmp;

        k1p = (struct keyspace_entry **)opendict_get(&keyspace1, (Val){.i = key1});
        if (!k1p) {
                eprintf("Key1 not in table.");
                return;
        }
        while (*k1p) {
                if ((*k1p)->key2 == key2 && (version < 0 || (*k1p)->version == version)) {
                        tmp = *k1p;
                        *k1p = (*k1p)->next;
                        ffree(&tmp->info);
                        free(tmp);
                } else {
                        k1p = &(*k1p)->next;
                }
        }

        k2p = (struct keyspace_entry **)chaineddict_get(&keyspace2, (Val){.i = key2});
        if (!k2p) {
                eprintf("Key2 not in table.");
                return;
        }
        while (*k2p) {
                if ((*k2p)->key1 == key1 && (version < 0 || (*k2p)->version == version)) {
                        tmp = *k2p;
                        *k2p = (*k2p)->next;
                        /* free(tmp->info); */
                        free(tmp);
                } else {
                        k2p = &(*k2p)->next;
                }
        }

}


void keyspace_set(int key1, int key2, struct file_mapped info, int version) {
        struct keyspace_entry **k1p;
        struct keyspace_entry **k2p;

        k1p = (struct keyspace_entry **)opendict_get(&keyspace1, (Val){.i = key1});
        if (!k1p) {
                opendict_set(&keyspace1, (Val){.i = key1}, NULL);
                k1p = (struct keyspace_entry **)opendict_get(&keyspace1, (Val){.i = key1});
        }

        while (*k1p) {
                if ((*k1p)->key2 == key2) {
                        version = version > (*k1p)->version ? version : (*k1p)->version;
                }
                k1p = &((*k1p)->next);
        }
        *k1p = xmalloc(sizeof(struct keyspace_entry));
        (*k1p)->key1 = key1;
        (*k1p)->key2 = key2;
        (*k1p)->info = info;
        (*k1p)->version = version + 1;
        (*k1p)->next = NULL;

        version = -1;
        k2p = (struct keyspace_entry **)chaineddict_get(&keyspace2, (Val){.i = key2});
        if (!k2p) {
                chaineddict_set(&keyspace2, (Val){.i = key2}, NULL);
                k2p = (struct keyspace_entry **)chaineddict_get(&keyspace2, (Val){.i = key2});
        }

        while (*k2p) {
                if ((*k2p)->key1 == key1) {
                        version = version > (*k2p)->version ? version : (*k2p)->version;
                }
                k2p = &((*k2p)->next);
        }
        *k2p = xmalloc(sizeof(struct keyspace_entry));
        (*k2p)->key1 = key1;
        (*k2p)->key2 = key2;
        (*k2p)->info = info;
        (*k2p)->version = version + 1;
        (*k2p)->next = NULL;

}

void menu() {
        puts("1. Add element.");
        puts("2. Get element by key.");
        puts("3. Remove element.");
        puts("4. Remove key.");
        puts("5. Print table.");
        puts("6. Exit.");
}

void keyspace1_find(int key1) {
        struct keyspace_entry **k1p;
        char *info;
        k1p = (struct keyspace_entry **)opendict_get(&keyspace1, (Val){.i = key1});
        puts("{");
        if (k1p) {
                while (*k1p) {
                        info = freadstr(&(*k1p)->info);
                        printf("key1=%i, key2=%i, version=%i, info=%s\n", key1, (*k1p)->key2, (*k1p)->version, info);
                        free(info);
                        k1p = &(*k1p)->next;
                }
        }
        puts("}");
}

void keyspace2_find(int key2) {
        struct keyspace_entry **k2p;
        char *info;
        k2p = (struct keyspace_entry **)chaineddict_get(&keyspace2, (Val){.i = key2});
        puts("{");
        if (k2p) {
                while (*k2p) {
                        info = freadstr(&(*k2p)->info);
                        printf("key1=%i, key2=%i, version=%i, info=%s\n", (*k2p)->key1, key2, (*k2p)->version, info);
                        free(info);
                        k2p = &(*k2p)->next;
                }
        }
        puts("}");
}

void keyspace_find(int key1, int key2, int version) {
        struct keyspace_entry **k1p;
        char *info;
        k1p = (struct keyspace_entry **)opendict_get(&keyspace1, (Val){.i = key1});
        puts("{");
        if (k1p) {
                while (*k1p) {
                        if (((*k1p)->key2 == key2 || key2 < 0) && ((*k1p)->version == version || version < 0)) {
                                info = freadstr(&(*k1p)->info);
                                printf("key1=%i, key2=%i, version=%i, info=%s\n", key1, (*k1p)->key2, (*k1p)->version, info);
                                free(info);
                        }
                        k1p = &(*k1p)->next;
                }
        }
        puts("}");
}

void print_table() {
        DictEntry *elem;
        char *info;
        struct keyspace_entry *k1p;

        elem = keyspace1.elements;
        puts("{");
        while (elem) {
                k1p = elem->value;
                while (k1p) {
                        info = freadstr(&k1p->info);
                        printf("key1=%i, key2=%i, version=%i, info=%s\n", k1p->key1, k1p->key2, k1p->version, info);
                        free(info);
                        k1p = k1p->next;
                }
                elem = elem->next;
        }
        puts("}");
}

void save_to_files() {
        DictEntry *elem;
        ChainedDict *map;
        struct keyspace_entry *k1p;

        map = fdefragment(logfile);

        fseek(keyfile, 0, SEEK_SET);

        elem = keyspace1.elements;
        while (elem) {
                k1p = elem->value;
                while (k1p) {

                        fputc(1, keyfile);
                        fwrite(&k1p->key1, 1, sizeof(k1p->key1), keyfile);
                        fwrite(&k1p->key2, 1, sizeof(k1p->key2), keyfile);
                        fwrite(&k1p->version, 1, sizeof(k1p->version), keyfile);
                        fwrite(chaineddict_get(map, (Val){.i = k1p->info.start}), 1, sizeof(size_t), keyfile);

                        k1p = k1p->next;
                }
                elem = elem->next;
        }
        fputc(0, keyfile);
        chaineddict_destroy(map);
        free(map);
}

void load_from_files() {
        int key1;
        int key2;
        int version;
        size_t start;
        int has_more;

        while ((has_more = fgetc(keyfile)) != EOF && has_more) {
                fread(&key1, 1, sizeof(key1), keyfile);
                fread(&key2, 1, sizeof(key2), keyfile);
                fread(&version, 1, sizeof(version), keyfile);
                fread(&start, 1, sizeof(size_t), keyfile);
                keyspace_set(key1, key2, (struct file_mapped) {.file = logfile, .start = start}, version - 1);
        }
}

int main() {
        int choice;
        int key1;
        int key2;
        int version;
        char *info;
        char *keyfile_name;
        char *logfile_name;
        setvbuf(stdin, NULL, _IONBF, 0);
        setvbuf(stdout, NULL, _IONBF, 0);

        printf("logfile: ");
        logfile_name = inputline(stdin);
        printf("keyfile: ");
        keyfile_name = inputline(stdin);

        keyfile = fopen(keyfile_name, "rb+");
        logfile = fopen(logfile_name, "rb+");
        if (!keyfile || !logfile) {
                keyfile = fopen(keyfile_name, "wb+");
                logfile = fopen(logfile_name, "wb+");
                if (!keyfile || !logfile) {
                        puts("Can't open nessesary files.");
                        exit(-10);
                }
        }


        opendict_init(&keyspace1, 0, idhash, valcmp, NULL, keyspace1_remove);
        chaineddict_init(&keyspace2, 0, idhash, valcmp, NULL, keyspace2_remove);
        load_from_files();

        while (true) {
                menu();
                choice = -1;
                choice = input_int();
                switch(choice) {
                        case 1:
                                printf("key1: ");
                                key1 = input_int();
                                printf("key2: ");
                                key2 = input_int();
                                scanf("%*[^\n]");
                                scanf("%*1[\n]");
                                printf("info: ");
                                info = inputline(stdin);

                                keyspace_set(key1, key2, fmalloc(logfile, info), -1);
                                free(info);
                                break;
                        case 2:
                                printf("key1 (-1 for any): ");
                                key1 = input_int();
                                printf("key2 (-1 for any): ");
                                key2 = input_int();
                                if (key1 == -1 && key2 == -1) {
                                        eprintf("Can't search on 0 keys.");
                                }
                                if (key1 == -1) {
                                        keyspace2_find(key2);
                                } else if (key2 == -1) {
                                        keyspace1_find(key1);
                                } else {
                                        keyspace_find(key1, key2, -1);
                                }
                                break;
                        case 3:
                                printf("key1: ");
                                key1 = input_int();
                                printf("key2: ");
                                key2 = input_int();
                                printf("version (-1 for any): ");
                                version = input_int();
                                keyspace_remove(key1, key2, version);
                                break;
                        case 4:
                                printf("which key? (1 or 2): ");
                                if (input_int() == 1) {
                                        printf("key: ");
                                        key1 = input_int();
                                        if (opendict_get(&keyspace1, (Val){.i = key1})) {
                                                opendict_remove(&keyspace1, (Val){.i = key1});
                                        } else {
                                                puts("no such key");
                                        }
                                } else if (input_int() == 2) {
                                        printf("key: ");
                                        key2 = input_int();
                                        if (chaineddict_get(&keyspace2, (Val){.i = key2})) {
                                                chaineddict_remove(&keyspace2, (Val){.i = key2});
                                        } else {
                                                puts("no such key");
                                        }
                                } else {
                                        puts("Only 1 and 2 are allowed.");
                                }
                                break;
                        case 5:
                                print_table();
                                break;
                        case 6:
                                save_to_files();
                                fclose(logfile);
                                fclose(keyfile);
                                logfile = keyfile = NULL;
                                opendict_destroy(&keyspace1);
                                chaineddict_destroy(&keyspace2);
                                exit(0);

                }
        }


}
