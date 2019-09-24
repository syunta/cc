#include "9cc.h"

Node *code;
LVar *locals;

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
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

Node *new_definition(Token *tok, Node* params, Node *body) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_DEFINE;
    node->tok = tok;
    node->params = params;
    node->body = body;
    return node;
}

Node *new_call(Token *tok, Node *args) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_CALL;
    node->tok = tok;
    node->args = args;
    return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

void initialize_locals() {
    locals = calloc(1, sizeof(LVar));
    locals->offset = 0;
}

LVar *find_lvar(Token *tok) {
    for (LVar *var = locals; var; var = var->next) {
        if (var->len == tok->len && !strncmp(tok->str, var->name, var->len)) {
            return var;
        }
    }
    return NULL;
}

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
    Token *name = expect_ident();
    expect("(");
    Node* ps = params();
    expect(")");
    Node *body = block();
    return new_definition(name, ps, body);
}

Node* params() {
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
            node = new_node(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume("*"))
            node = new_node(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

Node *unary() {
    if (consume("+"))
        return primary();
    if (consume("-"))
        return new_node(ND_SUB, new_node_num(0), primary());
    return primary();
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

        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;

        LVar *lvar = find_lvar(tok);
        if (lvar) {
            node->offset = lvar->offset;
        } else {
            lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;
            lvar->name = tok->str;
            lvar->len = tok->len;
            lvar->offset = locals->offset + 8;
            node->offset = lvar->offset;
            locals = lvar;
        }
        return node;
    }

    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    return new_node_num(expect_number());
}

Node *args() {
    Node head;
    head.next = NULL;
    Node *cur = &head;
    while (!peek(1, ")")) {
        Node *s = expr();
        cur->next = s;
        cur = s;
        consume(",");
    }
    return head.next;
}
