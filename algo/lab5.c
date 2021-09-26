#include <stdio.h>
#include <string.h>
#include "struct.h"
#include "util.h"

#define LABYRINTH_EXIT 0b100
#define LABYRINTH_VERTEX 0b1000
#define HORIZONTAL_EDGE 0b1
#define VERTICAL_EDGE 0b10

struct point {
        int x;
        int y;
};

struct labyrinth_edge {
        struct labyrinth_vertex *from;
        struct labyrinth_vertex *to;
};

struct labyrinth_vertex {
        int x;
        int y;
        char *name;
        bool is_exit;
        RBTree adjacent; /* (x,y) -> NULL */
};

typedef struct labyrinth {
        RBTree vertices_by_name; /* name -> vertex */
        RBTree edges_by_hor;
        RBTree edges_by_ver;
        RBTree vertices_by_hor;
        RBTree vertices_by_ver;
        RBTree vertices; /* (x, y) -> vertex */
} Labyrinth;

struct dijkstra_vertex_distance {
        int distance;
        Vector path;
        struct labyrinth_vertex *vertex;
};

int labyrinth_vertex_cmp(Val a, Val b) {
        const struct labyrinth_vertex *va = a.p;
        const struct labyrinth_vertex *vb = b.p;

        if (va->x > vb->x) {
                return 1;
        }
        if (va->x < vb->x) {
                return -1;
        }
        if (va->y > vb->y) {
                return 1;
        }
        if (va->y < vb->y) {
                return -1;
        }
        return 0;
}

int point_cmp(Val a, Val b) {
        const struct point *va = a.p;
        const struct point *vb = b.p;

        if (va->x > vb->x) {
                return 1;
        }
        if (va->x < vb->x) {
                return -1;
        }
        if (va->y > vb->y) {
                return 1;
        }
        if (va->y < vb->y) {
                return -1;
        }
        return 0;
}

void labyrinth_vertex_init(struct labyrinth_vertex *vertex, struct point point, char *name, bool is_exit) {
        vertex->x = point.x;
        vertex->y = point.y;
        vertex->name = strdup(name);
        vertex->is_exit = is_exit;

        rbtree_init(&vertex->adjacent, labyrinth_vertex_cmp, NULL, NULL);
}


int valstrcmp(Val a, Val b) {
        return strcmp((char *)a.p, (char *)b.p);
}
void valfree(Val val) {
        free(val.p);
}

void labyrinth_vertex_destroy(struct labyrinth_vertex *vertex) {
        rbtree_destroy(&vertex->adjacent);
        free(vertex->name);
}

void labyrinth_vertex_free(void *vertex) {
        labyrinth_vertex_destroy(vertex);
        free(vertex);
}

void labyrinth_init(Labyrinth *labyrinth) {
        rbtree_init(&labyrinth->vertices_by_name, valstrcmp, NULL, NULL);
        rbtree_init(&labyrinth->vertices, point_cmp, valfree, labyrinth_vertex_free);

        rbtree_init(&labyrinth->edges_by_hor, valcmp, NULL, free);
        rbtree_init(&labyrinth->edges_by_ver, valcmp, NULL, free);

        rbtree_init(&labyrinth->vertices_by_hor, valcmp, NULL, NULL);
        rbtree_init(&labyrinth->vertices_by_ver, valcmp, NULL, NULL);
}

void labyrinth_remove_edge(Labyrinth *labyrinth, struct labyrinth_vertex *from, struct labyrinth_vertex *to) {

        rbtree_remove(&from->adjacent, (Val){.p = to});
        rbtree_remove(&to->adjacent, (Val){.p = from});

        if (from->x == to->x) {
                rbtree_remove(&labyrinth->edges_by_hor, (Val){.i = ((unsigned long long)to->x<<32) + from->y > to->y ? from->y : to->y});
        }
        if (from->y == to->y) {
                rbtree_remove(&labyrinth->edges_by_ver, (Val){.i = ((unsigned long long)to->x<<32) + from->y > to->x ? from->x : to->x});
        }
}

void labyrinth_remove_vertex(Labyrinth *labyrinth, struct labyrinth_vertex *vertex) {
        RBTreeNode *iter;
        struct point point;

        iter = vertex->adjacent.root;
        while (iter) {
                labyrinth_remove_edge(labyrinth, vertex, iter->key.p);
                iter = vertex->adjacent.root;
        }
        
        point.x = vertex->x;
        point.y = vertex->y;
        rbtree_remove(&labyrinth->vertices_by_hor, (Val){.i = ((unsigned long long)vertex->x<<32) + vertex->y});
        rbtree_remove(&labyrinth->vertices_by_ver, (Val){.i = ((unsigned long long)vertex->y<<32) + vertex->x});
        rbtree_remove(&labyrinth->vertices_by_name, (Val){.p = vertex->name});
        rbtree_remove(&labyrinth->vertices, (Val){.p = &point});


}

struct labyrinth_vertex *input_vertex_by_coord(Labyrinth *labyrinth) {
        struct labyrinth_vertex **vertex;
        struct point point;
        printf("x = ");
        point.x = input_int();
        printf("y = ");
        point.y = input_int();

        vertex = (struct labyrinth_vertex **)rbtree_get(&labyrinth->vertices, (Val){.p = &point});
        if (!vertex) {
                eprintf("Point with coord (%i, %i) doesn't exist.\n", point.x, point.y);
                return NULL;
        }
        return *vertex;
}

struct labyrinth_vertex *input_vertex_by_name(Labyrinth *labyrinth) {
        struct labyrinth_vertex **vertex;
        char *name;
        printf("name: ");
        name = inputline(stdin);

        vertex = (struct labyrinth_vertex **)rbtree_get(&labyrinth->vertices_by_name, (Val){.p = name});
        if (!vertex) {
                eprintf("Point with name %s doesn't exist.\n", name);
                return NULL;
        }
        return *vertex;
}

struct labyrinth_vertex *input_vertex(Labyrinth *labyrinth) {
        char *s;
        int c;
        for(;;) {
                printf("Type c(oordinates) or n(ame): ");
                c = getchar();
                switch (c) {
                        case 'C':
                        case 'c':
                                scanf("%*[^\n]");
                                scanf("%*1[\n]");
                                return input_vertex_by_coord(labyrinth);
                        case 'N':
                        case 'n':
                                scanf("%*[^\n]");
                                scanf("%*1[\n]");
                                return input_vertex_by_name(labyrinth);
                        case EOF:
                                exit(-1);
                        default:
                                continue;
                }
        }
}

/* void labyrinth_add_edge_by_name(Labyrinth *labyrinth, char *from_name, char *to_name) { */
/*         struct labyrinth_vertex **from; */
/*         struct labyrinth_vertex **to; */

/*         from = rbtree_get(&labyrinth->vertices_by_name, from_name); */
/*         to = rbtree_get(&labyrinth->vertices_by_name, to_name); */
/*         if (from == NULL) { */
/*                 eprintf("Point with name %s doesn't exist.", from_name); */
/*                 return; */
/*         } */
/*         if (to == NULL) { */
/*                 eprintf("Point with name %s doesn't exist.", to_name); */
/*                 return; */
/*         } */
/*         return labyrinth_add_edge(*from, *to); */
/* } */

/* void labyrinth_add_edge_by_point(Labyrinth *labyrinth, struct point from_point, struct point to_point) { */
/*         struct labyrinth_vertex **from; */
/*         struct labyrinth_vertex **to; */

/*         from = rbtree_get(&labyrinth->vertices, from_point); */
/*         to = rbtree_get(&labyrinth->vertices, {to_point}); */
/*         if (from == NULL) { */
/*                 return; */
/*         } */
/*         if (to == NULL) { */
/*                 eprintf("Point with coord (%i, %i) doesn't exist.", to_point.x, to_point.y); */
/*                 return; */
/*         } */
/*         return labyrinth_add_edge(*from, *to); */
/* } */

bool labyrinth_edges_intersect(struct labyrinth_edge *a, struct labyrinth_edge *b) {
        int a_max_x;
        int a_min_x;
        int a_max_y;
        int a_min_y;


        int b_max_x;
        int b_min_x;
        int b_max_y;
        int b_min_y;

        if (a->from->x > a->to->x) {
                a_max_x = a->from->x;
                a_min_x = a->to->x;
        } else {
                a_max_x = a->to->x;
                a_min_x = a->from->x;
        }
        if (a->from->y > a->to->y) {
                a_max_y = a->from->y;
                a_min_y = a->to->y;
        } else {
                a_max_y = a->to->y;
                a_min_y = a->from->y;
        }
        if (b->from->x > b->to->x) {
                b_max_x = b->from->x;
                b_min_x = b->to->x;
        } else {
                b_max_x = b->to->x;
                b_min_x = b->from->x;
        }
        if (b->from->y > b->to->y) {
                b_max_y = b->from->y;
                b_min_y = b->to->y;
        } else {
                b_max_y = b->to->y;
                b_min_y = b->from->y;
        }
        return
                a_max_x >= b_min_x && b_max_x >= a_min_x &&
                a_max_y >= b_min_y && b_max_y >= a_min_y;
}

void labyrinth_add_edge(Labyrinth *labyrinth, struct labyrinth_vertex *from, struct labyrinth_vertex *to) {
        RBTreeNode *iter;
        struct labyrinth_edge *edge;
        struct labyrinth_vertex *vertex;
        struct labyrinth_edge *new_edge;

        struct labyrinth_vertex *from_edge;
        struct labyrinth_vertex *to_edge;
        int min_coord;
        int max_coord;

        if (from->x == to->x && from->y == to->y) {
                /* Shouln't be reached. */
                eprintf("Attempting to add a corridor between point %s (%i, %i) and itself.\n", from->name, from->x, from->y);
                return;
        }

        if (from->x != to->x && from->y != to->y) {
                eprintf("Attempting to add a corridor between 2 points: "
                                "%s (%i, %i) and %s (%i, %i) - "
                                "that don't have a common coordinate.\n",
                                from->name, from->x, from->y,
                                to->name, to->x, to->y);
                return;
        }

        if (rbtree_get(&from->adjacent, (Val){.p = to})) {
                eprintf("Attempting to add a corridor between 2 points: "
                                "%s (%i, %i) and %s (%i, %i), "
                                "but one already exists.\n",
                                from->name, from->x, from->y,
                                to->name, to->x, to->y);
                return;
        }

        /* Check that edge doesn't cross any other edge */
        new_edge = xmalloc(sizeof(struct labyrinth_edge));
        new_edge->from = from;
        new_edge->to = to;

        if (from->x == to->x) {
                min_coord = from->y < to->y ? from->y : to->y;
                max_coord = from->y > to->y ? from->y : to->y;
                iter = rbtree_any_bound(&labyrinth->vertices_by_ver, (Val){.i = (unsigned long long)min_coord<<32});
                while (iter != NULL) {
                        vertex = iter->value;
                        if (vertex->y > max_coord) {
                                break;
                        }
                        if (min_coord < vertex->y && vertex->y < max_coord && vertex->x == from->x) {
                                free(new_edge);
                                labyrinth_add_edge(labyrinth, vertex, from);
                                labyrinth_add_edge(labyrinth, vertex, to);
                                return;
                        }
                        iter = rbtree_next(iter);
                }
                iter = rbtree_any_bound(&labyrinth->edges_by_ver, (Val){.i = (unsigned long long)min_coord<<32});
                while (iter != NULL) {
                        edge = iter->value;
                        if (edge->from->y > max_coord && edge->to->y > max_coord) {
                                break;
                        }
                        if (labyrinth_edges_intersect(new_edge, edge) && 
                                        edge->from != from && edge->from != to &&
                                        edge->to != from && edge->to != to) {
                                eprintf("Attempting to add a corridor between 2 points: "
                                                "%s (%i, %i) and %s (%i, %i) - "
                                                "that intersects with another corridor between "
                                                "%s (%i, %i) and %s (%i, %i).\n",
                                                from->name, from->x, from->y,
                                                to->name, to->x, to->y,
                                                edge->from->name, edge->from->x, edge->from->y,
                                                edge->to->name, edge->to->x, edge->to->y);
                                free(new_edge);
                                return;
                        }
                        iter = rbtree_next(iter);
                }
        }

        if (from->y == to->y) {
                min_coord = from->x < to->x ? from->x : to->x;
                max_coord = from->x > to->x ? from->x : to->x;
                iter = rbtree_any_bound(&labyrinth->vertices_by_hor, (Val){.i = (unsigned long long)min_coord<<32});
                while (iter != NULL) {
                        vertex = iter->value;
                        if (vertex->x > max_coord) {
                                break;
                        }
                        if (min_coord < vertex->x && vertex->x < max_coord && vertex->y == from->y) {
                                free(new_edge);
                                labyrinth_add_edge(labyrinth, vertex, from);
                                labyrinth_add_edge(labyrinth, vertex, to);
                                return;
                        }
                        iter = rbtree_next(iter);
                }
                iter = rbtree_any_bound(&labyrinth->edges_by_hor, (Val){.i = (unsigned long long)min_coord<<32});
                while (iter != NULL) {
                        edge = iter->value;
                        if (edge->from->x > max_coord && edge->to->x > max_coord) {
                                break;
                        }
                        if (labyrinth_edges_intersect(new_edge, edge) && 
                                        edge->from != from && edge->from != to &&
                                        edge->to != from && edge->to != to) {
                                eprintf("Attempting to add a corridor between 2 points: "
                                                "%s (%i, %i) and %s (%i, %i) - "
                                                "that intersects with another corridor between "
                                                "%s (%i, %i) and %s (%i, %i).\n",
                                                from->name, from->x, from->y,
                                                to->name, to->x, to->y,
                                                edge->from->name, edge->from->x, edge->from->y,
                                                edge->to->name, edge->to->x, edge->to->y);
                                free(new_edge);
                                return;
                        }
                        iter = rbtree_next(iter);
                }
        }

        rbtree_set(&from->adjacent, (Val){.p = to}, NULL);
        rbtree_set(&to->adjacent, (Val){.p = from}, NULL);

        if (from->x == to->x) {
                rbtree_set(&labyrinth->edges_by_hor, (Val){.i = ((unsigned long long)to->x<<32) + from->y > to->y ? from->y : to->y}, new_edge);
        }
        if (from->y == to->y) {
                rbtree_set(&labyrinth->edges_by_ver, (Val){.i = ((unsigned long long)to->y<<32) + from->x > to->x ? from->x : to->x}, new_edge);
        }
}

void labyrinth_destroy(Labyrinth *labyrinth) {
        rbtree_destroy(&labyrinth->vertices_by_hor);
        rbtree_destroy(&labyrinth->vertices_by_ver);

        rbtree_destroy(&labyrinth->edges_by_hor);
        rbtree_destroy(&labyrinth->edges_by_ver);

        rbtree_destroy(&labyrinth->vertices_by_name);
        rbtree_destroy(&labyrinth->vertices);

}

struct labyrinth_vertex *labyrinth_add_vertex(Labyrinth *labyrinth, struct point point, char *name, bool is_exit) {
        struct point *point_copy;
        struct labyrinth_vertex *vertex;
        RBTreeNode *iter;
        struct labyrinth_edge *edge;
        struct labyrinth_vertex *to;
        struct labyrinth_vertex *from;
        int max_c;
        int min_c;

        if (rbtree_get(&labyrinth->vertices_by_name, (Val){.p = name}) != NULL) {
                eprintf("Vertex with name %s already exists.\n");
                return NULL;
        }
        if (rbtree_get(&labyrinth->vertices, (Val){.p = &point}) != NULL) {
                eprintf("Vertex at point (%i, %i) already exists.\n", point.x, point.y);
                return NULL;
        }

        vertex = xmalloc(sizeof(struct labyrinth_vertex));
        labyrinth_vertex_init(vertex, point, name, is_exit);
        point_copy = xmalloc(sizeof(struct point));
        *point_copy = point;

        rbtree_set(&labyrinth->vertices_by_name, (Val){.p = vertex->name}, vertex);
        rbtree_set(&labyrinth->vertices, (Val){.p = point_copy}, vertex);

        rbtree_set(&labyrinth->vertices_by_hor, (Val){.i = ((unsigned long long)point.x<<32) + point.y}, vertex);
        rbtree_set(&labyrinth->vertices_by_ver, (Val){.i = ((unsigned long long)point.y<<32) + point.x}, vertex);

        iter = rbtree_any_bound(&labyrinth->edges_by_hor, (Val){ .i = ((unsigned long long)point.x<<32)});
        while (iter) {
                edge = iter->value;

                if (edge->from->y > edge->to->y) {
                        max_c = edge->from->y;
                        min_c = edge->to->y;
                } else {
                        max_c = edge->to->y;
                        min_c = edge->from->y;
                }
                if (point.x == edge->to->x && min_c < point.y && point.y < max_c) {
                        to = edge->to;
                        from = edge->from;
                        labyrinth_remove_edge(labyrinth, edge->from, edge->to);
                        labyrinth_add_edge(labyrinth, from, vertex);
                        labyrinth_add_edge(labyrinth, to, vertex);
                        iter = rbtree_any_bound(&labyrinth->edges_by_ver, (Val){ .i = ((unsigned long long)point.y<<32)});
                }
                iter = rbtree_next(iter);
        }

        iter = rbtree_any_bound(&labyrinth->edges_by_ver, (Val){ .i = ((unsigned long long)point.y<<32)});
        while (iter) {
                edge = iter->value;

                if (edge->from->x > edge->to->x) {
                        max_c = edge->from->x;
                        min_c = edge->to->x;
                } else {
                        max_c = edge->to->x;
                        min_c = edge->from->x;
                }
                if (point.y == edge->to->y && min_c < point.x && point.x < max_c) {
                        to = edge->to;
                        from = edge->from;
                        labyrinth_remove_edge(labyrinth, edge->from, edge->to);
                        labyrinth_add_edge(labyrinth, from, vertex);
                        labyrinth_add_edge(labyrinth, to, vertex);

                        iter = rbtree_any_bound(&labyrinth->edges_by_ver, (Val){ .i = ((unsigned long long)point.y<<32)});
                }
                iter = rbtree_next(iter);
        }
        return vertex;
}




int dijkstra_distance_cmp(Val a, Val b) {
        const struct dijkstra_vertex_distance *va = a.p;
        const struct dijkstra_vertex_distance *vb = b.p;
        return va->distance < vb->distance;
}

static int edge_len(struct labyrinth_vertex *a, struct labyrinth_vertex *b) {
        /* Because a->x == b->y || a->y == b->y: */
        return abs(a->x - b->x + a->y - b->y);
}

void labyrinth_find_exit(Labyrinth *labyrinth, struct labyrinth_vertex *vertex) {
        RBTree visited;
        Heap queue;
        int distance;
        size_t i;
        Vector path;
        RBTreeNode *adj_vertex;

        Val queue_vertex_val;
        struct dijkstra_vertex_distance *queue_vertex;
        struct labyrinth_vertex *current_vertex;

        rbtree_init(&visited, labyrinth_vertex_cmp, NULL, NULL);
        heap_init(&queue, dijkstra_distance_cmp);

        current_vertex = vertex;
        queue_vertex = malloc(sizeof(struct dijkstra_vertex_distance));
        queue_vertex->vertex = current_vertex;
        queue_vertex->distance = 0;

        vector_init(&path, 0);
        queue_vertex->path = path;

        heap_push(&queue, (Val) { .p = queue_vertex });

        while (!heap_empty(&queue)) {
                heap_pop(&queue, &queue_vertex_val);
                queue_vertex = queue_vertex_val.p;

                distance = queue_vertex->distance;
                path = queue_vertex->path;
                current_vertex = queue_vertex->vertex;
                free(queue_vertex);
                vector_pb(&path, (Val) { .p = current_vertex });

                if (current_vertex->is_exit) {
                        break;
                }

                if (rbtree_get(&visited, (Val){ .p = current_vertex })) {
                        vector_destroy(&path);
                        continue;
                }
                rbtree_set(&visited, (Val){ .p = current_vertex }, NULL);

                adj_vertex = rbtree_min(&current_vertex->adjacent);

                while (adj_vertex) {
                        if (!rbtree_get(&visited, (Val){ .p = adj_vertex->key.p })) {
                                queue_vertex = malloc(sizeof(struct dijkstra_vertex_distance));
                                queue_vertex->vertex = adj_vertex->key.p;
                                queue_vertex->distance = distance + edge_len(current_vertex, queue_vertex->vertex);
                                vector_from_arr(&queue_vertex->path, path.arr, path.len);

                                heap_push(&queue, (Val){.p = queue_vertex});
                        }
                        adj_vertex = rbtree_next(adj_vertex);
                }
                vector_destroy(&path);

        }

        if (current_vertex->is_exit) {
                printf("Path to exit: ");
                for(i = 0; i < path.len - 1; i++){
                        current_vertex = path.arr[i].p;
                        printf("%s (%i, %i) -> ",
                                        current_vertex->name,
                                        current_vertex->x,
                                        current_vertex->y);
                }

                current_vertex = path.arr[i].p;
                printf("%s (%i, %i) EXIT.\n",
                                current_vertex->name,
                                current_vertex->x,
                                current_vertex->y);
                vector_destroy(&path);
        } else {
                puts("No path found.");
        }

        while (!heap_empty(&queue)) {
                heap_pop(&queue, &queue_vertex_val);
                queue_vertex = queue_vertex_val.p;
                path = queue_vertex->path;
                free(queue_vertex);
                vector_destroy(&path);

        }
}


void labyrinth_draw(Labyrinth *labyrinth) {
        int **table;
        int max_x;
        int min_x;
        int max_y;
        int min_y;
        size_t width;
        size_t height;
        size_t i;
        size_t j;
        size_t edge_len;
        RBTreeNode *vertex_iter;
        RBTreeNode *edge_iter;
        struct labyrinth_vertex *vertex;
        struct labyrinth_vertex *second_vertex;

        vertex_iter = rbtree_min(&labyrinth->vertices);
        if (!vertex_iter) {
                puts("**");
                puts("**");
                return;
        }

        vertex = vertex_iter->value;
        max_x = min_x = vertex->x;
        max_y = min_y = vertex->x;
        vertex_iter = rbtree_next(vertex_iter);
        while (vertex_iter) {
                vertex = vertex_iter->value;
                if (vertex->x > max_x) {
                        max_x = vertex->x;
                }
                if (vertex->x < min_x) {
                        min_x = vertex->x;
                }
                if (vertex->y > max_y) {
                        max_y = vertex->y;
                }
                if (vertex->y < min_y) {
                        min_y = vertex->y;
                }
                vertex_iter = rbtree_next(vertex_iter);
        }

        width = max_x - min_x + 1;
        height = max_y - min_y + 1;
        table = calloc(height, sizeof(int *));
        for(i = 0; i < height; i++) {
                table[i] = calloc(width, sizeof(int));
        }

        for (vertex_iter = rbtree_min(&labyrinth->vertices); vertex_iter; vertex_iter = rbtree_next(vertex_iter)){
                vertex = vertex_iter->value;
                table[vertex->y - min_y][vertex->x - min_x] |= LABYRINTH_VERTEX;
                if (vertex->is_exit) {
                        table[vertex->y - min_y][vertex->x - min_x] |= LABYRINTH_EXIT;
                }
                for (edge_iter = rbtree_min(&vertex->adjacent); edge_iter; edge_iter = rbtree_next(edge_iter)){
                        second_vertex = edge_iter->key.p;
                        if (vertex->x == second_vertex->x) {
                                j = vertex->x - min_x;
                                if (vertex->y < second_vertex->y) {
                                        i = vertex->y - min_y;
                                        edge_len = second_vertex->y - min_y;
                                } else {
                                        i = second_vertex->y - min_y;
                                        edge_len = vertex->y - min_y;
                                }

                                for(; i <= edge_len; i++) {
                                        table[i][j] |= VERTICAL_EDGE;
                                }
                        }
                        if (vertex->y == second_vertex->y) {
                                i = vertex->y - min_y;
                                if (vertex->x < second_vertex->x) {
                                        j = vertex->x - min_x;
                                        edge_len = second_vertex->x - min_x;
                                } else {
                                        j = second_vertex->x - min_x;
                                        edge_len = vertex->x - min_x;
                                }

                                for(; j <= edge_len; j++) {
                                        table[i][j] |= HORIZONTAL_EDGE;
                                }
                        }
                }
        }
        putchar('\n');
        for(i = 0; i < width; i++) {
                for(j = 0; j < width; j++) {
                        if (table[i][j] & LABYRINTH_EXIT) {
                                fwrite("\x1b[92m", 1, 5, stdout);
                        }
                        if (table[i][j] & LABYRINTH_VERTEX) {
                                fwrite("\x1b[100m", 1, 6, stdout);
                        }
                        if (table[i][j] & VERTICAL_EDGE && table[i][j] & HORIZONTAL_EDGE) {
                                putchar('+');
                        } else if (table[i][j] & HORIZONTAL_EDGE) {
                                putchar('-');
                        } else if (table[i][j] & VERTICAL_EDGE) {
                                putchar('|');
                        } else {
                                putchar('*');
                        }

                        fwrite("\x1b[0m", 1, 4, stdout);
                }
                putchar('\n');
        }
}

void load_file(Labyrinth *labyrinth, FILE *stream) {
        int res;
        int type;
        struct labyrinth_vertex **vertex;
        struct labyrinth_vertex **vertex2;
        struct point point;
        struct point point2;
        size_t name_size;
        int is_exit;
        char *name;

        while ((type = fgetc(stream)) != EOF) {
                if (type == 'E' || type == 'e') {
                        if (fscanf(stream, "%i", &point.x) == EOF) {
                                break;
                        }
                        if (fscanf(stream, "%i", &point.y) == EOF) {
                                break;
                        }
                        if (fscanf(stream, "%i", &point2.x) == EOF) {
                                break;
                        }
                        if (fscanf(stream, "%i", &point2.y) == EOF) {
                                break;
                        }
                        vertex = (struct labyrinth_vertex **)rbtree_get(&labyrinth->vertices, (Val){.p = &point});
                        if (!vertex) {
                                eprintf("Point with coord (%i, %i) doesn't exist.\n", point.x, point.y);
                                continue;
                        }
                        vertex2 = (struct labyrinth_vertex **)rbtree_get(&labyrinth->vertices, (Val){.p = &point2});
                        if (!vertex2) {
                                eprintf("Point with coord (%i, %i) doesn't exist.\n", point.x, point.y);
                                continue;
                        }
                        labyrinth_add_edge(labyrinth, *vertex, *vertex2);
                        fscanf(stream, "%*[^\n]");
                        fscanf(stream, "%*[\n]");
                } else if (type == 'V' || type == 'v') {
                        if (fscanf(stream, "%i", &point.x) == EOF) {
                                break;
                        }
                        if (fscanf(stream, "%i", &point.y) == EOF) {
                                break;
                        }
                        if (fscanf(stream, "%i", &is_exit) == EOF) {
                                break;
                        }
                        fgetc(stream);
                        name = inputline(stream);
                        labyrinth_add_vertex(labyrinth, point, name, is_exit);
                } else {
                        eprintf("Unknown type %hhx.\n", type);
                }
        }
}


void save_file(Labyrinth *labyrinth, FILE *stream) {
        RBTreeNode *vertex_iter;
        RBTreeNode *edge_iter;
        struct labyrinth_vertex *vertex;
        struct labyrinth_vertex *second_vertex;

        for (vertex_iter = rbtree_min(&labyrinth->vertices); vertex_iter; vertex_iter = rbtree_next(vertex_iter)){
                vertex = vertex_iter->value;
                fprintf(stream, "v %i %i %i %s\n", vertex->x, vertex->y, vertex->is_exit, vertex->name);
        }
        for (vertex_iter = rbtree_min(&labyrinth->vertices); vertex_iter; vertex_iter = rbtree_next(vertex_iter)){
                vertex = vertex_iter->value;
                for (edge_iter = rbtree_min(&vertex->adjacent); edge_iter; edge_iter = rbtree_next(edge_iter)){
                        second_vertex = edge_iter->key.p;
                        if (second_vertex->x > vertex->x || second_vertex->y > vertex->y) {
                                fprintf(stream, "e %i %i %i %i\n", vertex->x, vertex->y, second_vertex->x, second_vertex->y);
                        }
                }
        }
}


void random_maze_dfs(char *maze[], size_t width, size_t height, size_t i, size_t j){
        int direction[][2] = {{0,1}, {0,-1}, {-1,0}, {1,0}};
        int visit_order[] = {0,1,2,3};
        size_t k;
        size_t tmp_i;
        size_t visited_neighbor_count;
        int tmp;

        if(i >= height || j >= width) { return; }

        if(maze[i][j] == '+') { return; }

        visited_neighbor_count = 0;
        for (int k = 0; k < 4; ++k) {
                if (i + direction[k][0] >= height || j + direction[k][1] >= width) { continue; }
                visited_neighbor_count += maze[ i + direction[k][0]][j + direction[k][1]] == '+';
        }
        if (visited_neighbor_count > 1) { return; }

        maze[i][j] = '+';

        for(k = 0; k < 4; k++) {
                tmp = visit_order[k];
                tmp_i = rand() % 4;
                visit_order[k] = visit_order[tmp_i];
                visit_order[tmp_i] = tmp;
        }

        for (int k = 0; k < 4; ++k) {
                random_maze_dfs(maze, width, height,
                                i + direction[visit_order[k]][0],
                                j + direction[visit_order[k]][1]);
        }
}

void random_maze(Labyrinth *labyrinth){
        char **maze;
        size_t width;
        size_t height;
        size_t i;
        size_t j;
        struct point point;
        struct point point2;
        struct labyrinth_vertex **vertex;
        struct labyrinth_vertex **vertex2;
        char name_buf[64];

        width = 10 + rand() % 20;
        height = 10 + rand() % 20;
        /* width = 3; */
        /* height = 3; */
        maze = xcalloc(height, sizeof(char *));
        for (int i = 0; i < height; ++i) {
                maze[i] = xcalloc(width + 1, sizeof(char));
                for (int j = 0; j < width; ++j) {
                        maze[i][j] = '*';
                }
        }
        random_maze_dfs(maze, width, height, 0, 0);
        for (int i = 0; i < height; ++i) {
            puts(maze[i]);
        }

        labyrinth_add_vertex(labyrinth, (struct point){.x = 0, .y = 0}, "exit", 1);
        for (i = 0; i < height; i++) {
                for (j = 0; j < width; j++) {
                        if (maze[i][j] == '+' &&
                                        (
                                         ((i < height - 1) && maze[i + 1][j] == '+') || 
                                         ((0 < i) && maze[i - 1][j] == '+')
                                        ) && 
                                         (
                                          ((j < width - 1) && maze[i][j + 1] == '+') || 
                                         ((0 < j) && maze[i][j - 1] == '+')
                                         )
                                        ) {
                                point.y = i;
                                point.x = j;
                                snprintf(name_buf, 64, "(%i, %i)", point.x, point.y);
                                /* printf("v %s\n", name_buf); */
                                labyrinth_add_vertex(labyrinth, point, name_buf, 0);
                        }

                }
        }
        for (i = 0; i < height; i++) {
                for (j = 0; j < width; j++) {
                        if (maze[i][j] == '*') {
                                continue;
                        }
                        point.y = i;
                        point.x = j;
                        while(j < width && maze[i][j] == '+') {
                                j++;
                        }
                        if (j >= width) {
                                j = width - 1;
                        }
                        while (maze[i][j] != '+') {
                                j--;
                        }

                        point2.y = i;
                        point2.x = j;
                        if (point2.x == point.x && point2.y == point.y) {
                                continue;
                        }
                        vertex = (struct labyrinth_vertex **)rbtree_get(&labyrinth->vertices, (Val) {.p = &point});

                        if (!vertex) {
                                snprintf(name_buf, 64, "(%i, %i)", point.x, point.y);
                                labyrinth_add_vertex(labyrinth, point, name_buf, 0);
                                vertex = (struct labyrinth_vertex **)rbtree_get(&labyrinth->vertices, (Val) {.p = &point});
                        }

                        vertex2 = (struct labyrinth_vertex **)rbtree_get(&labyrinth->vertices, (Val) {.p = &point2});
                        if (!vertex2) {
                                snprintf(name_buf, 64, "(%i, %i)", point2.x, point2.y);
                                labyrinth_add_vertex(labyrinth, point2, name_buf, 0);
                                vertex2 = (struct labyrinth_vertex **)rbtree_get(&labyrinth->vertices, (Val) {.p = &point2});
                        }

                        if (*vertex == *vertex2) {
                                continue;
                        }
                        labyrinth_add_edge(labyrinth, *vertex, *vertex2);
                }
        }
        for (j = 0; j < width; j++) {
                for (i = 0; i < height; i++) {
                        if (maze[i][j] == '*') {
                                continue;
                        }
                        point.y = i;
                        point.x = j;
                        while(i < height && maze[i][j] == '+') {
                                i++;
                        }
                        if (i >= height) {
                                i = height - 1;
                        }
                        while (maze[i][j] != '+') {
                                i--;
                        }

                        point2.y = i;
                        point2.x = j;
                        if (point2.x == point.x && point2.y == point.y) {
                                continue;
                        }
                        vertex = (struct labyrinth_vertex **)rbtree_get(&labyrinth->vertices, (Val) {.p = &point});

                        if (!vertex) {
                                snprintf(name_buf, 64, "(%i, %i)", point.x, point.y);
                                labyrinth_add_vertex(labyrinth, point, name_buf, 0);
                                vertex = (struct labyrinth_vertex **)rbtree_get(&labyrinth->vertices, (Val) {.p = &point});
                        }

                        vertex2 = (struct labyrinth_vertex **)rbtree_get(&labyrinth->vertices, (Val) {.p = &point2});
                        if (!vertex2) {
                                snprintf(name_buf, 64, "(%i, %i)", point2.x, point2.y);
                                labyrinth_add_vertex(labyrinth, point2, name_buf, 0);
                                vertex2 = (struct labyrinth_vertex **)rbtree_get(&labyrinth->vertices, (Val) {.p = &point2});
                        }
                        if (*vertex == *vertex2) {
                                continue;
                        }
                        labyrinth_add_edge(labyrinth, *vertex, *vertex2);
                }
        }
        for (i = 0; i < height; i++) {
                free(maze[i]);
        }
        free(maze);
}

void menu() {
        puts("1. Add point.");
        puts("2. Add corridor.");
        puts("3. Toggle exit.");
        puts("4. Delete point.");
        puts("5. Delete corridor.");
        puts("6. Find exit.");
        puts("7. Print labyrinth.");
        puts("8. Load from file.");
        puts("9. Save to file.");
        puts("10. Random labyrinth.");
        puts("11. Exit.");
}

int main(int argc, char *argv[]) {
        Labyrinth labyrinth;
        struct labyrinth_vertex *vertex;
        struct labyrinth_vertex *vertex2;
        struct point point;
        char *name;
        int is_exit;
        size_t i;
        FILE *file;
        int choice;

        setvbuf(stdin, NULL, _IONBF, 0);
        setvbuf(stdout, NULL, _IONBF, 0);
        /* srand(time(0)); */
        labyrinth_init(&labyrinth);

        for(i = 1; i < argc; i++) {

                file = fopen(argv[i], "r");
                if (file) {
                        load_file(&labyrinth, file);
                        fclose(file);
                } else {
                        eprintf("Could not open file %s.\n", argv[i]);
                }

        }
        
        for(;;) {
                menu();
                printf("> ");
                choice = input_int();
                scanf("%*[^\n]");
                scanf("%*1[\n]");
                switch(choice) {
                        case 1:
                                printf("x = ");
                                point.x = input_int();
                                printf("y = ");
                                point.y = input_int();
                                scanf("%*1[\n]");
                                printf("name: ");
                                name = inputline(stdin);
                                printf("is_exit = ");
                                is_exit = input_int();
                                labyrinth_add_vertex(&labyrinth, point, name, is_exit);
                                break;

                        case 2:
                                vertex = input_vertex(&labyrinth);
                                if (!vertex) { break; }
                                vertex2 = input_vertex(&labyrinth);
                                if (!vertex2) { break; }
                                labyrinth_add_edge(&labyrinth, vertex, vertex2);
                                break;
                        case 3:
                                vertex = input_vertex(&labyrinth);
                                if (!vertex) { break; }
                                printf("is_exit = ");
                                is_exit = input_int();
                                vertex->is_exit = (bool)is_exit;
                                break;
                        case 4:
                                vertex = input_vertex(&labyrinth);
                                if (!vertex) { break; }
                                labyrinth_remove_vertex(&labyrinth, vertex);
                                break;
                        case 5:
                                vertex = input_vertex(&labyrinth);
                                if (!vertex) { break; }
                                vertex2 = input_vertex(&labyrinth);
                                if (!vertex2) { break; }
                                labyrinth_remove_vertex(&labyrinth, vertex2);
                                break;
                        case 6:
                                vertex = input_vertex(&labyrinth);
                                if (!vertex) { break; }
                                labyrinth_find_exit(&labyrinth, vertex);
                                break;
                        case 7:
                                labyrinth_draw(&labyrinth);
                                break;
                        case 8:
                                printf("name: ");
                                name = inputline(stdin);

                                file = fopen(name, "r");
                                if (file) {
                                        load_file(&labyrinth, file);
                                        fclose(file);
                                } else {
                                        eprintf("Could not open file %s.\n", name);
                                }
                                free(name);
                                break;
                        case 9:
                                printf("name: ");
                                name = inputline(stdin);

                                file = fopen(name, "w");
                                if (file) {
                                        save_file(&labyrinth, file);
                                        fclose(file);
                                } else {
                                        eprintf("Could not open file %s.\n", name);
                                }
                                free(name);
                                break;
                        case 10:
                                labyrinth_destroy(&labyrinth);
                                labyrinth_init(&labyrinth);
                                random_maze(&labyrinth);
                                break;
                        case 11:
                                labyrinth_destroy(&labyrinth);
                                exit(0);
                }
        }
}
