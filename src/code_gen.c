#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "lexer.h"
#include "parser.h"
#include "code_gen.h"
#include "symbol_table.h"

#define INITIAL_BUFFER_SIZE 1024

void init_buffer(CodeBuffer* buffer) {
    buffer->body_capacity = buffer->data_capacity = buffer->util_capacity= INITIAL_BUFFER_SIZE;
    buffer->body_size = buffer->data_size = buffer->util_size = 0;
    buffer->label_counter = 0;

    buffer->body = (char*)malloc(sizeof(char)*buffer->body_capacity);
    buffer->body[0] = '\0';

    buffer->data = (char*)malloc(sizeof(char)*buffer->data_capacity);
    buffer->data[0] = '\0';

    buffer->util = (char*)malloc(sizeof(char)*buffer->util_capacity);
    buffer->util[0] = '\0';
}

// segment 0 is body, 1 is data, 2 is util
void append_to_buffer(CodeBuffer* buf, int segment, const char* format, ...) {
    char** append_buf = &buf->body;
    int* capacity = &buf->body_capacity;
    int* size = &buf->body_size;
    if (segment == 1){
        append_buf = &buf->data;
        capacity = &buf->data_capacity;
        size = &buf->data_size;
    }else if (segment == 2){
        append_buf = &buf->util;
        capacity = &buf->util_capacity;
        size = &buf->util_size;
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
    free(buffer->util);

    buffer->body = NULL;
    buffer->body_size = 0;
    buffer->body_capacity = 0;
    
    buffer->data = NULL;
    buffer->data_size = 0;
    buffer->data_capacity = 0;

    buffer->util = NULL;
    buffer->util_size = 0;
    buffer->util_capacity = 0;

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
    fwrite(buffer->util, 1, buffer->util_size, file);
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
    append_to_buffer(&buffer, 1, "newline: .asciiz \"\\n\"\n");

    // start generating code from the root
    // Note: as of now we only support having a "main" function
    generate_code(&ast->root->children[0], &buffer, &table);

    FILE* file = fopen("util/strcmp.asm", "r");
    if (!file) {
        fprintf(stderr, "Error opening util/strcmp.asm file\n");
        exit(EXIT_FAILURE);
    }
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line,"\n")] = '\0';
        append_to_buffer(&buffer, 2, "%s\n", line);
    }
    fclose(file);

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

    append_to_buffer(buffer, 0, "# == print start ==\n");
    struct TokenNode* open_paren = &node->children[0];
    // exclude the closing parenthesis child at the end
    for(int i=0; i < (open_paren->children_count-1); ++i){
        TokenType t = open_paren->children[i].token_data->type;
        if (t == STRING_LITERAL){
            if (strcmp(open_paren->children[i].token_data->data, "\"\\n\"") == 0){
                // reuse the same newline asciiz label in data section so that
                // a ton of newlines aren't created
                // TODO: might want to remove once/if strings become mutable
                append_to_buffer(buffer, 0, "la $a0, newline\n");
            }else{
                char* label = generate_label(buffer);
                append_to_buffer(buffer, 1, "%s: .asciiz %s\n", label, open_paren->children[i].token_data->data);
                append_to_buffer(buffer, 0, "la $a0, %s\n", label);
                free(label);
            }
            append_to_buffer(buffer, 0, "li $v0, 4\n");
            append_to_buffer(buffer, 0, "syscall\n");
        }else if (t == INT_LITERAL){
            append_to_buffer(buffer, 0, "li $a0, %s\n", open_paren->children[i].token_data->data);
            append_to_buffer(buffer, 0, "li $v0, 1\n");
            append_to_buffer(buffer, 0, "syscall\n");
        }else if (t == IDENTIFIER){
            // find the string in symbol table, retrieve what the label is from data segment
            const char* data = open_paren->children[i].token_data->data;
            Symbol* sym = find_symbol(table, data);
            if (!sym) return;
            if (sym->type == STRING_TYPE){
                append_to_buffer(buffer, 0, "la $a0, %s\n", sym->label);
                append_to_buffer(buffer, 0, "li $v0, 4\n");
                append_to_buffer(buffer, 0, "syscall\n");
            }else if (sym->type == INT_TYPE){
                append_to_buffer(buffer, 0, "la $t0, %s\n", sym->label);
                append_to_buffer(buffer, 0, "lw $a0, 0($t0)\n");
                append_to_buffer(buffer, 0, "li $v0, 1\n");
                append_to_buffer(buffer, 0, "syscall\n");
            }
        }
    }
    append_to_buffer(buffer, 0, "# == print end ==\n");
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

    // Create a new label for the variable just in case the variable name is an assembly instruction
    const char* var_name = node->children[0].token_data->data;
    char* var_label = generate_label(buffer);
    const char* value = node->children[2].token_data->data;

    if (type == STRING_TYPE) {
        add_symbol(table, var_name, var_label, STRING_TYPE, value);
        append_to_buffer(buffer, 1, "%s: .asciiz %s\n", var_label, value);
    } else if (type == INT_TYPE) {
        add_symbol(table, var_name, var_label, INT_TYPE, value);
        append_to_buffer(buffer, 1, "%s: .word %s\n", var_label, value);
    } else {
        free(var_label);
        fprintf(stderr, "Unknown type at line: %d\n", node->children[0].token_data->line);
        exit(EXIT_FAILURE);
    }
    free(var_label);
}

void evaluate_condition(struct TokenNode* node, CodeBuffer* buffer, SymbolTable* table, char* false_label, char* start_body_label) {
    if (node->token_data->type != OPEN_PAREN || node->children_count < 4) {
        if (!(node->children_count == 2 && (node->children[0].token_data->type == TRUE 
            || node->children[0].token_data->type == FALSE))){
            fprintf(stderr, "No condition found in 'if' statement on line %d\n", node->token_data->line);
            exit(EXIT_FAILURE);
        }
    }

    struct TokenNode* condition = node->children;
    int num_comps = node->children_count;

    // handle an isolated true or false in condition
    if (num_comps == 2 && (condition[0].token_data->type == TRUE || condition[0].token_data->type == FALSE)) {
        if (condition[0].token_data->type == TRUE) {
            append_to_buffer(buffer, 0, "# Condition is TRUE, always proceed\n");
        } else if (condition[0].token_data->type == FALSE) {
            append_to_buffer(buffer, 0, "# Condition is FALSE, always branch\n");
            append_to_buffer(buffer, 0, "j %s\n\n", false_label);
        }
        return;
    }
    
    int string_cmp = 0;
    int i = 0;
    int last_logic_AND = 1;

    // go through all comparisons in the conditional (...), joined together by 'and's and 'or's
    // no precedence, just evaluates left to right with short circuit
    // Though note that OR's require checking every outcome
    while (i < num_comps - 1) {
        struct TokenNode* left = &condition[i];
        struct TokenNode* comparator = NULL;
        struct TokenNode* right = NULL;
        if (left->token_data->type == TRUE || left->token_data->type == FALSE){
            generate_operand_code(left, buffer, table, &string_cmp, 0);
        }else if (i + 2 < num_comps - 1){
            comparator = &condition[i + 1];
            right = &condition[i + 2];
            generate_operand_code(left, buffer, table, &string_cmp, 0);
            generate_operand_code(right, buffer, table, &string_cmp, 1);
            generate_comparator_code(comparator->token_data->type, buffer, string_cmp);
            printf("Comparing %s and %s, with %s\n", left->token_data->data, right->token_data->data, comparator->token_data->data);
        }else{
            fprintf(stderr, "Missing comparator and operand two in condition on line %d\n", node->token_data->line);
            exit(EXIT_FAILURE);
        }

        // handle AND, OR, conjunctions
        // if (x != y and i == k)
        int increment = comparator == NULL ? 1 : 3;
        if (i + increment < num_comps - 1) {
            struct TokenNode* op = &condition[i + increment];
            if (op->token_data->type == LOGIC_AND) {
                append_to_buffer(buffer, 0, "beq $v0, $zero, %s\n", false_label);
                last_logic_AND = 1;
            } else if (op->token_data->type == LOGIC_OR) {
                append_to_buffer(buffer, 0, "bne $v0, $zero, %s\n", start_body_label);
                last_logic_AND = 0;
            }
        } else {
            if (last_logic_AND) {
                append_to_buffer(buffer, 0, "beq $v0, $zero, %s\n", false_label);
            }else{
                append_to_buffer(buffer, 0, "bne $v0, $zero, %s\n", start_body_label);
            }
            break;
        }
        i += (increment + 1);
    }
    // if we ended on or, and it fails, we should jump over the body
    if (!last_logic_AND){
        append_to_buffer(buffer, 0, "j %s\n", false_label);
    }
}

void generate_operand_code(struct TokenNode* node, CodeBuffer* buffer, SymbolTable* table, int* string_cmp, int is_right) {
    if (node->token_data->type == TRUE) {
        append_to_buffer(buffer, 0, "li $v0, 1\n");
    } else if (node->token_data->type == FALSE) {
        append_to_buffer(buffer, 0, "li $v0, 0\n");
    } else if (node->token_data->type == IDENTIFIER) {
        Symbol* sym = find_symbol(table, node->token_data->data);
        if (!sym) {
            fprintf(stderr, "Undefined variable '%s' in condition.\n", node->token_data->data);
            exit(EXIT_FAILURE);
        }
        if (sym->type == INT_TYPE) {
            append_to_buffer(buffer, 0, "la $t%d, %s\n", is_right ? 2 : 1, sym->label);
            append_to_buffer(buffer, 0, "lw $t%d, 0($t%d)\n", is_right ? 1 : 0, is_right ? 2 : 1);
        } else if (sym->type == STRING_TYPE) {
            *string_cmp = 1;
            append_to_buffer(buffer, 0, "la $a%d, %s\n", is_right ? 1 : 0, sym->label);
        } else {
            fprintf(stderr, "Unsupported variable type '%s' in condition.\n", node->token_data->data);
            exit(EXIT_FAILURE);
        }
    } else if (node->token_data->type == INT_LITERAL) {
        append_to_buffer(buffer, 0, "li $t%d, %s\n", is_right ? 1 : 0, node->token_data->data);
    } else if (node->token_data->type == STRING_LITERAL) {
        *string_cmp = 1;
        char* label = generate_label(buffer);
        append_to_buffer(buffer, 1, "%s: .asciiz %s\n", label, node->token_data->data);
        append_to_buffer(buffer, 0, "la $a%d, %s\n", is_right ? 1 : 0, label);
        free(label);
    } else {
        fprintf(stderr, "Unsupported operand type in condition.\n");
        exit(EXIT_FAILURE);
    }
}

void generate_comparator_code(TokenType comparator_type, CodeBuffer* buffer, int string_cmp) {
    if (string_cmp) {
        append_to_buffer(buffer, 0, "jal strcmp\n");
    }
    switch (comparator_type) {
        case EQUAL_EQUAL:
            if (string_cmp){
                append_to_buffer(buffer, 0, "seq $v0, $v0, $zero\n");
            }else{
                append_to_buffer(buffer, 0, "seq $v0, $t0, $t1\n"); // $t0 = ($t0 == $t1)
            }
            break;
        case BANG_EQUAL:
            if (string_cmp){
                append_to_buffer(buffer, 0, "sne $v0, $v0, $zero\n");
            }else{
                append_to_buffer(buffer, 0, "sne $v0, $t0, $t1\n"); // $t0 = ($t0 != $t1)
            }
            break;
        case GREATER:
            if (string_cmp){
                append_to_buffer(buffer, 0, "sgt $v0, $v0, $zero\n");
            }else{
                append_to_buffer(buffer, 0, "sgt $v0, $t0, $t1\n"); // $t0 = ($t0 > $t1)
            }
            break;
        case GREATER_EQUAL:
            if (string_cmp){
                append_to_buffer(buffer, 0, "sge $v0, $v0, $zero\n");
            }else{
                append_to_buffer(buffer, 0, "sge $v0, $t0, $t1\n"); // $t0 = ($t0 >= $t1)
            }
            break;
        case LESS:
            if (string_cmp){
                append_to_buffer(buffer, 0, "slt $v0, $v0, $zero\n");
            }else{
                append_to_buffer(buffer, 0, "slt $v0, $t0, $t1\n"); // $t0 = ($t0 < $t1)
            }
            break;
        case LESS_EQUAL:
            if (string_cmp){
                append_to_buffer(buffer, 0, "sle $v0, $v0, $zero\n");
            }else{
                append_to_buffer(buffer, 0, "sle $v0, $t0, $t1\n"); // $t0 = ($t0 <= $t1)
            }
            break;
        default:
            fprintf(stderr, "Unsupported comparator '%s' in condition.\n", token_type_to_string[comparator_type]);
            exit(EXIT_FAILURE);
    }
}

void handle_if_statement(struct TokenNode* node, CodeBuffer* buffer, SymbolTable* table) {
    if (node->children_count < 2 || node->children[0].token_data->type != OPEN_PAREN ||
        node->children[1].token_data->type != OPEN_BRACE || node->children[1].children_count == 1) return;

    append_to_buffer(buffer, 0, "# == if-conditional start ==\n");

    // list of condition
    char* false_label = generate_label(buffer);
    char* start_body_label = generate_label(buffer);
    evaluate_condition(&node->children[0], buffer, table, false_label, start_body_label);

    append_to_buffer(buffer, 0, "%s:\n", start_body_label);
    struct TokenNode* body = &node->children[1];
    for (int i=0; i<body->children_count-1; ++i){
        // gen code for the body of the if statement
        generate_code(&body->children[i], buffer, table);
    }

    // label for skipping the body
    append_to_buffer(buffer, 0, "# == if-conditional end ==\n");
    append_to_buffer(buffer, 0, "%s:\n", false_label);
    free(false_label);
    free(start_body_label);
}

void handle_while_loop(struct TokenNode* node, CodeBuffer* buffer, SymbolTable* table){
    if (node->children_count < 2 || node->children[0].token_data->type != OPEN_PAREN ||
        node->children[1].token_data->type != OPEN_BRACE || node->children[1].children_count == 1) return;

    append_to_buffer(buffer, 0, "# == while-conditional start ==\n");

    // list of condition
    char* loop_label = generate_label(buffer);
    append_to_buffer(buffer, 0, "%s:\n", loop_label);

    char* false_label = generate_label(buffer);
    char* start_body_label = generate_label(buffer);
    evaluate_condition(&node->children[0], buffer, table, false_label, start_body_label);

    append_to_buffer(buffer, 0, "%s:\n", start_body_label);
    struct TokenNode* body = &node->children[1];
    for (int i=0; i<body->children_count-1; ++i){
        // gen code for the body of the if statement
        generate_code(&body->children[i], buffer, table);
    }

    // looping
    append_to_buffer(buffer, 0, "j %s\n", loop_label);

    append_to_buffer(buffer, 0, "# == while-conditional loop/end ==\n");
    // label for skipping the body
    append_to_buffer(buffer, 0, "%s:\n", false_label);

    free(loop_label);
    free(false_label);
    free(start_body_label);
}

// operations such as plus, plus_equal, etc, are terminated by semicolon
void handle_int_operations(struct TokenNode* node, CodeBuffer* buffer, SymbolTable* table) {
    if (node->children_count < 3 || node->children_count > 4) {
        fprintf(stderr, "Invalid integer operation '%s' on line: %d\n", node->token_data->data, node->token_data->line);
        exit(EXIT_FAILURE);
    }

    struct TokenNode* var = &node->children[0];
    struct TokenNode* operand_one = &node->children[1];
    struct TokenNode* operand_two = (node->children_count == 4) ? &node->children[2] : NULL;

    if (var->token_data->type != IDENTIFIER) {
        fprintf(stderr, "Left-hand side of operation '%s' on line: %d must be a variable\n", node->token_data->data, node->token_data->line);
        exit(EXIT_FAILURE);
    }

    Symbol* sym = find_symbol(table, var->token_data->data);
    if (!sym) {
        fprintf(stderr, "Undefined variable '%s' in operation on line: %d\n", var->token_data->data, node->token_data->line);
        exit(EXIT_FAILURE);
    }

    if (sym->type != INT_TYPE) {
        fprintf(stderr, "Operation '%s' on line: %d is only for integers\n", node->token_data->data, node->token_data->line);
        exit(EXIT_FAILURE);
    }

    // load lhs variable
    append_to_buffer(buffer, 0, "la $t0, %s\n", sym->label);
    append_to_buffer(buffer, 0, "lw $t1, 0($t0)\n");

    // load the operand(s)
    // OP 1
    if (operand_one->token_data->type == INT_LITERAL) {
        append_to_buffer(buffer, 0, "li $t2, %s\n", operand_one->token_data->data);
    } else if (operand_one->token_data->type == IDENTIFIER) {
        Symbol* operand_one_sym = find_symbol(table, operand_one->token_data->data);
        if (!operand_one_sym) {
            fprintf(stderr, "Undefined operand variable '%s' on line: %d\n", operand_one->token_data->data, node->token_data->line);
            exit(EXIT_FAILURE);
        }
        if (operand_one_sym->type != INT_TYPE) {
            fprintf(stderr, "Operand '%s' on line: %d must be an integer\n", operand_one->token_data->data, node->token_data->line);
            exit(EXIT_FAILURE);
        }
        append_to_buffer(buffer, 0, "la $t2, %s\n", operand_one_sym->label);
        append_to_buffer(buffer, 0, "lw $t2, 0($t2)\n");
    }

    // for += and -=, there's only one operand
    if (node->token_data->type == PLUS_EQUAL || node->token_data->type == MINUS_EQUAL) {
        // perform the operation
        if (node->token_data->type == PLUS_EQUAL) {
            append_to_buffer(buffer, 0, "add $t1, $t1, $t2\n");
        } else if (node->token_data->type == MINUS_EQUAL) {
            append_to_buffer(buffer, 0, "sub $t1, $t1, $t2\n");
        }
        append_to_buffer(buffer, 0, "sw $t1, 0($t0)\n");
        return;
    }


    if (operand_two == NULL){
        fprintf(stderr, "Did not provide a second operand for operation '%s' on line: %d\n", node->token_data->data, node->token_data->line);
        exit(EXIT_FAILURE);
    }

    // for +, -, *, /, there are two operands
    // OP 2 into $t3
    if (operand_two->token_data->type == INT_LITERAL) {
        append_to_buffer(buffer, 0, "li $t3, %s\n", operand_two->token_data->data);
    } else if (operand_two->token_data->type == IDENTIFIER) {
        Symbol* operand_two_sym = find_symbol(table, operand_two->token_data->data);
        if (!operand_two_sym) {
            fprintf(stderr, "Undefined operand variable '%s' on line: %d\n", operand_two->token_data->data, node->token_data->line);
            exit(EXIT_FAILURE);
        }
        if (operand_two_sym->type != INT_TYPE) {
            fprintf(stderr, "Operand '%s' on line: %d must be an integer\n", operand_two->token_data->data, node->token_data->line);
            exit(EXIT_FAILURE);
        }
        append_to_buffer(buffer, 0, "la $t3, %s\n", operand_two_sym->label);
        append_to_buffer(buffer, 0, "lw $t3, 0($t3)\n");
    }

    // perform the operation
    switch (node->token_data->type) {
        case PLUS: append_to_buffer(buffer, 0, "add $t1, $t2, $t3\n"); break;
        case MINUS: append_to_buffer(buffer, 0, "sub $t1, $t2, $t3\n"); break;
        case STAR: append_to_buffer(buffer, 0, "mul $t1, $t2, $t3\n"); break; // use pseudo for 32 bit maximum result
        case SLASH:
            append_to_buffer(buffer, 0, "div $t2, $t3\n");
            append_to_buffer(buffer, 0, "mflo $t1\n"); // we will just max it to lower 32 bits of division
            break;
        case MODULO:
            // bit mask using logical and was the first idea, but it is only an optimization for when t3 is a power of 2
            // a generalized modulo operation:
            append_to_buffer(buffer, 0, "div $t2, $t3\n");
            append_to_buffer(buffer, 0, "mfhi $t1\n");
            break;
        case BIT_AND:
            append_to_buffer(buffer, 0, "and $t1, $t2, $t3\n");
            break;
        case BIT_OR:
            append_to_buffer(buffer, 0, "or $t1, $t2, $t3\n");
            break;
        default:
            fprintf(stderr, "Unsupported arithmetic operation '%s' on line: %d\n", node->token_data->data, node->token_data->line);
            exit(EXIT_FAILURE);
    }
    // store result
    append_to_buffer(buffer, 0, "sw $t1, 0($t0)\n");
}

// delegate functions based on node token type
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
        case PLUS:
        case MINUS:
        case SLASH:
        case STAR:
        case MODULO:
        case PLUS_EQUAL:
        case MINUS_EQUAL:
        case BIT_AND:
        case BIT_OR:
            handle_int_operations(node, buffer, table);
            break;
        case IF:
            handle_if_statement(node, buffer, table);
            break;
        case WHILE:
            handle_while_loop(node, buffer, table);
            break;
        default: break;
    }
}

// TODO: should be adding function to symbol table, specifying what is allocated to stack
void handle_function(struct TokenNode* node, CodeBuffer* buffer, SymbolTable* table) {
    if (node->children_count < 2 || node->children[0].token_data->type != OPEN_PAREN ||
            node->children[1].token_data->type != OPEN_BRACE) return;

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
            append_to_buffer(buffer, 0, "jr $ra\n\n");
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
