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
    Stack parent_stack = {NULL, 0, 0};

    for (int i=0; i<prog->token_count; ++i) {
        Token* curr_token = &prog->tokens[i];
        struct TokenNode* child_node = create_token_node(curr_token);
        struct TokenNode* parent_node = parent_stack.size == 0 ? ast->root : parent_stack.buf[parent_stack.size-1];
        
        if (curr_token->type == OPEN_BRACE){
            // open new ast layer
            add_child(parent_node, child_node);
            stack_push(&parent_stack, &parent_node->children[parent_node->children_count - 1]);

        } else if (curr_token->type == CLOSE_BRACE) {
            // close ast layer
            stack_pop(&parent_stack);
            add_child(parent_node, child_node);
        } else {
            // either adding to ast->root or adding within some ast layer
            add_child(parent_node, child_node);
        }
    }
}

void print_ast(struct TokenNode* root, int depth, int is_root){
    if (is_root) {
        printf("Root\n");
        is_root = 0;
    }
    if (!root) return;
    for (int i=0; i<depth; ++i){
        printf("-");
    }
    if (root->token_data){
        printf("%s\n", root->token_data->data);
    }
    for(int i=0; i<root->children_count; ++i){
        print_ast(&root->children[i], depth+1, is_root);
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
