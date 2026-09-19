#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"

typedef struct {
    Lexer lexer;
    Token current;
} Parser;

void parser_init(Parser *parser, const char *source);
AstNode *parser_parse(Parser *parser);

#endif
