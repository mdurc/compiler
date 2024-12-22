
#include "lexer.h"
#include "parser.h"

void free_program(Program* prog){
    int i;
    for (i=0; i<prog->token_count; ++i) {
        free(prog->tokens[i].data);
    }
    free(prog->tokens);
}

int main(int argc, char** argv){
    if (argc != 2){
        fprintf(stderr, "Usage: ./a.out input.code\n");
        exit(EXIT_FAILURE);
    }
    FILE* fp = fopen(argv[1], "r");

    Program prog = {NULL, 0, 0};
    lex_file(fp, &prog.tokens, &prog.token_count, &prog.token_capacity);

    AST ast;
    build_ast(&prog, &ast);

    print_ast(ast.root, 0, 1);

    free_program(&prog);
    fclose(fp);
    return EXIT_SUCCESS;
}
