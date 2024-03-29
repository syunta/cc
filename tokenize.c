#include "9cc.h"

Token *token;
char *user_input;

void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, "");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool consume(char *op) {
    if (token->kind != TK_RESERVED || strlen(op) != token->len || strncmp(token->str, op, token->len))
        return false;
    token = token->next;
    return true;
}

Token *consume_ident() {
    if (token->kind != TK_IDENT)
        return NULL;
    Token *t = token;
    token = token->next;
    return t;
}

Type *consume_type() {
    Type *type = NULL;
    if (consume("int")) {
        type = new_type(INT);
    }
    if (consume("char")) {
        type = new_type(CHAR);
    }
    return type;
}

void expect(char *op) {
    if (token->kind != TK_RESERVED || strlen(op) != token->len || strncmp(token->str, op, token->len))
        error_at(token->str, "'%s'ではありません", op);
    token = token->next;
}

int expect_number() {
    if (token->kind != TK_NUM)
        error_at(token->str, "数ではありません");
    int val = token->val;
    token = token->next;
    return val;
}

Token *expect_ident() {
    if (token->kind != TK_IDENT)
        error_at(token->str, "識別子ではありません");
    Token *t = token;
    token = token->next;
    return t;
}

bool peek(int n, char *p) {
    Token *t = token;
    for (int i = 0; i < n - 1; i++) {
        t = t->next;
    }
    return strncmp(t->str, p, t->len) == 0;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

bool startswith(char *p, char *q) {
    return memcmp(p, q, strlen(q)) == 0;
}

bool is_alpha(char p) {
    return ('a' <= p && p <= 'z') || ('A' <= p && p <= 'Z') || p == '_';
}

bool is_alnum(char p) {
    return ('a' <= p && p <= 'z') ||
        ('A' <= p && p <= 'Z') ||
        ('0' <= p && p <= '9') ||
        p == '_';
}

char *is_keyword(char *p) {
    char *keywords[] = { "return", "if", "else", "while", "for", "int", "char", "sizeof" };

    for (int i = 0; i < sizeof(keywords) / sizeof(*keywords); i++) {
        int len = strlen(keywords[i]);
        if (strncmp(p, keywords[i], len) == 0 && !is_alnum(p[len])) {
            return keywords[i];
        }
    }
    return NULL;
}

Token *tokenize() {
    char *p = user_input;
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") || startswith(p, ">=")) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        char *kw = is_keyword(p);
        if (kw) {
            int len = strlen(kw);
            cur = new_token(TK_RESERVED, cur, kw, len);
            p += len;
            continue;
        }

        if (strchr("+-*/()[]{}<>=;,&", *p)) {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        if (is_alpha(*p)) {
            char *q = p++;
            while(is_alpha(*p)) {
                p++;
            }
            cur = new_token(TK_IDENT, cur, q, p - q);
            continue;
        }

        if (strchr("\"", *p)) {
            p++;
            char *q = p++;
            while(!strchr("\"", *p)) {
                p++;
            }
            p++;
            cur = new_token(TK_STRING, cur, q, p - q - 1);
            continue;
        }

        error_at(token->str, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}
