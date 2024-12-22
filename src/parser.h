#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef struct{
    Token* tokens;
    int token_count;
    int token_capacity;
} Program;

struct TokenNode {
    Token* token_data;
    struct TokenNode* children;
    int children_count;
    int children_capacity;
};

// abstract syntax tree
typedef struct {
    struct TokenNode* root;
} AST;

typedef struct{
    struct TokenNode** buf;
    int size;
    int capacity;
} Stack;

void add_child(struct TokenNode* parent, Token* child);
void build_ast(Program* prog, AST* ast);
void free_ast_children(struct TokenNode* node);
void free_ast(AST* ast);
void print_ast(struct TokenNode* root, int depth, int is_root);

#endif
