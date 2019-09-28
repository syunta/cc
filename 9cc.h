#define _GNU_SOURCE
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Tokenizer

typedef enum {
    TK_RESERVED,
    TK_IDENT,
    TK_NUM,
    TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
    int len;
};

// Types

typedef enum {
    INT,
    PTR
} TypeKind;

typedef struct Type Type;
struct Type {
    TypeKind ty;
    int size;
    Type *ptr_to;
};

// Parser

typedef enum {
    ND_ADD, // int + int -> int
    ND_SUB, // int - int -> int
    ND_MUL, // int * int -> int
    ND_DIV, // int / int -> int
    ND_PTR_ADD, // pointer + num -> pointer, num + pointer -> pointer
    ND_PTR_SUB, // pointer - num -> pointer, num - pointer -> pointer
    ND_EQ, // ==
    ND_NE, // !=
    ND_LT, // <
    ND_LE, // <=
    ND_ASSIGN, // =
    ND_DEFINE, // function define
    ND_CALL, // function call
    ND_BLOCK, // { ... }
    ND_RETURN, // return
    ND_IF, // if
    ND_LOOP, // while
    ND_LVAR, // local variable, function param
    ND_NUM, // Integer
    ND_ADDR, // pointer
    ND_DEREF, // dereference pointer
    ND_LDECLARE, // local variable declaration
} NodeKind;

typedef struct Node Node;
struct Node {
    NodeKind kind;
    Type *type; // return value type
    Node *lhs;
    Node *rhs;

    Node *next;

    // if
    Node *pred;
    Node *con;
    Node *alt;

    // while, for
    Node *init;
    Node *end;

    // an expression or block
    Node *body;

    // function define
    Node *params;

    // function call
    Node *args;

    int val; // for ND_NUM
    int offset; // for ND_LVAR, ND_DEFINE
    Token *tok; // for ND_DEFINE, ND_CALL
};

typedef struct LVar LVar;
struct LVar {
    LVar *next;
    Type *type;
    char *name;
    int len;
    int offset;
};

typedef struct GEnv GEnv;
struct GEnv {
    GEnv *next;
    Type *type; // return value type
    char *name;
    int len;
};

// Tokenizer

extern Token *token;
extern char *user_input;

void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);

bool consume(char *op);
Token *consume_ident();
void expect(char *op);
int expect_number();
Token *expect_ident();
bool peek(int n, char *p);

Token *tokenize();
bool at_eof();

// Parser

extern Node *code;
extern LVar *locals;
extern GEnv *globals;

void program();

// Generator

extern int label_count;
void gen_globals(Node *node);

// Types

Type *new_type(TypeKind type);
Type *new_pointer_to(Type *type);
Type *deref_type(Node *node);
