#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

#define OTHERDIR(dir) (1 ^ (dir))

#define mutex_init pthread_mutex_init
#define mutex_lock pthread_mutex_lock
#define mutex_unlock pthread_mutex_unlock
#define mutex_destroy pthread_mutex_destroy

typedef pthread_mutex_t mutex;


enum direction {
        LEFT = 0,
        RIGHT = 1,
        NODIRECTION = 0xff,
};


typedef union StructValue {
        void *p;
        uint64_t i;
} StructValue;

typedef StructValue Val;

typedef unsigned long hash_t;


typedef struct Vector {
        StructValue *arr;
        size_t len;
        size_t cap;
} Vector;


#ifdef LINKED_LIST_STACK
struct StackNode {
        StructValue val;
        struct StackNode *bp;
};

typedef struct Stack {
        struct StackNode *stack;
        mutex mutex;
} Stack;
#else
typedef struct Stack {
        Vector stack;
        mutex mutex;
} Stack;
#endif

typedef struct dict_entry {
        Val key;
        void *value;
        hash_t hash;
        struct dict_entry *next;
        struct dict_entry *prev;
} DictEntry;

typedef struct chaineddict_link {
        DictEntry *entry;
        struct chaineddict_link *next;
} ChainedDictLink;

typedef struct chaineddict{
        size_t table_size;
        ChainedDictLink **table;
        DictEntry *elements;
        DictEntry *elements_end;

        hash_t (* hashfunc)(Val);
        int (* cmpfunc)(Val, Val);

        void (* key_destroy)(Val);
        void (* value_destroy)(void *);
} ChainedDict;

typedef struct opendict{
        size_t table_size;
        DictEntry **table;
        size_t entry_count;
        DictEntry *elements;
        DictEntry *elements_end;


        hash_t (* hashfunc)(Val);
        int (* cmpfunc)(Val, Val);

        void (* key_destroy)(Val);
        void (* value_destroy)(void *);
} OpenDict;

typedef struct heap {
        Vector heap;
        int (*cmpfunc)(Val, Val);
} Heap;

enum rbtree_color {
        RBTREE_BLACK,
        RBTREE_RED,
};

typedef struct rbtree_node {
        Val key;
        void *value;
        enum rbtree_color color;
        struct rbtree_node *parent;
        struct rbtree_node *children[2];
} RBTreeNode;

typedef struct rbtree{
        RBTreeNode *root;

        int (* cmpfunc)(Val, Val);

        void (* key_destroy)(Val);
        void (* value_destroy)(void *);
} RBTree;


typedef size_t (*rbtree_printfunc_t)(FILE *stream, struct rbtree_node *node);

void vector_init(Vector *vec, size_t len);
void vector_from_arr(Vector *vec, StructValue *arr, size_t len);
void vector_resize(Vector *vec, size_t len);
void vector_reserve(Vector *vec, size_t cap);
void vector_pb(Vector *vec, StructValue val);
int vector_pop(Vector *vec, StructValue *val);
int vector_last(Vector *vec, StructValue *val);
void vector_destroy(Vector *vec);

void stack_init(Stack *stack);
void stack_push(Stack *stack, StructValue val);
int stack_pop(Stack *stack, StructValue *val);
int stack_top(Stack *stack, StructValue *val);
void stack_destroy(Stack *stack);

void chaineddict_init(
                ChainedDict *dict, size_t size,
                hash_t (* hashfunc)(Val), 
                int (* cmpfunc)(Val, Val),
                void (* key_destroy)(Val),
                void (* value_destroy)(void *)
                );
void chaineddict_remove(ChainedDict *dict, Val key);
void chaineddict_set(ChainedDict *dict, Val key, void *value);
void **chaineddict_get(ChainedDict *dict, Val key);
void chaineddict_destroy(ChainedDict *dict);

hash_t idhash(Val v);
int valcmp(Val a, Val b);

void opendict_init(
                OpenDict *dict, size_t size,
                hash_t (* hashfunc)(Val), 
                int (* cmpfunc)(Val, Val),
                void (* key_destroy)(Val),
                void (* value_destroy)(void *)
                );
void opendict_remove(OpenDict *dict, Val key);
void opendict_rebuild(OpenDict *dict, size_t size);
void opendict_set(OpenDict *dict, Val key, void *value);
void **opendict_get(OpenDict *dict, Val key);
void opendict_destroy(OpenDict *dict);

void rbtree_init(RBTree *tree,
                int (* cmpfunc)(Val, Val),
                void (* key_destroy)(Val),
                void (* value_destroy)(void *));
RBTreeNode *rbtree_find(RBTree *tree, Val key, RBTreeNode **parent);
void rbtree_set(RBTree *tree, Val key, void *value);
RBTreeNode *rbtree_next(RBTreeNode *node);
RBTreeNode *rbtree_prev(RBTreeNode *node);
void rbtree_remove(RBTree *tree, Val key);
void **rbtree_get(RBTree *tree, Val key);
RBTreeNode *rbtree_lower_bound(RBTree *tree, Val key);
RBTreeNode *rbtree_upper_bound(RBTree *tree, Val key);
RBTreeNode *rbtree_any_bound(RBTree *tree, Val key);
RBTreeNode *rbtree_min(RBTree *tree);
RBTreeNode *rbtree_max(RBTree *tree);
void rbtree_print(RBTree *tree, FILE *stream, rbtree_printfunc_t printfunc);
void rbtree_destroy(RBTree *tree);
bool rbtree_is_valid(RBTree *tree);

void heap_init(Heap *heap, int (*cmpfunc)(Val, Val));
int heap_top(Heap *heap, Val *val);
int heap_pop(Heap *heap, Val *val);
void heap_push(Heap *heap, Val val);
bool heap_empty(Heap *heap);

struct file_mapped {
        FILE *file;
        size_t start;
};

struct file_mapped fmalloc(FILE *file, char *s);
void ffree(struct file_mapped *fm);
ChainedDict *fdefragment(FILE *file);
char *freadstr(struct file_mapped *fm);
