#include "9cc.h"

Type *new_type(TypeKind ty) {
    Type *type = calloc(1, sizeof(Type));
    type->ty = ty;
    switch (ty) {
        case INT:
            type->size = 4;
            break;
        case PTR:
            type->size = 8;
            break;
    }
    return type;
}

Type *new_array_type(size_t len, Type *type) {
    Type *t = calloc(1, sizeof(Type));
    t->ty = ARRAY;
    t->array_len = len;
    t->size = type->size * len; // int a[10] -> 4 * 10
    t->ptr_to = type;
    return t;
}

Type *new_pointer_to(Type *type) {
    Type *ptr = new_type(PTR);
    ptr->ptr_to = type;
    return ptr;
}

bool is_ref(Type *type) {
    if (type->ty == PTR || type->ty == ARRAY) {
        return true;
    }
    return false;
}
