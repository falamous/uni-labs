#include "struct.h"

void heap_init(Heap *heap, int (*cmpfunc)(Val, Val)) {
        heap->cmpfunc = cmpfunc;
        vector_init(&heap->heap, 0);
}

int heap_top(Heap *heap, Val *val) {
        if (heap->heap.len == 0) {
                return 1;
        }
        *val = heap->heap.arr[0];
        return 0;
}

static size_t parent(size_t n) {
        return (n - 1)/2;
}
static size_t left_child(size_t n) {
        return n * 2 + 1;
}
static size_t right_child(size_t n) {
        return n * 2 + 2;
}

static void heapifydown(Heap *heap, size_t i) {
        Val tmp;
        size_t max_child;
        /* size_t left; */
        size_t right;

        for(;;) {
                max_child = left_child(i);
                right = right_child(i);

                if (
                                max_child < heap->heap.len &&
                                right < heap->heap.len && 
                                heap->cmpfunc(heap->heap.arr[max_child], heap->heap.arr[right]) > 0) {
                        max_child = right;
                }

                if (max_child < heap->heap.len &&
                                heap->cmpfunc(heap->heap.arr[i], heap->heap.arr[max_child]) > 0) {
                        tmp = heap->heap.arr[i];
                        heap->heap.arr[i] = heap->heap.arr[max_child];
                        heap->heap.arr[max_child] = tmp;
                } else {
                        break;
                }
        }
}

static void heapifyup(Heap *heap, size_t i) {
        Val tmp;

        i = heap->heap.len - 1;
        while (
                        i <= heap->heap.len &&
                        parent(i) <= heap->heap.len &&
                        heap->cmpfunc(heap->heap.arr[parent(i)], heap->heap.arr[i]) > 0) {
                tmp = heap->heap.arr[i];
                heap->heap.arr[i] = heap->heap.arr[parent(i)];
                heap->heap.arr[parent(i)] = tmp;
                i = parent(i);
        }
}

int heap_pop(Heap *heap, Val *val) {
        if (heap->heap.len == 0) {
                return 1;
        }
        *val = heap->heap.arr[0];


        heap->heap.arr[0] = heap->heap.arr[heap->heap.len - 1];
        vector_pop(&heap->heap, NULL);
        heapifydown(heap, 0);

        return 0;
}


void heap_push(Heap *heap, Val val) {
        Val tmp;
        vector_pb(&heap->heap, val);
        heapifyup(heap, heap->heap.len - 1);
}

bool heap_empty(Heap *heap) {
        return heap->heap.len == 0;
}
