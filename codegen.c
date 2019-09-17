#include "9cc.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int label_count = 0;

void gen_lval(Node *node) {
    if (node->kind != ND_LVAR) {
        error("代入の左辺値が変数ではありません");
    }

    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
}

void gen_if(Node *node) {
    gen(node->pred);
    printf("  pop rax\n");
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
    printf(".Lbegin%d:\n", c);
    gen(node->pred);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  .Lend%d\n", c);
    gen(node->body);
    printf("  jmp .Lbegin%d\n", c);
    printf(".Lend%d:\n", c);
}

void gen(Node *node) {
    if (!node) return;
    switch (node->kind) {
        case ND_NUM:
            printf("  push %d\n", node->val);
            return;
        case ND_LVAR:
            gen_lval(node);
            printf("  pop rax\n");
            printf("  mov rax, [rax]\n");
            printf("  push rax\n");
            return;
        case ND_ASSIGN:
            gen_lval(node->lhs);
            gen(node->rhs);
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  mov [rax], rdi\n");
            printf("  push rdi\n");
            return;
        case ND_RETURN:
            gen(node->lhs);
            printf("  pop rax\n");
            printf("  mov rsp, rbp\n");
            printf("  pop rbp\n");
            printf("  ret\n");
            return;
        case ND_IF:
            gen_if(node);
            return;
        case ND_LOOP:
            gen_loop(node);
            break;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind) {
        case ND_ADD:
            printf("  add rax, rdi\n");
            break;
        case ND_SUB:
            printf("  sub rax, rdi\n");
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

    printf("  push rax\n");
}
