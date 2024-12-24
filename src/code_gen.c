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
            const char* label = generate_label(buffer);
            append_to_buffer(buffer, 1, "%s: .asciiz %s\n", label, open_paren->children[i].token_data->data);
            append_to_buffer(buffer, 0, "la $a0, %s\n", label);
            append_to_buffer(buffer, 0, "li $v0, 4\n");
            append_to_buffer(buffer, 0, "syscall\n\n");
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
        add_symbol(table, var_name, STRING_TYPE);
        append_to_buffer(buffer, 1, "%s: .asciiz %s\n", var_name, value);
    } else if (type == INT_TYPE) {
        add_symbol(table, var_name, INT_TYPE);
        append_to_buffer(buffer, 1, "%s: .word %s\n", var_name, value);
    } else {
        fprintf(stderr, "Unknown type at line: %d\n", node->children[0].token_data->line);
        exit(EXIT_FAILURE);
    }
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

const char* generate_label(CodeBuffer* buffer) {
    static char label[20];
    snprintf(label, sizeof(label), "L%d", buffer->label_counter);
    ++buffer->label_counter;
    return label;
}
