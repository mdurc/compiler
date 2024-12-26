#ifndef CODE_GEN_H
#define CODE_GEN_H

#include "parser.h"
#include "symbol_table.h"
#include <stddef.h>

typedef struct {
    char* data;
    char* body;
    char* util; // for functions like strcmp
    int body_size;
    int body_capacity;

    int data_size;
    int data_capacity;

    int util_size;
    int util_capacity;

    int label_counter;
} CodeBuffer;

void init_buffer(CodeBuffer* buffer);
void append_to_buffer(CodeBuffer* buf, int segment, const char* format, ...);
void free_buffer(CodeBuffer* buffer);
void write_buffer_to_file(CodeBuffer* buffer, const char* filename);
void generate_mips_code(AST* ast, const char* output_filename);
void handle_print(struct TokenNode* node, CodeBuffer* buffer, SymbolTable* table);
void handle_variable_declaration(struct TokenNode* node, CodeBuffer* buffer, SymbolTable* table, TokenType type);
void generate_operand_code(struct TokenNode* node, CodeBuffer* buffer, SymbolTable* table, int* string_cmp, int is_right);
void generate_comparator_code(TokenType comparator_type, CodeBuffer* buffer, int string_cmp);
void evaluate_condition(struct TokenNode* node, CodeBuffer* buffer, SymbolTable* table, char* false_label, char* start_body_label);
void handle_if_statement(struct TokenNode* node, CodeBuffer* buffer, SymbolTable* table);
void handle_while_loop(struct TokenNode* node, CodeBuffer* buffer, SymbolTable* table);
void handle_int_operations(struct TokenNode* node, CodeBuffer* buffer, SymbolTable* table);
void generate_code(struct TokenNode* node, CodeBuffer* buffer, SymbolTable* table);
void handle_function(struct TokenNode* node, CodeBuffer* buffer, SymbolTable* table);
void handle_statement(struct TokenNode* node, CodeBuffer* buffer);
char* generate_label(CodeBuffer* buffer);

#endif
