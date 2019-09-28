#include "9cc.h"

int label_count = 0;

char *reg64[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
char *reg32[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};

char *reg_of(int size, int idx) {
    if (size == 4)
        return reg32[idx];
    if (size == 8)
        return reg64[idx];
}

void gen_statements();
void gen();
void ggen();

void gen_save_from(char* reg) {
    printf("  mov [rax], %s\n", reg);
}

void gen_load(Node *node) {
    if (node->type->size == 4)
        printf("  movsxd rax, dword ptr [rax]\n");
    if (node->type->size == 8)
        printf("  mov rax, [rax]\n");
}

void gen_lval(Node *node) {
    switch (node->kind) {
        case ND_LVAR:
            printf("  mov rax, rbp\n");
            printf("  sub rax, %d\n", node->offset);
            return;
        case ND_DEREF:
            gen(node->lhs);
            return;
    }
    error("代入の左辺値が変数ではありません");
}

void gen_if(Node *node) {
    gen(node->pred);
    printf("  cmp rax, 0\n");
    int c = label_count;
    label_count++;
    if (node->alt) {
        printf("  je  .Lelse%d\n", c);
        gen(node->con);
        printf("  jmp .Lend%d\n", c);
        printf(".Lelse%d:\n", c);
        gen(node->alt);
        printf(".Lend%d:\n", c);
    } else {
        printf("  je  .Lend%d\n", c);
        gen(node->con);
        printf(".Lend%d:\n", c);
    }
}

void gen_loop(Node *node) {
    int c = label_count;
    label_count++;
    gen(node->init);
    printf(".Lbegin%d:\n", c);
    gen(node->pred);
    printf("  cmp rax, 0\n");
    printf("  je  .Lend%d\n", c);
    gen(node->body);
    gen(node->end);
    printf("  jmp .Lbegin%d\n", c);
    printf(".Lend%d:\n", c);
}

void gen_args(Node *node) {
    Node *cur = node;
    for (int i = 0; cur; i++) {
        gen(cur);
        printf("  mov %s, rax\n", reg64[i]);
        cur = cur->next;
    }
}

void gen_params(Node *node) {
    Node *cur = node;
    for (int i = 0; cur; i++) {
        gen_lval(cur);
        gen_save_from(reg_of(cur->type->size, i));
        cur = cur->next;
    }
}

void gen(Node *node) {
    if (!node) return;
    switch (node->kind) {
        case ND_NUM:
            printf("  mov rax, %d\n", node->val);
            return;
        case ND_LVAR:
            gen_lval(node);
            gen_load(node);
            return;
        case ND_ASSIGN:
            gen_lval(node->lhs);
            printf("  push rax\n");
            gen(node->rhs);
            printf("  mov rdi, rax\n");
            printf("  pop rax\n");
            gen_save_from(reg_of(node->lhs->type->size, 0));
            printf("  mov rax, rdi\n");
            return;
        case ND_RETURN:
            gen(node->lhs);
            printf("  mov rsp, rbp\n");
            printf("  pop rbp\n");
            printf("  ret\n");
            return;
        case ND_IF:
            gen_if(node);
            return;
        case ND_LOOP:
            gen_loop(node);
            return;
        case ND_BLOCK:
            gen_statements(node->body);
            return;
        case ND_CALL:
            gen_args(node->args);
            printf("  call %s\n", strndup(node->tok->str, node->tok->len));
            return;
        case ND_ADDR:
            gen_lval(node->lhs);
            return;
        case ND_DEREF:
            gen(node->lhs);
            gen_load(node);
            return;
        case ND_LDECLARE:
            return;
    }

    gen(node->lhs);
    printf("  push rax\n");
    gen(node->rhs);
    printf("  mov rdi, rax\n");
    printf("  pop rax\n");

    switch (node->kind) {
        case ND_ADD:
            printf("  add rax, rdi\n");
            break;
        case ND_SUB:
            printf("  sub rax, rdi\n");
            break;
        case ND_PTR_ADD:
            printf("  imul rdi, %d\n", node->lhs->type->ptr_to->size);
            printf("  sub rax, rdi\n");
            break;
        case ND_PTR_SUB:
            printf("  imul rdi, %d\n", node->lhs->type->ptr_to->size);
            printf("  add rax, rdi\n");
            break;
        case ND_MUL:
            printf("  imul rax, rdi\n");
            break;
        case ND_DIV:
            printf("  cqo\n");
            printf("  idiv rdi\n");
            break;
        case ND_EQ:
            printf("  cmp rax, rdi\n");
            printf("  sete al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_NE:
            printf("  cmp rax, rdi\n");
            printf("  setne al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_LT:
            printf("  cmp rax, rdi\n");
            printf("  setl al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_LE:
            printf("  cmp rax, rdi\n");
            printf("  setle al\n");
            printf("  movzb rax, al\n");
            break;
    }
}

void ggen(Node *node) {
    if (!node) return;
    switch (node->kind) {
        case ND_DEFINE:
            printf("%s:\n", strndup(node->tok->str, node->tok->len));
            printf("  push rbp\n");
            printf("  mov rbp, rsp\n");
            printf("  sub rsp, %d\n", node->offset);

            gen_params(node->params);
            gen(node->body);

            printf("  mov rsp, rbp\n");
            printf("  pop rbp\n");
            printf("  ret\n");
            return;
    }
}

void gen_statements(Node *node) {
    Node *cur = node;
    while (cur) {
        gen(cur);
        cur = cur->next;
    }
}

void gen_globals(Node *node) {
    Node *cur = node;
    while (cur) {
        ggen(cur);
        cur = cur->next;
    }
}
