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

// TODO: verify program ast is within context free language of the program
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
    TokenType parent_keywords[][2] = {{IF, CLOSE_BRACE}, {ELSE, CLOSE_BRACE},
                                    {WHILE, CLOSE_BRACE}, {FOR, CLOSE_BRACE},
                                    {MAIN, CLOSE_BRACE}, {RETURN, INT},
                                    {PRINT_STRING, CLOSE_PAREN}, {PRINT_INT, CLOSE_PAREN}};
    TokenType parent_enclosers[][2] = {{OPEN_BRACE, CLOSE_BRACE}, {OPEN_PAREN, CLOSE_PAREN},
                                    {OPEN_BRACKET, CLOSE_BRACKET}};

    for (int i=0; i<prog->token_count; ++i) {
    //for (int i=0; i<7; ++i) {
        Token* curr_token = &prog->tokens[i];
        struct TokenNode* parent_node = parent_stack.size == 0 ? ast->root : parent_stack.buf[parent_stack.size-1];

        // TokenNode is 4 bytes, each parent_token is one tokens
        int new_layer = 0;
        for (int j = 0; j < (int)(sizeof(parent_enclosers)/(sizeof(TokenType)*2)); ++j){
            if (curr_token->type == parent_enclosers[j][0]) {
                new_layer = 1; break;
            }else if (curr_token->type == parent_enclosers[j][1]) {
                new_layer = -1; break;
            }
        }
        for (int j=0; j < (int)(sizeof(parent_keywords)/sizeof(TokenType)); ++j){
            if (curr_token->type == parent_keywords[j][0]){
                new_layer = 1; break;
            }else if (curr_token->type == parent_keywords[j][1]){
                new_layer = -1; break;
            }
        }
        if (new_layer == 1){
            // open new ast layer
            add_child(parent_node, curr_token);
            stack_push(&parent_stack, &parent_node->children[parent_node->children_count - 1]);

        } else if (new_layer == -1) {
            // close ast layer
            stack_pop(&parent_stack);

            // Go back to original scope of conditonal
            if (parent_stack.size > 0){
                for (int j=0; j < (int)(sizeof(parent_keywords)/sizeof(TokenType)); ++j){
                    if (parent_stack.buf[parent_stack.size-1]->token_data->type == parent_keywords[j][0] &&
                            curr_token->type == parent_keywords[j][1]){
                        stack_pop(&parent_stack);
                        break;
                    }
                }
            }

            add_child(parent_node, curr_token);
        } else {
            // either adding to ast->root or adding within some ast layer
            add_child(parent_node, curr_token);
        }
    }
    if (parent_stack.buf){
        free(parent_stack.buf);
        parent_stack.buf = NULL;
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
    const char *token_type_to_string[] = {
        "OPEN_PAREN", "CLOSE_PAREN", "OPEN_BRACE", "CLOSE_BRACE",
        "OPEN_BRACKET", "CLOSE_BRACKET",
        "COMMA", "DOT", "MINUS", "PLUS", "SLASH", "STAR",

        "BANG", "BANG_EQUAL", "EQUAL", "EQUAL_EQUAL",
        "GREATER", "GREATER_EQUAL", "LESS", "LESS_EQUAL",

        "IDENTIFIER", "STRING", "INT", "MAIN",

        "AND", "OR", "IF", "ELSE", "TRUE", "FALSE", "FOR", "WHILE", 
        "PRINT_INT", "PRINT_STRING", "RETURN"
    };
    if (root->token_data){
        printf("%s\t\t(%s, %d)\n", root->token_data->data,
               token_type_to_string[root->token_data->type], root->children_count);
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
    if (node->children){
        free(node->children);
        node->children = NULL;
    }
}

void free_ast(AST* ast){
    if (ast == NULL) return;
    free_ast_children(ast->root);
    free(ast->root);
    ast->root = NULL;
}
