#ifndef CODE_GEN_H
#define CODE_GEN_H

#include "parser.h"
#include <stddef.h>

typedef struct {
    char* data;
    char* body;
    int body_size;
    int body_capacity;

    int data_size;
    int data_capacity;

    int label_counter;
} CodeBuffer;

typedef enum {
    JUMP,
    ASCIIZ,
    WORD
} LabelType;

void init_buffer(CodeBuffer* buffer);
void append_to_buffer(CodeBuffer* buf, int data_segment, const char* format, ...);
void free_buffer(CodeBuffer* buffer);
void write_buffer_to_file(CodeBuffer* buffer, const char* filename);
void generate_mips_code(AST* ast, const char* output_filename);
void generate_code(struct TokenNode* node, CodeBuffer* buffer);
void handle_function(struct TokenNode* node, CodeBuffer* buffer);
void handle_statement(struct TokenNode* node, CodeBuffer* buffer);
const char* generate_label(CodeBuffer* buffer);

#endif
