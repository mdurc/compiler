#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "lexer.h"

typedef struct{
    char name[50];
    TokenType type;
    char value[50];
    // int offset; // from $sp, might be useful
} Symbol;

typedef struct {
    Symbol* symbols;
    int count;
    int capacity;
} SymbolTable;


void init_symbol_table(SymbolTable* table);
void add_symbol(SymbolTable* table, const char* name, TokenType type, const char* value);
Symbol* find_symbol(SymbolTable* table, const char* name);
void free_symbol_table(SymbolTable* table);

#endif
