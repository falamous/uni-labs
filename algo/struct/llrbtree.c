#include <stdbool.h>
#include "struct.h"
#include "util.h"


static bool RED = true;
static bool BLACK = false;

struct llrbtree_node {
        Val key;
        Val value;
        bool color;
        struct llrbtree_node *left, *right;
};

typedef struct llrbtree {
        struct llrbtree_node *root;

        int (* cmpfunc)(Val, Val);

        void (* key_destroy)(Val);
        void (* value_destroy)(Val);
} LLRBTree;


void llrbtree_init(LLRBTree *tree,
                int (* cmpfunc)(Val, Val),
                void (* key_destroy)(Val),
                void (* value_destroy)(Val)) {
        tree->root = NULL;
        tree->cmpfunc = cmpfunc;

        tree->key_destroy = key_destroy;
        tree->value_destroy = value_destroy;
}

static bool llrbtree_is_red(struct llrbtree_node *node) {
    return node && node->color == RED;
}

static struct llrbtree_node *llrbtree_rotate_left(struct llrbtree_node *node) {
    struct llrbtree_node *child;
    child = node->right;
    node->right = child->left;
    child->left = node;
    child->color = node->color;
    node->color = RED;
    return child;
}

static struct llrbtree_node *llrbtree_rotate_right(struct llrbtree_node *node) {
    struct llrbtree_node *child;
    child = node->left;
    node->left = child->right;
    child->right = node;
    child->color = node->color;
    node->color = RED;
    return child;
}

static void llrbtree_flip_color(struct llrbtree_node *node) {
    node->color = !node->color;
    node->left->color =  !node->left->color;
    node->right->color = !node->right->color;
}

struct llrbtree_node *llrbtree_fix_up(struct llrbtree_node *node) {
    if (!node) {
        return node;
    }

	if (llrbtree_is_red(node->right)) {
		node = llrbtree_rotate_left(node);
	}
	if (llrbtree_is_red(node->left) && llrbtree_is_red(node->left->left)) {
		node = llrbtree_rotate_right(node);
	}
	if (llrbtree_is_red(node->left) && llrbtree_is_red(node->right)) {
		llrbtree_flip_color(node);
	}
	return node;
}

struct llrbtree_node *llrbtree_find(LLRBTree *tree, Val key) {
    struct llrbtree_node *node;
    int cmp;

    node = tree->root;
    while (node) {
        cmp = tree->cmpfunc(key, node->key);
        if (cmp == 0) {
            break;
        } else if (cmp > 0) {
            node = node->right;
        } else if (cmp < 0) {
            node = node->left;
        }
    }
    return node;
}

Val *llrbtree_get(LLRBTree *tree, Val key) {
    struct llrbtree_node *node;
    node = llrbtree_find(tree, key);
    return node ? &node->value : NULL;
}

struct llrbtree_node *llrbtree_node_new(Val key, Val value, bool color) {
    struct llrbtree_node *node;
    node = malloc(sizeof(*node));
    node->key = key;
    node->value = value;
    node->color = color;
    node->left = NULL;
    node->right = NULL;
    return node;
}

static struct llrbtree_node *llrbtree_insert(LLRBTree *tree, struct llrbtree_node *node, Val key, Val value) {
    int cmp;

    if (node == NULL) {
        return llrbtree_node_new(key, value, RED);
    }

    cmp = tree->cmpfunc(key, node->key);

    if (cmp == 0) {
        if (tree->value_destroy) { tree->value_destroy(value); }
        node->value = value;
    } else if (cmp < 0) {
        node->left =  llrbtree_insert(tree, node->left, key, value);
    } else {
        node->right = llrbtree_insert(tree, node->right, key, value);
    }
    return llrbtree_fix_up(node);

}




void llrbtree_set(LLRBTree *tree, Val key, Val value) {
    tree->root = llrbtree_insert(tree, tree->root, key, value);
    tree->root->color = BLACK;
}

static void llrbtree_destroy_dfs(LLRBTree *tree, struct llrbtree_node *node) {
        if (!node) {
                return;
        }
        llrbtree_destroy_dfs(tree, node->left);
        llrbtree_destroy_dfs(tree, node->right);
        if (tree->key_destroy) { tree->key_destroy(node->key); }
        if (tree->value_destroy) { tree->value_destroy(node->value); }
        free(node);
}

void llrbtree_destroy(LLRBTree *tree) {
        llrbtree_destroy_dfs(tree, tree->root);
}


static bool llrbtree_is_valid_dfs(LLRBTree *tree, struct llrbtree_node *node, int black_count, int *needed_black_count) {
        if (!node){
                if (*needed_black_count <= 0) {
                        *needed_black_count = black_count;
                }
                return *needed_black_count == black_count;
        }
        if (llrbtree_is_red(node) && (
                                llrbtree_is_red(node->left) || 
                                llrbtree_is_red(node->left)
                                )) {
                return false;
        }
        if (node->left && tree->cmpfunc(node->key, node->left->key) < 0) {
                return false;
        }
        if (node->right && tree->cmpfunc(node->key, node->right->key) > 0) {
                return false;
        }

        if (!llrbtree_is_red(node)) {
                black_count++;
        }

        return llrbtree_is_valid_dfs(tree, node->left, black_count, needed_black_count) &&
                llrbtree_is_valid_dfs(tree, node->left, black_count, needed_black_count);
}

static struct llrbtree_node *llrbtree_move_red_right(struct llrbtree_node *node){
	llrbtree_flip_color(node);
	if (llrbtree_is_red(node->left->left)) {
		node = llrbtree_rotate_right(node);
		llrbtree_flip_color(node);
	}
	return node;
}

static struct llrbtree_node *llrbtree_move_red_left(struct llrbtree_node *node){
	llrbtree_flip_color(node);
	if (llrbtree_is_red(node->right->left)) {
		node->right = llrbtree_rotate_right(node->right);
		node = llrbtree_rotate_left(node);
		llrbtree_flip_color(node);
	}
	return node;
}

static struct llrbtree_node *llrbtree_delete_min(LLRBTree *tree, struct llrbtree_node *node){
    struct llrbtree_node *result;

    if (node->left == NULL) {
        free(node);
        return NULL;
    }
    if (!llrbtree_is_red(node->left) && node->left && !llrbtree_is_red(node->left->left)) {
        node = llrbtree_move_red_left(node);
    }
    node->left = llrbtree_delete_min(tree, node->left);
    return llrbtree_fix_up(node);
}

struct llrbtree_node *llrbtree_min(struct llrbtree_node *node) {
    while (node && node->left) {
        node = node->left;
    }

    return node;
}

static struct llrbtree_node *llrbtree_delete(LLRBTree *tree, struct llrbtree_node *node, Val key){
    struct llrbtree_node *min_node;
    int cmp;

    if (!node) {
        return NULL;
    }

	cmp = tree->cmpfunc(key, node->key);
	if (cmp < 0) {
		// RS forgot to ensure left exists before left->left check
		if (!llrbtree_is_red(node->left) && &node->left != NULL && !llrbtree_is_red(node->left->left)) {
			node = llrbtree_move_red_left(node);
		}
		node->left = llrbtree_delete(tree, node->left, key);
	} else {
		if (llrbtree_is_red(node->left)) {
			node = llrbtree_rotate_right(node);
            cmp = tree->cmpfunc(key, node->key);
		}
        if (cmp == 0 && node->right == NULL) {
            if (tree->value_destroy) { tree->value_destroy(node->value); }
            if (tree->key_destroy) { tree->key_destroy(node->key); }
            free(node);
			return NULL;
		}
		// RS forgot to ensure right exists before right->left check
		if (!llrbtree_is_red(node->right) && node->right != NULL && !llrbtree_is_red(node->right->left)) {
			node = llrbtree_move_red_right(node);
            cmp = tree->cmpfunc(key, node->key);
		}
		if (cmp == 0) {
            min_node = llrbtree_min(node->right);

            if (tree->value_destroy) { tree->value_destroy(node->value); }
            if (tree->key_destroy) { tree->key_destroy(node->key); }

			node->key = min_node->key;
			node->value = min_node->value;
			node->right = llrbtree_delete_min(tree, node->right);
		} else {
			node->right = llrbtree_delete(tree, node->right, key);
		}
	}
	return llrbtree_fix_up(node);
}

struct llrbtree_node *llrbtree_remove(LLRBTree *tree, Val key) {
    tree->root = llrbtree_delete(tree, tree->root, key);
    if (tree->root) {
        tree->root->color = BLACK;
    }
}
bool llrbtree_is_valid(LLRBTree *tree) {
        int black_count = 0;
        return llrbtree_is_valid_dfs(tree, tree->root, 0, &black_count);
}
