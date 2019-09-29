#include "9cc.h"

Node *code;
LVar *locals;
GEnv *globals;

// Environment

void initialize_locals() {
    locals = calloc(1, sizeof(LVar));
    locals->offset = 0;
}

void initialize_globals() {
    globals = calloc(1, sizeof(GEnv));
    globals->name = NULL;
}

LVar *extend_locals(Token *tok, Type *type) {
    LVar *lvar = calloc(1, sizeof(LVar));
    lvar->next = locals;
    lvar->type = type;
    lvar->name = tok->str;
    lvar->len = tok->len;
    lvar->offset = locals->offset + type->size;

    locals = lvar;

    return lvar;
}

GEnv *extend_globals(Token *tok, Type *type) {
    GEnv *genv = calloc(1, sizeof(GEnv));
    genv->next = globals;
    genv->type = type;
    genv->name = tok->str;
    genv->len = tok->len;

    globals = genv;

    return genv;
}

LVar *find_lvar(Token *tok) {
    for (LVar *var = locals; var; var = var->next) {
        if (var->len == tok->len && !strncmp(tok->str, var->name, var->len)) {
            return var;
        }
    }
    error_at(token->str, "変数が未定義です: %s", strndup(tok->str, tok->len));
}

GEnv *find_genv(Token *tok) {
    for (GEnv *env = globals; env; env = env->next) {
        if (env->len == tok->len && !strncmp(tok->str, env->name, env->len)) {
            return env;
        }
    }
    error_at(token->str, "関数が未定義です: %s", strndup(tok->str, tok->len));
}

// AST Node

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_add(Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->lhs = lhs;
    node->rhs = rhs;
    node->kind = ND_ADD;
    if (lhs->type->ty == INT && rhs->type->ty == INT) {
        node->type = new_type(INT);
    }
    if (is_ref(lhs->type) == PTR && rhs->type->ty == INT) {
        node->kind = ND_PTR_ADD;
        node->type = lhs->type;
    }
    if (lhs->type->ty == INT && is_ref(rhs->type)) {
        // swap node for codegen easily
        node->lhs = rhs;
        node->rhs = lhs;
        node->kind = ND_PTR_ADD;
        node->type = rhs->type;
    }
    return node;
}

Node *new_sub(Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->lhs = lhs;
    node->rhs = rhs;
    node->kind = ND_SUB;
    if (lhs->type->ty == INT && rhs->type->ty == INT) {
        node->type = new_type(INT);
    }
    if (is_ref(lhs->type) == PTR && rhs->type->ty == INT) {
        node->kind = ND_PTR_SUB;
        node->type = lhs->type;
    }
    if (lhs->type->ty == INT && is_ref(rhs->type)) {
        // swap node for codegen easily
        node->lhs = rhs;
        node->rhs = lhs;
        node->kind = ND_PTR_SUB;
        node->type = rhs->type;
    }
    return node;
}

Node *new_mul(Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_MUL;
    node->type = new_type(INT);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_div(Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_DIV;
    node->type = new_type(INT);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_addr(Node *lhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_ADDR;
    node->type = new_pointer_to(lhs->type);
    node->lhs = lhs;
    return node;
}

Node *new_deref(Node *lhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_DEREF;
    node->type = lhs->type->ptr_to;
    node->lhs = lhs;
    return node;
}

Node *new_if(Node *pred, Node *con, Node *alt) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_IF;
    node->pred = pred;
    node->con = con;
    node->alt = alt;
    return node;
}

Node *new_loop(Node *init, Node *pred, Node *end, Node *body) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LOOP;
    node->init = init;
    node->pred = pred;
    node->end = end;
    node->body = body;
    return node;
}

Node *new_block(Node *body) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    node->body = body;
    return node;
}

Node *new_definition(Token *tok, Node* params, Node *body, size_t offset) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_DEFINE;
    node->tok = tok;
    node->params = params;
    node->body = body;
    node->offset = offset;
    return node;
}

Node *new_call(Token *tok, Node *args) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_CALL;
    GEnv *func = find_genv(tok);
    node->type = func->type;
    node->tok = tok;
    node->args = args;
    return node;
}

Node *new_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->type = new_type(INT);
    node->val = val;
    return node;
}

Node *new_lvar(LVar *lvar) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    node->type = lvar->type;
    node->offset = lvar->offset;
    return node;
}

Node *new_ldeclare() {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LDECLARE;
    return node;
}

Node *new_sizeof(Node *n) {
    Node *node = new_num(n->type->size);
    return node;
}

Node *implicit_conv(Node *node) {
    if (node->type->ty == ARRAY) {
        if (node->kind == ND_LVAR) {
            node->kind = ND_ADDR;
            Node *n = calloc(1, sizeof(Node));
            n->kind = ND_LVAR;
            n->type = node->type->ptr_to;
            n->offset = node->offset - n->type->size * (node->type->array_len - 1);
            node->lhs = n;
        }
    }
    return node;
}

// Parser

Node *define();
Node *params();
Node *block();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();
Node *args();

void program() {
    initialize_globals();

    Node head;
    head.next = NULL;
    Node *cur = &head;
    while (!at_eof()) {
        Node *s = define();
        cur->next = s;
        cur = s;
    }
    code = head.next;
}

Node* define() {
    initialize_locals();

    expect("int");
    Type *return_type = new_type(INT);
    Token *name = expect_ident();
    expect("(");
    Node* ps = params();
    expect(")");
    extend_globals(name, return_type);
    Node *body = block();

    size_t max_offset = locals->offset;
    return new_definition(name, ps, body, max_offset);
}

Node* params() {
    Node head;
    head.next = NULL;
    Node *cur = &head;

    while (!peek(1, ")")) {
        if (cur != &head) {
            expect(",");
        }
        expect("int");
        Type *t = new_type(INT);
        while (consume("*")) {
            t = new_pointer_to(t);
        }
        Token *tok = expect_ident();
        LVar *lvar = extend_locals(tok, t);
        // set value as local variable
        Node *p = new_lvar(lvar);
        cur->next = p;
        cur = p;
    }
    return head.next;
}

Node *block() {
    Node *node = NULL;
    if (consume("{")) {
        Node head;
        head.next = NULL;
        Node *cur = &head;

        while (!consume("}")) {
            Node *s = stmt();
            cur->next = s;
            cur = s;
        }

        node = new_block(head.next);
    }
    return node;
}

Node *stmt() {
    Node *node;

    Node *b = block();
    if (b) {
        node = b;
        return node;
    }

    if (consume("return")) {
        node = new_node(ND_RETURN, expr(), NULL);
        expect(";");
        return node;
    }

    if (consume("if")) {
        expect("(");
        Node *predicate = expr();
        expect(")");
        Node *consequent = stmt();
        Node *alternative = NULL;
        if (consume("else")) {
            alternative = stmt();
        }
        node = new_if(predicate, consequent, alternative);
        return node;
    }

    if (consume("while")) {
        expect("(");
        Node *predicate = expr();
        expect(")");
        Node *body = stmt();
        node = new_loop(NULL, predicate, NULL, body);
        return node;
    }

    if (consume("for")) {
        expect("(");
        Node *init = NULL;
        if (!peek(1, ";")) {
            init = expr();
        }
        expect(";");
        Node *predicate = expr();
        expect(";");
        Node *end = NULL;
        if (!peek(1, ")")) {
            end = expr();
        }
        expect(")");
        Node *body = stmt();
        node = new_loop(init, predicate, end, body);
        return node;
    }

    if (consume("int")) {
        Type *t = new_type(INT);
        while (consume("*")) {
            t = new_pointer_to(t);
        }
        Token* tok = expect_ident();

        if(consume("[")) {
            int len = expect_number();
            t = new_array_type(len, t);
            expect("]");
        }

        extend_locals(tok, t);
        node = new_ldeclare();
        expect(";");
        return node;
    }

    node = expr();
    expect(";");
    return node;
}

Node *expr() {
    return assign();
}

Node *assign() {
    Node *node = equality();
    if (consume("=")) {
        node = new_node(ND_ASSIGN, node, assign());
    }
    return node;
}

Node *equality() {
    Node *node = relational();

    for (;;) {
        if (consume("=="))
            node = new_node(ND_EQ, node, relational());
        else if (consume("!="))
            node = new_node(ND_NE, node, relational());
        else
            return node;
    }
}

Node *relational() {
    Node *node = add();

    for (;;) {
        if (consume("<"))
            node = new_node(ND_LT, node, add());
        else if (consume("<="))
            node = new_node(ND_LE, node, add());
        else if (consume(">"))
            node = new_node(ND_LT, add(), node);
        else if (consume(">="))
            node = new_node(ND_LE, add(), node);
        else
            return node;
    }
}

Node *add() {
    Node *node = mul();

    for (;;) {
        if (consume("+"))
            node = new_add(node, mul());
        else if (consume("-"))
            node = new_sub(node, mul());
        else
            return node;
    }
}

Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume("*"))
            node = new_mul(node, unary());
        else if (consume("/"))
            node = new_div(node, unary());
        else
            return node;
    }
}

// if primary() returns array, parser converts array to reference of array implicitly.
Node *unary() {
    if (consume("sizeof"))
        return new_sizeof(unary());
    if (consume("+"))
        return implicit_conv(primary());
    if (consume("-"))
        return new_sub(new_num(0), implicit_conv(primary()));
    if (consume("*"))
        return new_deref(unary());
    if (consume("&"))
        return new_addr(unary());
    return implicit_conv(primary());
}

Node *primary() {
    Token *tok = consume_ident();
    if (tok) {
        if (consume("(")) {
            Node *arguments = args();
            consume(")");
            Node *node = new_call(tok, arguments);
            return node;
        }

        Node *node;
        LVar *lvar = find_lvar(tok);
        node = new_lvar(lvar);
        return node;
    }

    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    return new_num(expect_number());
}

Node *args() {
    Node head;
    head.next = NULL;
    Node *cur = &head;
    while (!peek(1, ")")) {
        if (cur != &head) {
            expect(",");
        }
        Node *a = expr();
        cur->next = a;
        cur = a;
    }
    return head.next;
}
