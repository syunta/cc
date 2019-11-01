#include "9cc.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }

    user_input = argv[1];
    token = tokenize();

    initialize_locals();
    program();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");

    gen_globals(code);

    return 0;
}
