#include <stdbool.h>
#include "struct.h"
#include "util.h"

void rbtree_init(RBTree *tree,
                int (* cmpfunc)(Val, Val),
                void (* key_destroy)(Val),
                void (* value_destroy)(void *)) {
        tree->root = NULL;
        tree->cmpfunc = cmpfunc;

        tree->key_destroy = key_destroy;
        tree->value_destroy = value_destroy;
}

static enum direction node_direction(RBTreeNode *node) {
        if (node && node->parent) {
                return node->parent->children[LEFT] == node ? LEFT : RIGHT;
        } else {
                return NODIRECTION;
        }
}

static enum rbtree_color node_color(RBTreeNode *node) {
        return node ? node->color : RBTREE_BLACK;
}

/* static void rbtree_rotate(RBTree *tree, RBTreeNode *node, enum direction dir) { */
/*         RBTreeNode *parent; */

/*         /1* the parent required for the operation so its existance is not checked *1/ */
/*         parent = node->parent; */

/*         parent->children[OTHERDIR(dir)] = node->children[dir]; */
/*         if (parent->children[OTHERDIR(dir)]) { parent->children[OTHERDIR(dir)]->parent = parent; } */
/*         node->children[dir] = parent; */
/*         node->parent = parent->parent; */
/*         if (node->parent) { */
/*                 node->parent->children[node_direction(parent)] = node; */
/*         } else { */
/*                 tree->root = node; */
/*         } */
/*         parent->parent = node; */
/* } */

static void rbtree_rotate(RBTree *tree, RBTreeNode *np, enum direction dir) {
        RBTreeNode *child;

        /* the child required for the operation so its existance is not checked */
        child = np->children[dir];

        np->children[dir] = child->children[OTHERDIR(dir)];
        if (np->children[dir]) { np->children[dir]->parent = np; }
        child->children[OTHERDIR(dir)] = np;
        child->parent = np->parent;
        if (child->parent) {
                child->parent->children[node_direction(np)] = child;
        } else {
                tree->root = child;
        }
        np->parent = child;
}

RBTreeNode *rbtree_find(RBTree *tree, Val key, RBTreeNode **parent) {
        RBTreeNode *node_parent;
        RBTreeNode *node;
        int cmprez;

        node = tree->root;
        node_parent = NULL;
        while (node) {
                node_parent = node;

                cmprez = tree->cmpfunc(key, node->key);
                if (!cmprez) {
                        break;
                }
                if (cmprez < 0) {
                        node = node->children[LEFT];
                }
                if (cmprez > 0) {
                        node = node->children[RIGHT];
                }
        }
        if (parent) { *parent = node_parent; }
        return node;
}

RBTreeNode *rbtree_lower_bound(RBTree *tree, Val key) {
        RBTreeNode *parent;
        RBTreeNode *node;
        int cmprez;

        node = rbtree_find(tree, key, &parent);
        if (node) {
                return node;
        }

        node = parent;
        if (!node) {
                return NULL;
        }

        if (node_direction(node) == LEFT) {
                node = node->parent;
                while (node && node->children[RIGHT]) {
                        node = node->children[RIGHT];
                }
        } else {
                while (node && node_direction(node) != LEFT) {
                        node = node->parent;
                }
        }

        return node;
}

RBTreeNode *rbtree_upper_bound(RBTree *tree, Val key) {
        RBTreeNode *parent;
        RBTreeNode *node;
        int cmprez;

        node = rbtree_find(tree, key, &parent);
        if (node) {
                return node;
        }

        node = parent;
        if (!node) {
                return NULL;
        }

        if (node_direction(node) == RIGHT) {
                node = node->parent;
                while (node && node->children[LEFT]) {
                        node = node->children[LEFT];
                }
        } else {
                while (node && node_direction(node) != RIGHT) {
                        node = node->parent;
                }
        }

        return node;
}

RBTreeNode *rbtree_any_bound(RBTree *tree, Val key) {
        RBTreeNode *node;
        node = rbtree_lower_bound(tree, key);
        if (!node) {
                node = rbtree_min(tree);
        }
        return node;
}
void rbtree_set(RBTree *tree, Val key, void *value) {
        RBTreeNode *parent;
        register RBTreeNode *node;
        register RBTreeNode *uncle;
        register enum direction dir;
        int cmprez;

        node = rbtree_find(tree, key, &parent);
        if (node) {
                if (tree->value_destroy) { tree->value_destroy(node->value); }
                node->value = value;
                return;
        }

        if (parent) {
                if (tree->cmpfunc(key, parent->key) < 0) {
                        node = parent->children[LEFT] = xmalloc(sizeof(RBTreeNode));
                } else {
                        node = parent->children[RIGHT] = xmalloc(sizeof(RBTreeNode));
                }
        } else {
                        node = tree->root = xmalloc(sizeof(RBTreeNode));
        }

        node->key = key;
        node->value = value;
        node->color = RBTREE_RED;
        node->parent = parent;
        node->children[LEFT] = NULL;
        node->children[RIGHT] = NULL;

        while (node && (parent = node->parent) && parent->color != RBTREE_BLACK) {
                if (parent->parent == NULL) {
                        /* parent is root */
                        parent->color = RBTREE_BLACK;
                        break;
                }
                dir = node_direction(node);

                uncle = parent->parent->children[OTHERDIR(node_direction(parent))];
                if (node_color(uncle) == RBTREE_RED) {
                        parent->parent->color = RBTREE_RED;
                        parent->color = RBTREE_BLACK;
                        uncle->color = RBTREE_BLACK;

                        node = parent->parent;
                } else /* if (uncle->color == RBTREE_BLACK) */ {
                        if (node_direction(parent) != dir) {
                                rbtree_rotate(tree, parent, dir);
                                parent = node;
                                dir = OTHERDIR(dir);
                        }
                        parent->parent->color = RBTREE_RED;
                        parent->color = RBTREE_BLACK;
                        rbtree_rotate(tree, parent->parent, dir);
                        break;
                }

        }
}

static RBTreeNode *inorder_successor(RBTreeNode *node) {
        while (node && node->children[LEFT]) {
                node = node->children[LEFT];
        }
        return node;
}

static RBTreeNode *postorder_successor(RBTreeNode *node) {
        while (node && node->children[RIGHT]) {
                node = node->children[RIGHT];
        }
        return node;
}

RBTreeNode *rbtree_next(RBTreeNode *node) {
        if (!node) { return NULL; }

        if (node->children[RIGHT]) {
                return inorder_successor(node->children[RIGHT]);
        }

        if (node_direction(node) == LEFT) {
                return node->parent;
        }

        while (node && node_direction(node) == RIGHT) {
                node = node->parent;
        }
        if (node) {
            node = node->parent;
        }
        return node;
}

RBTreeNode *rbtree_prev(RBTreeNode *node) {
        if (!node) { return NULL; }

        if (node->children[LEFT]) {
                return inorder_successor(node->children[RIGHT]);
        }
        if (node_direction(node) == RIGHT) {
                return node->parent;
        }
        while (node && node_direction(node) != RIGHT) {
                node = node->parent;
        }
        return node;
}


static void rbtree_print_dfs(struct rbtree_node *node, FILE *stream, rbtree_printfunc_t printfunc, FILE *null_stream, int len_so_far) {
    int node_len;

    if (!node) {
        return;
    }
    node_len = printfunc(null_stream, node);
    rbtree_print_dfs(node->children[1], stream, printfunc, null_stream, len_so_far + node_len);
    fprintf(stream, "%*s", len_so_far, "");
    printfunc(stream, node);
    fputc('\n', stream);
    rbtree_print_dfs(node->children[0], stream, printfunc, null_stream, len_so_far + node_len);
}

void rbtree_print(RBTree *tree, FILE *stream, rbtree_printfunc_t printfunc) {
    FILE *null_stream;
    null_stream = fopen("/dev/null", "w");
    if (!null_stream) {
        die(-4, "Can't open /dev/zero for writing.\n");
    }
    rbtree_print_dfs(tree->root, stream, printfunc, null_stream, 0);
    fclose(null_stream);
}

void rbtree_remove(RBTree *tree, Val key) {
        register RBTreeNode *node;
        register RBTreeNode *old_node;
        register RBTreeNode *parent;
        register RBTreeNode *sibling;
        RBTreeNode *successor;
        register enum direction dir;

        old_node = rbtree_find(tree, key, NULL);
        if (!old_node) { return; }
        if (tree->key_destroy) { tree->key_destroy(old_node->key); }
        if (tree->value_destroy) { tree->value_destroy(old_node->value); }

        /* Binary search tree delete. */
        for(;;) {
                parent = old_node->parent;
                if (!!old_node->children[LEFT] && !!old_node->children[RIGHT]) {
                        /* There always exists a successor
                         * because the node has at least 2 children.
                         */
                        successor = inorder_successor(old_node->children[RIGHT]);
                        old_node->value = successor->value;
                        old_node->key = successor->key;
                        old_node = successor;
                } else {
                        dir = node_direction(old_node);
                        if (parent) {
                                parent->children[node_direction(old_node)] = node =
                                        old_node->children[LEFT] ?: old_node->children[RIGHT];
                                if (node) { node->parent = parent; }
                        } else {
                                tree->root = node =
                                        old_node->children[LEFT] ?: old_node->children[RIGHT];
                                parent = NULL;
                                if (node) { node->parent = NULL; }
                        }
                        break;
                }
        }


        if (old_node->color == RBTREE_RED) {
                free(old_node);
                return;
        }
        if (node_color(node) == RBTREE_RED) {
                free(old_node);
                node->color = RBTREE_BLACK;
                return;
        }

        /* 
         * this is nessesary for leafs
         * works, because the parent is never changed
         */
        node = old_node;

        while (parent = node->parent) {
                sibling = parent->children[OTHERDIR(dir)];
                /*
                 *   Pp
                 *  / \
                 * Nn  Ss
                 *    / \
                 *   Cc  Dd
                 *
                 * Uppercase letters are used for red vertices,
                 * lowercase - for black vertices,
                 * both cases (eg. 'Pp') - for vertices of any color.
                 *
                 */

                /* sibling has black length at least 1 so it is not NULL */
                if (node_color(sibling) == RBTREE_RED) {
                        /* 
                         * p, c, d - black
                         * S - red
                         */

                        rbtree_rotate(tree, parent, dir);
                        /*
                         *   p            S          s        P
                         *  / \          / \        / \      / \
                         * n   S   =>   p   d =>   P   d => n   s
                         *    / \      / \        / \
                         *   c   d    n   c      n   c
                         */
                        parent->color = RBTREE_RED;
                        sibling->color = RBTREE_BLACK;
                        /* S = C */
                        sibling = parent->children[OTHERDIR(dir)];
                }

                if (
                                node_color(sibling->children[dir]) == RBTREE_RED &&
                                node_color(sibling->children[OTHERDIR(dir)]) == RBTREE_BLACK) {
                        /*
                         * d - black
                         * C - red
                         */
                        rbtree_rotate(tree, sibling, dir);
                        /*
                         *   Pp         Pp            Pp
                         *  / \        / \           / \
                         * n   s   => n   C      => n   c
                         *    / \        / \           / \
                         *   C   d          s             S
                         *                 / \           / \
                         *                    d             d 
                         */
                        sibling->color = RBTREE_BLACK;
                        if (sibling->children[dir]) { sibling->children[dir]->color = RBTREE_BLACK; }
                        sibling = parent->children[OTHERDIR(dir)];
                }

                if (node_color(sibling->children[OTHERDIR(dir)]) == RBTREE_RED) {
                        /*
                         * s - black
                         * D  - red
                         */
                        rbtree_rotate(tree, parent, OTHERDIR(dir));
                        /*
                         *   Pp           s          Ss
                         *  / \          / \        / \
                         * n   s   =>   Pp  D =>   p   d
                         *    / \      /          /
                         *       D    n          n
                         */
                        sibling->color = parent->color;
                        parent->color = RBTREE_BLACK;
                        sibling->children[OTHERDIR(dir)]->color = RBTREE_BLACK;
                        break;
                }

                if (parent->color == RBTREE_RED) {
                        /*
                         * s, c, d - black
                         * p - red
                         */

                        /*   P          p
                         *  / \        / \
                         * n   s   => n   S
                         *    / \        / \
                         *   c   d      c   d
                         */
                        parent->color = RBTREE_BLACK;
                        sibling->color = RBTREE_RED;
                        break;
                        
                }
                /* if (parent->color == RBTREE_BLACK) */ {
                        /*
                         * p, s, c, d - black
                         */

                        /*   p          p
                         *  / \        / \
                         * n   s   => n   S
                         *    / \        / \
                         *   c   d      c   d
                         */
                        sibling->color = RBTREE_RED;
                        node = parent;
                        if (node->parent) { dir = node_direction(node); }
                }
        }
        free(old_node);
}

void **rbtree_get(RBTree *tree, Val key) {
        RBTreeNode *node;
        
        node = rbtree_find(tree, key, NULL);
        return node ? &(node->value) : NULL;
}

RBTreeNode *rbtree_min(RBTree *tree) {
        return inorder_successor(tree->root);
}

RBTreeNode *rbtree_max(RBTree *tree) {
        return postorder_successor(tree->root);
}

static void rbtree_destroy_dfs(RBTree *tree, RBTreeNode *node) {
        if (!node) {
                return;
        }
        rbtree_destroy_dfs(tree, node->children[0]);
        rbtree_destroy_dfs(tree, node->children[1]);
        if (tree->key_destroy) { tree->key_destroy(node->key); }
        if (tree->value_destroy) { tree->value_destroy(node->value); }
        free(node);
}

void rbtree_destroy(RBTree *tree) {
        rbtree_destroy_dfs(tree, tree->root);
}


static bool rbtree_is_valid_dfs(RBTree *tree, RBTreeNode *node, int black_count, int *needed_black_count) {
        if (!node){
                if (*needed_black_count <= 0) {
                        *needed_black_count = black_count;
                }
                return *needed_black_count == black_count;
        }
        if (node_color(node) == RBTREE_RED && (
                                node_color(node->children[LEFT]) != RBTREE_BLACK || 
                                node_color(node->children[LEFT]) != RBTREE_BLACK
                                )) {
                return false;
        }
        if (node->children[LEFT] && tree->cmpfunc(node->key, node->children[LEFT]->key) < 0) {
                return false;
        }
        if (node->children[RIGHT] && tree->cmpfunc(node->key, node->children[RIGHT]->key) > 0) {
                return false;
        }

        if (node_color(node) == RBTREE_BLACK) {
                black_count++;
        }

        return rbtree_is_valid_dfs(tree, node->children[LEFT], black_count, needed_black_count) &&
                rbtree_is_valid_dfs(tree, node->children[LEFT], black_count, needed_black_count);
}

bool rbtree_is_valid(RBTree *tree) {
        int black_count = 0;
        return rbtree_is_valid_dfs(tree, tree->root, 0, &black_count);
}

/* static void rbtree_print_dfs( */
/*                 RBTreeNode *node, */
/*                 int printfunc(RBTreeNode *node), */
/*                 int space_count, */
/*                 int space) { */
/*         if (node == NULL) { return; } */
/*         space += space_count; */
/*         rbtree_print_dfs(node->children[LEFT], printfunc, space_count, space); */
/*         printf("%*.s", space, "\0"); */
/*         /1* space += printfunc(node); *1/ */
/*         printfunc(node); */
/*         putchar('\n'); */
/*         rbtree_print_dfs(node->children[RIGHT], printfunc, space_count, space); */
/* } */

/* void rbtree_print( */
/*                 RBTree *tree, */
/*                 int printfunc(RBTreeNode *node), */
/*                 int space_count) { */

/*         rbtree_print_dfs(tree->root, printfunc, space_count, -space_count); */
/* } */

int printi(RBTreeNode *node) {
        int res;
        if (node->color == RBTREE_RED) {
                printf("\x1b[31m");
        }
        /* res = printf("%i", node->key.i); */
        res = printf("%s", node->key.p);
        printf("\x1b[0m");
        return res;
}
