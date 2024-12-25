#ifndef LEXER_H
#define LEXER_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum{
    OPEN_PAREN, CLOSE_PAREN, OPEN_BRACE, CLOSE_BRACE,
    OPEN_BRACKET, CLOSE_BRACKET,
    COMMA, DOT, MINUS, PLUS, SLASH, STAR, SEMICOLON,
    PLUS_EQUAL, MINUS_EQUAL, MODULO,

    BANG, BANG_EQUAL, EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL, LESS, LESS_EQUAL,

    IDENTIFIER, STRING_LITERAL, INT_LITERAL, STRING_TYPE, INT_TYPE, MAIN,

    AND, OR, IF, TRUE, FALSE, FOR, WHILE, PRINT, RETURN,

    ROOT
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

TokenType check_keyword(const char* str);
Token* create_token(TokenType type, const char* data, int line);
void add_token(Token** tokens, int* token_count, int* token_capacity, Token* token);
void lex_file(FILE* fp, Token** tokens, int* token_count, int* token_capacity);

#endif
