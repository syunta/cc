#include "9cc.h"

Type *new_type(TypeKind ty) {
    Type *type = calloc(1, sizeof(Type));
    type->ty = ty;
    switch (ty) {
        case INT:
            type->size = 8;
            break;
        case PTR:
            type->size = 8;
            break;
    }
    return type;
}

Type *new_pointer_to(Type *type) {
    Type *ptr = new_type(PTR);
    ptr->ptr_to = type;
    return ptr;
}

Type *deref_type(Node *node) {
    if (node->kind == ND_DEREF) {
        return deref_type(node->lhs);
    }
    return node->type;
}
