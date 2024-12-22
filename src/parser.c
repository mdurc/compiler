#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "parser.h"

void add_child(struct TokenNode* parent, Token* child) {
    if (parent->children == NULL) {
        parent->children_capacity = 5;
        parent->children = (struct TokenNode*)malloc(sizeof(struct TokenNode) * parent->children_capacity);
    } else if (parent->children_count == parent->children_capacity){
        parent->children_capacity *= 2;
        parent->children = (struct TokenNode*)realloc(parent->children, sizeof(struct TokenNode) * parent->children_capacity);
    }

    parent->children[parent->children_count].children_count = 0;
    parent->children[parent->children_count].children_capacity = 0;
    parent->children[parent->children_count].children = NULL;
    parent->children[parent->children_count].token_data = child;

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

    ast->root = (struct TokenNode*)malloc(sizeof(struct TokenNode));
    ast->root->token_data = NULL;
    ast->root->children = NULL;
    ast->root->children_capacity = 0;
    ast->root->children_count = 0;

    Stack parent_stack = {NULL, 0, 0};

    for (int i=0; i<prog->token_count; ++i) {
    //for (int i=0; i<7; ++i) {
        Token* curr_token = &prog->tokens[i];
        struct TokenNode* parent_node = parent_stack.size == 0 ? ast->root : parent_stack.buf[parent_stack.size-1];
        
        if (curr_token->type == OPEN_BRACE){
            // open new ast layer
            add_child(parent_node, curr_token);
            stack_push(&parent_stack, &parent_node->children[parent_node->children_count - 1]);

        } else if (curr_token->type == CLOSE_BRACE) {
            // close ast layer
            stack_pop(&parent_stack);
            add_child(parent_node, curr_token);
        } else {
            // either adding to ast->root or adding within some ast layer
            add_child(parent_node, curr_token);
        }
    }
    if (parent_stack.buf){
        free(parent_stack.buf);
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

void free_ast_children(struct TokenNode* node) {
    if (!node) return;
    for (int i = 0; i < node->children_count; ++i){
        // search all possible depths
        if (node->children[i].children_count > 0){
            free_ast_children(&node->children[i]);
        }
    }
    // we have found a node that, for all its children, has no more depth
    if (node->children) free(node->children);
}

void free_ast(AST* ast){
    if (ast == NULL) return;
    free_ast_children(ast->root);
    free(ast->root);
}
