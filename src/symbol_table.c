
#include "symbol_table.h"

void init_symbol_table(SymbolTable* table) {
    table->count = 0;
    table->capacity = 16;
    table->symbols = (Symbol*)malloc(table->capacity * sizeof(Symbol));
}

void add_symbol(SymbolTable* table, const char* name, const char* label, TokenType type, const char* value) {
    // check if symbol is already in table (duplicate varaible)
    if (find_symbol(table, name)){
        fprintf(stderr, "Error: Symbol '%s' already declared.\n", name);
        exit(EXIT_FAILURE);
    }

    if (table->count >= table->capacity) {
        table->capacity *= 2;
        table->symbols = (Symbol*)realloc(table->symbols, table->capacity * sizeof(Symbol));
    }
    Symbol* symbol = &table->symbols[table->count];
    ++table->count;

    strncpy(symbol->name, name, sizeof(symbol->name));
    strncpy(symbol->label, label, sizeof(symbol->label));
    symbol->type = type;
    strncpy(symbol->value, value, sizeof(symbol->value));
}

Symbol* find_symbol(SymbolTable* table, const char* name) {
    for (int i = 0; i < table->count; ++i) {
        if (strcmp(table->symbols[i].name, name) == 0) {
            return &table->symbols[i];
        }
    }
    return NULL;
}

void free_symbol_table(SymbolTable* table) {
    free(table->symbols);
    table->symbols = NULL;
    table->count = table->capacity = 0;
}
