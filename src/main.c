
#include "lexer.h"
#include "parser.h"
#include "code_gen.h"

void free_program_tokens(Program* prog){
    int i;
    for (i=0; i<prog->token_count; ++i) {
        free(prog->tokens[i].data);
        prog->tokens[i].data = NULL;
    }
    free(prog->tokens);
    prog->tokens = NULL;
}

int main(int argc, char** argv){
    if (argc != 2){
        fprintf(stderr, "Usage: ./a.out input.code\n");
        exit(EXIT_FAILURE);
    }
    FILE* fp = fopen(argv[1], "r");
    if (fp == NULL){
        fprintf(stderr, "Invalid input file: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    Program prog = {NULL, 0, 0};
    lex_file(fp, &prog.tokens, &prog.token_count, &prog.token_capacity);

    AST ast;
    build_ast(&prog, &ast);

    print_ast(ast.root, 0);

    generate_mips_code(&ast, "out.asm");

    free_ast(&ast);
    free_program_tokens(&prog);
    fclose(fp);
    return EXIT_SUCCESS;
}
