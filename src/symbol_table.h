#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "lexer.h"

typedef struct{
    char name[50];
    char label[50];
    TokenType type;
    char value[50];
} Symbol;

typedef struct {
    Symbol* symbols;
    int count;
    int capacity;
} SymbolTable;


void init_symbol_table(SymbolTable* table);
void add_symbol(SymbolTable* table, const char* name, const char* label, TokenType type, const char* value);
Symbol* find_symbol(SymbolTable* table, const char* name);
void free_symbol_table(SymbolTable* table);

#endif
