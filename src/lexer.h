#ifndef LEXER_H
#define LEXER_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum{
    OPEN_PAREN, CLOSE_PAREN, OPEN_BRACE, CLOSE_BRACE,
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,

    BANG, BANG_EQUAL, EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL, LESS, LESS_EQUAL,

    IDENTIFIER, STRING, INT, MAIN,

    AND, OR, IF, ELSE, TRUE, FALSE, FOR, WHILE, PRINT_INT,
    PRINT_STRING, RETURN
} TokenType;

typedef struct{
    TokenType type;
    char* data;
    int line;
} Token;

typedef struct {
    const char* keyword;
    TokenType type;
} Keyword;

extern Keyword keywords[];


TokenType check_keyword(const char* str);
Token* create_token(TokenType type, const char* data, int line);
void add_token(Token** tokens, int* token_count, int* token_capacity, Token* token);
void lex_file(FILE* fp, Token** tokens, int* token_count, int* token_capacity);

#endif
