#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "parser.h"
#include "code_gen.h"
#include "symbol_table.h"

#define INITIAL_BUFFER_SIZE 1024

void init_buffer(CodeBuffer* buffer) {
    buffer->body_capacity = buffer->data_capacity = INITIAL_BUFFER_SIZE;
    buffer->body_size = buffer->data_size = 0;
    buffer->label_counter = 0;

    buffer->data = (char*)malloc(sizeof(char)*buffer->data_capacity);
    buffer->data[0] = '\0';

    buffer->body = (char*)malloc(sizeof(char)*buffer->body_capacity);
    buffer->body[0] = '\0';
}

void append_to_buffer(CodeBuffer* buf, int data_segment, const char* format, ...) {
    char** append_buf = &buf->body;
    int* capacity = &buf->body_capacity;
    int* size = &buf->body_size;
    if (data_segment){
        append_buf = &buf->data;
        capacity = &buf->data_capacity;
        size = &buf->data_size;
    }

    va_list args;
    va_start(args, format);

    int remaining = *capacity - *size;
    char temp[1024];
    int written = vsnprintf(temp, sizeof(temp), format, args);

    if (written >= remaining) {
        *capacity *= 2;
        *append_buf = (char*)realloc(*append_buf, sizeof(char) * (*capacity));
    }

    // append to end of buffer
    strncpy(*append_buf + *size, temp, written);
    *size += written;
    (*append_buf)[*size] = '\0';

    va_end(args);
}

void free_buffer(CodeBuffer* buffer) {
    free(buffer->data);
    free(buffer->body);

    buffer->body = NULL;
    buffer->body_size = 0;
    buffer->body_capacity = 0;
    
    buffer->data = NULL;
    buffer->data_size = 0;
    buffer->data_capacity = 0;

    buffer->label_counter = 0;
}

void write_buffer_to_file(CodeBuffer* buffer, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error opening output file\n");
        exit(EXIT_FAILURE);
    }
    fwrite(buffer->data, 1, buffer->data_size, file);
    fwrite(buffer->body, 1, buffer->body_size, file);
    fclose(file);
}

// entry point of code generation
void generate_mips_code(AST* ast, const char* output_filename) {
    if (!ast || !ast->root) {
        fprintf(stderr, "Error: parse tree is empty.\n");
        return;
    }

    if (ast->root->children_count > 0 && ast->root->children[0].token_data->type != MAIN){
        fprintf(stderr, "Error: main function not found.\n");
        return;
    }

    CodeBuffer buffer;
    init_buffer(&buffer);

    SymbolTable table;
    init_symbol_table(&table);

    // Starting mips content
    // text segment should go in body buffer so that the main function directly follows it
    append_to_buffer(&buffer, 0, "\n.text\n");
    append_to_buffer(&buffer, 0, ".align 2\n");
    append_to_buffer(&buffer, 0, ".globl main\n\n");
    append_to_buffer(&buffer, 1, ".data\n");

    // start generating code from the root
    // Note: as of now we only support having a "main" function
    generate_code(&ast->root->children[0], &buffer, &table);

    // write code from buffer to asm output file
    write_buffer_to_file(&buffer, output_filename);
    free_buffer(&buffer);
    free_symbol_table(&table);
}

void handle_print(struct TokenNode* node, CodeBuffer* buffer, SymbolTable* table){
    if (!(node->children_count == 1 && node->children[0].token_data->type == OPEN_PAREN) ||
        !(node->children[0].children_count >=1)){
        return;
    }

    struct TokenNode* open_paren = &node->children[0];
    // exclude the closing parenthesis child at the end
    for(int i=0; i < (open_paren->children_count-1); ++i){
        TokenType t = open_paren->children[i].token_data->type;
        const char* d = open_paren->children[i].token_data->data;
        if (t == STRING_LITERAL){
            char* label = generate_label(buffer);
            append_to_buffer(buffer, 1, "%s: .asciiz %s\n", label, open_paren->children[i].token_data->data);
            append_to_buffer(buffer, 0, "la $a0, %s\n", label);
            append_to_buffer(buffer, 0, "li $v0, 4\n");
            append_to_buffer(buffer, 0, "syscall\n\n");
            free(label);
        }else if (t == INT_LITERAL){
            append_to_buffer(buffer, 0, "li $a0, %s\n", open_paren->children[i].token_data->data);
            append_to_buffer(buffer, 0, "li $v0, 1\n");
            append_to_buffer(buffer, 0, "syscall\n\n");
        }else if (t == IDENTIFIER){
            // find the string in symbol table, retrieve what the label is from data segment
            Symbol* sym = find_symbol(table, d);
            if (!sym) return;
            if (sym->type == STRING_TYPE){
                append_to_buffer(buffer, 0, "la $a0, %s\n", d);
                append_to_buffer(buffer, 0, "li $v0, 4\n");
                append_to_buffer(buffer, 0, "syscall\n\n");
            }else if (sym->type == INT_TYPE){
                append_to_buffer(buffer, 0, "la $t0, %s\n", d);
                append_to_buffer(buffer, 0, "lw $a0, 0($t0)\n");
                append_to_buffer(buffer, 0, "li $v0, 1\n");
                append_to_buffer(buffer, 0, "syscall\n\n");
            }
        }else if (t == INT_TYPE){
        }
    }
}

void handle_variable_declaration(struct TokenNode* node, CodeBuffer* buffer, SymbolTable* table, TokenType type) {
    // children should be: identifier, =, literal

    // Error in syntax checks:
    if (node->children_count != 3) {
        fprintf(stderr, "Invalid variable declaration at line: %d\n", node->children[0].token_data->line);
        exit(EXIT_FAILURE);
    }
    if (node->children[0].token_data->type != IDENTIFIER) {
        fprintf(stderr, "Expecting IDENTIFIER at line: %d\n", node->children[0].token_data->line);
        exit(EXIT_FAILURE);
    }
    if (node->children[1].token_data->type != EQUAL) {
        fprintf(stderr, "Expecting EQUAL at line: %d\n", node->children[1].token_data->line);
        exit(EXIT_FAILURE);
    }

    // Type specific portion of function
    if ((type == STRING_TYPE && node->children[2].token_data->type != STRING_LITERAL) ||
        (type == INT_TYPE && node->children[2].token_data->type != INT_LITERAL)) {
        fprintf(stderr, "Expecting valid LITERAL for variable declaration at line: %d\n", node->children[2].token_data->line);
        exit(EXIT_FAILURE);
    }

    const char* var_name = node->children[0].token_data->data;
    const char* value = node->children[2].token_data->data;

    if (type == STRING_TYPE) {
        add_symbol(table, var_name, STRING_TYPE, value);
        append_to_buffer(buffer, 1, "%s: .asciiz %s\n", var_name, value);
    } else if (type == INT_TYPE) {
        add_symbol(table, var_name, INT_TYPE, value);
        append_to_buffer(buffer, 1, "%s: .word %s\n", var_name, value);
    } else {
        fprintf(stderr, "Unknown type at line: %d\n", node->children[0].token_data->line);
        exit(EXIT_FAILURE);
    }
}

void handle_if_statement(struct TokenNode* node, CodeBuffer* buffer, SymbolTable* table) {
    if (node->children_count != 2
        || node->children[0].token_data->type != OPEN_PAREN
        || node->children[1].token_data->type != OPEN_BRACE) {
        fprintf(stderr, "Invalid 'if' statement structure on line %d.\n", node->token_data->line);
        exit(EXIT_FAILURE);
    }

    // this is the OPEN_PAREN's children node
    if (node->children[0].children_count == 1) {
        fprintf(stderr, "No condition found in 'if' statement on line %d\n", node->children[0].token_data->line);
        exit(EXIT_FAILURE);
    }else if (node->children[1].children_count == 1) {
        // there is no body of the 'if' statement, so don't evaluate anything
        return;
    }

    // list of condition
    int num_comps = node->children[0].children_count;
    struct TokenNode* condition = node->children[0].children;
    struct TokenNode* comparator = NULL;
    // exclude the closing parenthesis in children
    for (int i=0; i<(num_comps-1); ++i){
        if (condition[i].token_data->type == EQUAL_EQUAL || 
             condition[i].token_data->type == BANG_EQUAL || 
             condition[i].token_data->type == GREATER || 
             condition[i].token_data->type == GREATER_EQUAL || 
             condition[i].token_data->type == LESS || 
             condition[i].token_data->type == LESS_EQUAL||
             condition[i].token_data->type == AND ||
             condition[i].token_data->type == OR){
            if (i == 1) { // for now, only support the form: x == y (with any literals & types)
                comparator = &condition[i];
            }
            break; // For now we only support one comparator in conditional
        }
    }
    if ((condition->token_data->type != TRUE && condition->token_data->type != FALSE) && 
        (comparator == NULL || (num_comps-1) != 3)) {
        fprintf(stderr, "Not a valid condition in 'if' statement on line %d\n", node->children[0].token_data->line);
        fprintf(stderr, "Number of arguments: %d\n", num_comps-1); // exclude CLOSE_PAREN
        exit(EXIT_FAILURE);
    }

    struct TokenNode* body = node->children[1].children;
    char* false_label = generate_label(buffer);
    int string_cmp = 0;
    char* t0 = NULL;
    char* t1 = NULL;

    // evaluate condition: using $t0 to denote a valid or invalid condition
    if (condition->token_data->type == TRUE) {
        append_to_buffer(buffer, 0, "li $t0, 1\n");
    } else if (condition->token_data->type == FALSE) {
        append_to_buffer(buffer, 0, "li $t0, 0\n");
    }else {
        // multi-token condition
        struct TokenNode* left = &condition[0]; // left operand
        struct TokenNode* right = &condition[2]; // right operand

        printf("LEFT CONDITION: %s\nRIGHT CONDITION: %s\n", left->token_data->data, right->token_data->data);

        // evaluate left operand
        if (left->token_data->type == IDENTIFIER) {
            Symbol* sym = find_symbol(table, left->token_data->data);
            if (!sym) {
                fprintf(stderr, "Undefined variable '%s' in condition.\n", left->token_data->data);
                exit(EXIT_FAILURE);
            }
            if (sym->type == INT_TYPE) {
                append_to_buffer(buffer, 0, "la $t1, %s\n", left->token_data->data);
                append_to_buffer(buffer, 0, "lw $t0, 0($t1)\n"); // load left operand into $t0
            } else if(sym->type == STRING_TYPE) {
                string_cmp = 1;
                t0 = sym->value;
            } else {
                fprintf(stderr, "Invalid type for variable '%s' in condition.\n", left->token_data->data);
                exit(EXIT_FAILURE);
            }
        } else if (left->token_data->type == INT_LITERAL) {
            append_to_buffer(buffer, 0, "li $t0, %s\n", left->token_data->data);
        } else if (left->token_data->type == STRING_LITERAL) {
            string_cmp = 1;
            t0 = left->token_data->data;
        } else {
            fprintf(stderr, "Unsupported left operand in condition.\n");
            exit(EXIT_FAILURE);
        }

        // evaluate right operand
        if (right->token_data->type == IDENTIFIER) {
            Symbol* sym = find_symbol(table, right->token_data->data);
            if (!sym) {
                fprintf(stderr, "Undefined variable '%s' in condition.\n", right->token_data->data);
                exit(EXIT_FAILURE);
            }
            if (sym->type == INT_TYPE) {
                append_to_buffer(buffer, 0, "la $t2, %s\n", right->token_data->data);
                append_to_buffer(buffer, 0, "lw $t1, 0($t2)\n");
            } else if (sym->type == STRING_TYPE) {
                string_cmp = 1;
                t1 = sym->value;
            } else {
                fprintf(stderr, "Invalid type for variable '%s' in condition.\n", right->token_data->data);
                exit(EXIT_FAILURE);
            }
        } else if (right->token_data->type == INT_LITERAL) {
            append_to_buffer(buffer, 0, "li $t1, %s\n", right->token_data->data);
        } else if (right->token_data->type == STRING_LITERAL) {
            string_cmp = 1;
            t1 = right->token_data->data;
        } else {
            fprintf(stderr, "Unsupported right operand in condition.\n");
            exit(EXIT_FAILURE);
        }

        if (string_cmp) {
            append_to_buffer(buffer, 0, "li $t0, 1\n");
            if (strcmp(t0, t1) == 0) {
                append_to_buffer(buffer, 0, "li $t1, 1\n");
            }else{
                append_to_buffer(buffer, 0, "li $t1, 0\n");
            }
        }

        // Generate comparison instructions based on the comparator
        switch (comparator->token_data->type) {
            case EQUAL_EQUAL:
                append_to_buffer(buffer, 0, "seq $t0, $t0, $t1\n"); // $t0 = ($t0 == $t1)
                break;
            case BANG_EQUAL:
                append_to_buffer(buffer, 0, "sne $t0, $t0, $t1\n"); // $t0 = ($t0 != $t1)
                break;
            case GREATER:
                append_to_buffer(buffer, 0, "sgt $t0, $t0, $t1\n"); // $t0 = ($t0 > $t1)
                break;
            case GREATER_EQUAL:
                append_to_buffer(buffer, 0, "sge $t0, $t0, $t1\n"); // $t0 = ($t0 >= $t1)
                break;
            case LESS:
                append_to_buffer(buffer, 0, "slt $t0, $t0, $t1\n"); // $t0 = ($t0 < $t1)
                break;
            case LESS_EQUAL:
                append_to_buffer(buffer, 0, "sle $t0, $t0, $t1\n"); // $t0 = ($t0 <= $t1)
                break;
            default:
                fprintf(stderr, "Unsupported comparator in condition.\n");
                exit(EXIT_FAILURE);
        }
    }

    // branch if condition is false
    append_to_buffer(buffer, 0, "beq $t0, $zero, %s\n\n", false_label);

    // gen code for the body of the if statement
    generate_code(body, buffer, table);

    // label for skipping the body
    append_to_buffer(buffer, 0, "%s:\n", false_label);
    free(false_label);
}

// recursively go through parse tree
void generate_code(struct TokenNode* node, CodeBuffer* buffer, SymbolTable* table) {
    if (!node) return;
    if (!node->token_data) return;
    switch(node->token_data->type){
        case MAIN:
            handle_function(node, buffer, table);
            break;
        case RETURN:
            handle_statement(node, buffer);
            break;
        case PRINT:
            handle_print(node, buffer, table);
            break;
        case STRING_TYPE:
            handle_variable_declaration(node, buffer, table, STRING_TYPE);
            break;
        case INT_TYPE:
            handle_variable_declaration(node, buffer, table, INT_TYPE);
            break;
        case IF:
            handle_if_statement(node, buffer, table);
            break;
        case WHILE:
            break;
        default: break;
    }
}

// TODO: should be adding function to symbol table, specifying what is allocated to stack
void handle_function(struct TokenNode* node, CodeBuffer* buffer, SymbolTable* table) {
    if (node->children_count < 2 || node->children[1].token_data->type != OPEN_BRACE) return;

    append_to_buffer(buffer, 0, "\n%s:\n", node->token_data->data);
    append_to_buffer(buffer, 0, "addi $sp, $sp, -4\n");
    append_to_buffer(buffer, 0, "sw $ra, 0($sp)\n");
    append_to_buffer(buffer, 0, "# function content\n\n");

    //child[0] is open paren for arguments
    //child[1] is open curly brace for function content

    int has_return = 0;
    struct TokenNode* brace = &node->children[1];
    for (int i=0; i < (brace->children_count-1); ++i){
        if (brace->children[i].token_data->type == RETURN){
            has_return = 1;
        }
        generate_code(&brace->children[i], buffer, table);
    }
    if (!has_return) {
        fprintf(stderr, "No return statement on function: %s\n", node->token_data->data);
        exit(EXIT_FAILURE);
    }

    // unload function on the occurence of a "return" as one of the children
}

void handle_statement(struct TokenNode* node, CodeBuffer* buffer) {
    if (node->token_data->type == RETURN) {
        if (node->children_count == 1 && node->children[0].token_data->type == INT_LITERAL){
            // child[0] is integer value to return
            append_to_buffer(buffer, 0, "# returning here\n");
            append_to_buffer(buffer, 0, "li $v0, %s\n", node->children[0].token_data->data);
            append_to_buffer(buffer, 0, "# unloading function\n");
            append_to_buffer(buffer, 0, "lw $ra, 0($sp)\n");
            append_to_buffer(buffer, 0, "addi $sp, $sp, 4\n");
            append_to_buffer(buffer, 0, "jr $ra\n");
        }
    }
}

// caller must free mem
char* generate_label(CodeBuffer* buffer) {
    char* label = malloc(20);
    if (!label) {
        fprintf(stderr, "Memory allocation failed for label generation.\n");
        exit(EXIT_FAILURE);
    }
    snprintf(label, 20, "L%d", buffer->label_counter);
    ++buffer->label_counter;
    return label;
}
