#include "struct.h"

hash_t idhash(Val v) {
        return v.i;
}

int valcmp(Val a, Val b) {
        if (a.i < b.i) {
                return -1;
        }
        if (a.i > b.i) {
                return 1;
        }
        return 0;
}

