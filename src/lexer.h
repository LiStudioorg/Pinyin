#ifndef LEXER_H
#define LEXER_H

#include "token.h"

typedef struct {
    const char *source;
    const char *start;
    const char *current;
    int line;
    int col;
} Lexer;

void lexer_init(Lexer *lexer, const char *source);
Token lexer_next(Lexer *lexer);
Token lexer_peek(Lexer *lexer);
const char *token_type_name(TokenType type);

#endif
