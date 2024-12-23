#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "parser.h"
#include "code_gen.h"

#define INITIAL_BUFFER_SIZE 1024

void init_buffer(CodeBuffer* buffer) {
    buffer->capacity = INITIAL_BUFFER_SIZE;
    buffer->size = 0;
    buffer->label_counter = 0;
    buffer->data = (char*)malloc(sizeof(char)*buffer->capacity);
    buffer->data[0] = '\0';
}

void append_to_buffer(CodeBuffer* buffer, const char* format, ...) {
    va_list args;
    va_start(args, format);

    size_t remaining = buffer->capacity - buffer->size;
    char temp[1024];
    int written = vsnprintf(temp, sizeof(temp), format, args);

    if (written >= remaining) {
        buffer->capacity *= 2;
        buffer->data = (char*)realloc(buffer->data, sizeof(char)*buffer->capacity);
    }

    // append to end of buffer
    strncpy(buffer->data + buffer->size, temp, written);
    buffer->size += written;
    buffer->data[buffer->size] = '\0';

    va_end(args);
}

void free_buffer(CodeBuffer* buffer) {
    free(buffer->data);
    buffer->data = NULL;
    buffer->size = 0;
    buffer->capacity = 0;
    buffer->label_counter = 0;
}

void write_buffer_to_file(CodeBuffer* buffer, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error opening output file\n");
        exit(EXIT_FAILURE);
    }
    fwrite(buffer->data, 1, buffer->size, file);
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

    // Starting mips content
    append_to_buffer(&buffer, ".data\n");
    append_to_buffer(&buffer, "    newline: .asciiz \"\\n\"\n");
    append_to_buffer(&buffer, ".text\n");
    append_to_buffer(&buffer, "    .align 2\n");
    append_to_buffer(&buffer, "    .globl main\n");

    // start generating code from the root
    // Note: as of now we only support having a "main" function
    generate_code(&ast->root->children[0], &buffer);

    // write code from buffer to asm output file
    write_buffer_to_file(&buffer, output_filename);
    free_buffer(&buffer);
}

// recursively go through parse tree
void generate_code(struct TokenNode* node, CodeBuffer* buffer) {
    if (!node) return;
    if (!node->token_data) return;
    switch(node->token_data->type){
        case MAIN:
            handle_function(node, buffer);
            break;
        case RETURN:
            handle_statement(node, buffer);
            break;
        case PRINT_INT: break;
        case PRINT_STRING: break;
        default: break;
    }
}

void handle_function(struct TokenNode* node, CodeBuffer* buffer) {
    append_to_buffer(buffer, "%s:\n", node->token_data->data);
    append_to_buffer(buffer, "    addi $sp, $sp, -4\n");
    append_to_buffer(buffer, "    sw $ra, 4($sp)\n");

    //child[0] is open paren for arguments
    //child[1] is open curly brace for function content

    if (node->children_count >= 2 && node->children[1].token_data->type == OPEN_BRACE){
        append_to_buffer(buffer, "THIS IS A TEST\n");
    }

    append_to_buffer(buffer, "    lw $ra, 4($sp)\n");
    append_to_buffer(buffer, "    addi $sp, $sp, 4\n");
    append_to_buffer(buffer, "    jr $ra\n");
}

void handle_statement(struct TokenNode* node, CodeBuffer* buffer) {
    if (node->token_data->type == RETURN) {
        // child[0] is integer value to return
        append_to_buffer(buffer, "    # return statement\n");
        handle_expression(&node->children[0], buffer); // return value
        append_to_buffer(buffer, "    move $v0, $t0\n");
        append_to_buffer(buffer, "    jr $ra\n");
    }
}

void handle_expression(struct TokenNode* node, CodeBuffer* buffer) {
    if (node->token_data) {
        append_to_buffer(buffer, "    # Load constant\n");
        append_to_buffer(buffer, "    li $t0, %s\n", node->token_data->data);
    }
}

const char* generate_label(CodeBuffer* buffer) {
    static char label[20];
    snprintf(label, sizeof(label), "L%d", buffer->label_counter);
    ++buffer->label_counter;
    return label;
}
