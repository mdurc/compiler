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

struct TokenNode* create_token_node(Token* token);
void add_child(struct TokenNode* parent, struct TokenNode* child);
void build_ast(Program* prog, AST* ast);
void destroy_ast_node(struct TokenNode* node);
void destroy_ast(AST* ast);
void print_ast(struct TokenNode* root, int depth, int is_root);

#endif
