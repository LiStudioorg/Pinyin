#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    // 特殊
    TOK_EOF,
    TOK_ERROR,

    // 标识符和字面量
    TOK_IDENT,
    TOK_INT_LIT,
    TOK_STRING_LIT,

    // 关键字
    TOK_HANSHU,
    TOK_FANHUI,
    TOK_LING,
    TOK_RUGUO,
    TOK_FOUZE,
    TOK_DANG,
    TOK_SHUCHU,

    // 运算符
    TOK_PLUS,
    TOK_MINUS,
    TOK_STAR,
    TOK_SLASH,
    TOK_ASSIGN,
    TOK_EQ,
    TOK_NEQ,
    TOK_LT,
    TOK_GT,
    TOK_LE,
    TOK_GE,

    // 分隔符
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_COMMA,
    TOK_SEMICOLON,
} TokenType;

typedef struct {
    TokenType type;
    const char *start;
    int length;
    int line;
    int col;
    int int_val;
} Token;

#endif
