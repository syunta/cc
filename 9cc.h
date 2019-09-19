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

// Parser

typedef enum {
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_EQ, // ==
    ND_NE, // !=
    ND_LT, // <
    ND_LE, // <=
    ND_ASSIGN, // =
    ND_BLOCK, // { ... }
    ND_RETURN, // return
    ND_IF, // if
    ND_LOOP, // while
    ND_LVAR, // local variable
    ND_NUM, // Integer
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
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

    int val; // for ND_NUM
    int offset; // for ND_LVAR
};

typedef struct LVar LVar;

struct LVar {
    LVar *next;
    char *name;
    int len;
    int offset;
};

// Tokenizer

extern Token *token;
extern char *user_input;

void error(char *fmt, ...);

bool consume(char *op);
Token *consume_ident();
void expect(char *op);
int expect_number();
bool peek(int n, char *p);

Token *tokenize();
bool at_eof();

// Parser

extern Node *code;
extern LVar *locals;

void initialize_locals();
void program();

// Generator

extern int label_count;
void gen(Node *node);
void g(Node *node);
