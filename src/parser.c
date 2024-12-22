#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "parser.h"

struct TokenNode* create_token_node(Token* token) {
    struct TokenNode* node = (struct TokenNode*)malloc(sizeof(struct TokenNode));
    node->token_data = token;
    node->children = NULL;
    node->children_count = 0;
    return node;
}

void add_child(struct TokenNode* parent, struct TokenNode* child) {
    if (parent->children == NULL) {
        parent->children = (struct TokenNode*)malloc(sizeof(struct TokenNode));
    } else {
        parent->children = (struct TokenNode*)realloc(parent->children, (parent->children_count + 1) * sizeof(struct TokenNode));
    }

    parent->children[parent->children_count] = *child;
    ++parent->children_count;
}

void stack_push(Stack* s, struct TokenNode* node){
    if (s->buf == NULL){
        s->capacity = 10;
        s->buf = (struct TokenNode**) malloc(sizeof(struct TokenNode*) * s->capacity);
    }else if (s->size == s->capacity){
        s->capacity *= 2;
        s->buf = (struct TokenNode**) realloc(s->buf, sizeof(struct TokenNode*) * s->capacity);
    }
    s->buf[s->size] = node;
    ++s->size;
}

void stack_pop(Stack* s){
    if (s->buf == NULL || s->size <= 0) return;
    --s->size;
}

void build_ast(Program* prog, AST* ast) {
    if (!prog || prog->token_count == 0) {
        fprintf(stderr, "Program has no tokens.\n");
        return;
    }

    ast->root = create_token_node(NULL);
    Stack parents = {NULL, 0, 0};

    for (int i=0; i<prog->token_count; ++i) {
        Token* curr_token = &prog->tokens[i];
        struct TokenNode* child_node = create_token_node(curr_token);

        if (curr_token->type == OPEN_BRACE){
            if (parents.size == 0){
                add_child(ast->root, child_node);
                stack_push(&parents, &ast->root->children[ast->root->children_count-1]);
            }else{
                add_child(parents.buf[parents.size-1], child_node);
                // push newest added child from parent
                stack_push(&parents, &parents.buf[parents.size-1]->children[parents.buf[parents.size-1]->children_count-1]);
            }
        } else if (curr_token->type == CLOSE_BRACE) {
            stack_pop(&parents);
            if (parents.size == 0){
                add_child(ast->root, child_node);
            }else{
                add_child(parents.buf[parents.size-1], child_node);
            }
        } else {
            if (parents.size == 0){
                add_child(ast->root, child_node);
            }else{
                printf("Adding under brace (%d): %s\n", parents.size, child_node->token_data->data);
                add_child(parents.buf[parents.size-1], child_node);
                printf("%s's children incremented to %d\n", parents.buf[parents.size-1]->token_data->data, parents.buf[parents.size-1]->children_count);
            }
        }
    }
    printf("PRINTED VALUE: %d\n", ast->root->children[4].children_count);
    printf("NUM AST CHILDREN: %d\n", ast->root->children_count);
    printf("PRINTED END: %d\n", ast->root->children[5].children_count);
}

void print_ast(struct TokenNode* root, int depth){
    if (!root) return;
    for (int i=0; i<depth; ++i){
        printf("-");
    }
    if (root->token_data){
        printf("%s\n", root->token_data->data);
    }
    for(int i=0; i<root->children_count; ++i){
        print_ast(&root->children[i], depth+1);
    }
}


void destroy_ast_node(struct TokenNode* node) {
    if (node) {
        for (int i = 0; i < node->children_count; ++i) {
            destroy_ast_node(&node->children[i]);
        }
        free(node->children);
        free(node);
    }
}

void destroy_ast(AST* ast) {
    if (ast) {
        destroy_ast_node(ast->root);
        free(ast);
    }
}
