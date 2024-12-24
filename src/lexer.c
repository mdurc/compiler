
#include "lexer.h"

Keyword keywords[] = {
    {"and", AND}, {"or", OR}, {"if", IF}, {"else", ELSE}, {"true", TRUE}, {"false", FALSE},
    {"for", FOR}, {"while", WHILE}, {"int", INT}, {"main", MAIN}, {"print", PRINT}, {"return", RETURN},
};

TokenType check_keyword(const char* str) {
    int i;
    for (i = 0; i < (int)(sizeof(keywords) / sizeof(Keyword)); ++i) {
        if (strcmp(str, keywords[i].keyword) == 0) {
            return keywords[i].type;
        }
    }
    return IDENTIFIER;
}

Token* create_token(TokenType type, const char* data, int line){
    Token* ret = (Token*) malloc(sizeof(Token));
    ret->type = type;
    ret->data = strdup(data);
    ret->line = line;
    return ret;
}

void add_token(Token** tokens, int* token_count, int* token_capacity, Token* token){
    if (*token_count >= *token_capacity) {
        *token_capacity *= 2;
        *tokens = realloc(*tokens, sizeof(Token) * (*token_capacity));
        if (*tokens == NULL) {
            fprintf(stderr, "Realloc Failed\n");
            exit(EXIT_FAILURE);
        }
    }
    // put content of token in array of tokens
    (*tokens)[*token_count] = *token;
    ++(*token_count);
    free(token); // free the token (but not the char* within it)
    token = NULL;
}

void lex_file(FILE* fp, Token** tokens, int* token_count, int* token_capacity){
    if (*token_capacity != 0) exit(EXIT_FAILURE);

    *token_capacity = 100;
    *token_count = 0;
    *tokens = (Token*) malloc(sizeof(Token) * (*token_capacity));

    char buf[256] = {'\0'};
    int line = 1;

    char c;
    while ((c = getc(fp)) != EOF) {
        if (isspace(c)) {
            if (c == '\n') ++line;
            continue;
        }

        switch (c) {
            case '(': add_token(tokens, token_count, token_capacity, create_token(OPEN_PAREN, "(", line)); break;
            case ')': add_token(tokens, token_count, token_capacity, create_token(CLOSE_PAREN, ")", line)); break;
            case '{': add_token(tokens, token_count, token_capacity, create_token(OPEN_BRACE, "{", line)); break;
            case '}': add_token(tokens, token_count, token_capacity, create_token(CLOSE_BRACE, "}", line)); break;
            case '[': add_token(tokens, token_count, token_capacity, create_token(OPEN_BRACKET, "[", line)); break;
            case ']': add_token(tokens, token_count, token_capacity, create_token(CLOSE_BRACKET, "]", line)); break;
            case ',': add_token(tokens, token_count, token_capacity, create_token(COMMA, ",", line)); break;
            case '.': add_token(tokens, token_count, token_capacity, create_token(DOT, ".", line)); break;
            case '-': add_token(tokens, token_count, token_capacity, create_token(MINUS, "-", line)); break;
            case '+': add_token(tokens, token_count, token_capacity, create_token(PLUS, "+", line)); break;
            case '/': add_token(tokens, token_count, token_capacity, create_token(SLASH, "/", line)); break;
            case '*': add_token(tokens, token_count, token_capacity, create_token(STAR, "*", line)); break;
            case '!': 
                // check for != or just !
                if ((c = getc(fp)) == '=') {
                    add_token(tokens, token_count, token_capacity, create_token(BANG_EQUAL, "!=", line));
                }else{
                    ungetc(c, fp);
                    add_token(tokens, token_count, token_capacity, create_token(BANG, "!", line));
                }
                break;
            case '=':
                // check for == or just =
                if ((c = getc(fp)) == '=') {
                    add_token(tokens, token_count, token_capacity, create_token(EQUAL_EQUAL, "==", line));
                }else{
                    ungetc(c, fp);
                    add_token(tokens, token_count, token_capacity, create_token(EQUAL, "=", line));
                }
                break;
            case '>':
                // check for >= or just >
                if ((c = getc(fp)) == '=') {
                    add_token(tokens, token_count, token_capacity, create_token(GREATER_EQUAL, ">=", line));
                }else{
                    ungetc(c, fp);
                    add_token(tokens, token_count, token_capacity, create_token(GREATER, ">", line));
                }
                break;
            case '<':
                // check for <= or just <
                if ((c = getc(fp)) == '=') {
                    add_token(tokens, token_count, token_capacity, create_token(LESS_EQUAL, "<=", line));
                }else{
                    ungetc(c, fp);
                    add_token(tokens, token_count, token_capacity, create_token(LESS, "<", line));
                }
                break;
            case '"':
                // string
                buf[0] = c;
                int i = 1;
                while ((c = getc(fp))) {
                    buf[i] = c;
                    ++i;
                    if (c == '"') break;
                }
                buf[i] = '\0';
                add_token(tokens, token_count, token_capacity, create_token(STRING, buf, line));
                break;
            default:
                buf[0] = c;
                if (isalpha(c)){
                    // identifier or keyword or main
                    int i = 1;
                    while (isalnum(c = getc(fp)) || c == '_') {
                        buf[i] = c;
                        ++i;
                    }
                    buf[i] = '\0';
                    ungetc(c, fp); // put the ending char back into file stream
                    TokenType type = check_keyword(buf);
                    add_token(tokens, token_count, token_capacity, create_token(type, buf, line));
                } else if (isdigit(c)) {
                    // number
                    int i = 1;
                    while (isdigit(c = getc(fp))) {
                        buf[i] = c;
                        ++i;
                    }
                    buf[i] = '\0';
                    ungetc(c, fp);
                    add_token(tokens, token_count, token_capacity, create_token(INT, buf, line));
                } else {
                    fprintf(stderr, "Illegal token: '%c' on line %d\n", c, line);
                    exit(EXIT_FAILURE);
                }
                break;
        }
    }
}
