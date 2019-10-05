#include "9cc.h"

Node *code;
Env *locals;
Env *globals;

// Environment

void initialize_locals() {
    locals = calloc(1, sizeof(Env));
    locals->kind = None;
}

void initialize_globals() {
    globals = calloc(1, sizeof(Env));
    globals->kind = None;
}

Env *extend_locals(Token *tok, Type *type) {
    Env *lvar = calloc(1, sizeof(Env));
    lvar->next = locals;
    lvar->kind = LVar;
    lvar->type = type;
    lvar->tok = tok;
    lvar->offset = locals->offset + type->size;

    locals = lvar;

    return lvar;
}

Env *extend_globals(EnvKind kind, Token *tok, Type *type) {
    Env *genv = calloc(1, sizeof(Env));
    genv->kind = kind;
    genv->next = globals;
    genv->type = type;
    genv->tok = tok;

    if (kind == GVar)
        genv->offset = globals->offset + type->size;
    else
        genv->offset = globals->offset;

    globals = genv;

    return genv;
}

Env *find_func(Token *tok) {
    for (Env *env = globals; env; env = env->next) {
        if (env->kind == FUNCTION && env->tok->len == tok->len && !strncmp(tok->str, env->tok->str, env->tok->len)) {
            return env;
        }
    }
    error_at(token->str, "関数が未定義です: %s", strndup(tok->str, tok->len));
}

Env *find_var(Token *tok) {
    for (Env *var = locals; var; var = var->next) {
        if (var->kind == LVar && var->tok->len == tok->len && !strncmp(tok->str, var->tok->str, var->tok->len)) {
            return var;
        }
    }
    for (Env *var = globals; var; var = var->next) {
        if (var->kind == GVar && var->tok->len == tok->len && !strncmp(tok->str, var->tok->str, var->tok->len)) {
            return var;
        }
    }
    error_at(token->str, "変数が未定義です: %s", strndup(tok->str, tok->len));
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
    if (is_int(lhs->type) && is_int(rhs->type)) {
        node->type = new_type(INT);
    }
    if (is_ref(lhs->type) && is_int(rhs->type)) {
        node->kind = ND_PTR_ADD;
        node->type = lhs->type;
    }
    if (is_int(lhs->type) && is_ref(rhs->type)) {
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
    if (is_int(lhs->type) && is_int(rhs->type)) {
        node->type = new_type(INT);
    }
    if (is_ref(lhs->type) && is_int(rhs->type)) {
        node->kind = ND_PTR_SUB;
        node->type = lhs->type;
    }
    if (is_int(lhs->type) && is_ref(rhs->type)) {
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
    Env *func = find_func(tok);
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

Node *new_var(Env *var) {
    Node *node = calloc(1, sizeof(Node));
    if (var->kind == GVar) {
        node->kind = ND_GVAR;
    }
    if (var->kind == LVar) {
        node->kind = ND_LVAR;
    }
    node->tok = var->tok;
    node->offset = var->offset;
    node->type = var->type;
    return node;
}

Node *new_ldeclare() {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LDECLARE;
    return node;
}

Node *new_declare(Token *tok, Env *gvar) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_DECLARE;
    node->tok = tok;
    node->offset = gvar->offset;
    return node;
}

Node *new_sizeof(Node *n) {
    Node *node = new_num(n->type->size);
    return node;
}

Node *implicit_conv(Node *node) {
    if (node->type->ty == ARRAY && (node->kind == ND_LVAR || node->kind == ND_GVAR)) {
        Node *n = calloc(1, sizeof(Node));
        n->kind = node->kind;
        n->tok = node->tok;

        node->kind = ND_ADDR;

        n->type = node->type->ptr_to;
        n->offset = node->offset - n->type->size * (node->type->array_len - 1);
        node->lhs = implicit_conv(n);
    }
    return node;
}

// Parser

Node *define();
Type *base_type();
Type *type_suffix(Type *type);
Node *func(Token *tok, Type *return_type);
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
Node *idx_access(Token *tok);

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

    Type *type = base_type();
    Token *tok = expect_ident();

    if (consume("(")) {
        return func(tok, type);
    }
    type = type_suffix(type);
    Env *gvar = extend_globals(GVar, tok, type);
    expect(";");
    return new_declare(tok, gvar);
}

Type *base_type() {
    Type *t = consume_type();
    if (!t) return NULL;

    while (consume("*")) {
        t = new_pointer_to(t);
    }
    return t;
}

Type *type_suffix(Type *type) {
    if(consume("[")) {
        int len = expect_number();
        type = new_array_type(len, type);
        expect("]");
        type = type_suffix(type);
    }
    return type;
}

Node* func(Token *tok, Type* return_type) {
    Node* ps = params();
    expect(")");
    extend_globals(FUNCTION, tok, return_type);
    Node *body = block();

    size_t max_offset = locals->offset;
    return new_definition(tok, ps, body, max_offset);
}

Node* params() {
    Node head;
    head.next = NULL;
    Node *cur = &head;

    while (!peek(1, ")")) {
        if (cur != &head) {
            expect(",");
        }

        Type *t = base_type();
        Token *tok = expect_ident();

        Env *lvar = extend_locals(tok, t);
        // set value as local variable
        Node *p = new_var(lvar);
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


    Type *t = base_type();
    if (t) {
        Token* tok = expect_ident();

        t = type_suffix(t);

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
            expect(")");
            return new_call(tok, arguments);
        }

        Node *node = idx_access(tok);
        if (node)
            return node;

        Env *var = find_var(tok);
        return new_var(var);
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

Node *idx_access(Token *tok) {
    if (consume("[")) {
        Node *i = expr();
        expect("]");
        Node* n = idx_access(tok);
        if (!n) {
            Env *var = find_var(tok);
            n = implicit_conv(new_var(var));
        }
        return new_deref(new_add(n, i)); // a[i] -> *(a+i)
    }
    return NULL;
}
