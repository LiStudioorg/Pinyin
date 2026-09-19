#include "lexer.h"
#include "util.h"

static char advance(Lexer *lexer) {
    char c = *lexer->current++;
    if (c == '\n') {
        lexer->line++;
        lexer->col = 1;
    } else {
        lexer->col++;
    }
    return c;
}

static char peek(Lexer *lexer) {
    return *lexer->current;
}

static char peek_next(Lexer *lexer) {
    if (*lexer->current == '\0') return '\0';
    return lexer->current[1];
}

static int match(Lexer *lexer, char expected) {
    if (*lexer->current == expected) {
        advance(lexer);
        return 1;
    }
    return 0;
}

static int is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static int is_digit(char c) {
    return c >= '0' && c <= '9';
}

static int is_alnum(char c) {
    return is_alpha(c) || is_digit(c);
}

static Token make_token(Lexer *lexer, TokenType type) {
    Token t;
    t.type = type;
    t.start = lexer->start;
    t.length = (int)(lexer->current - lexer->start);
    t.line = lexer->line;
    t.col = lexer->col - t.length;
    t.int_val = 0;
    return t;
}

static TokenType check_keyword(const char *start, int length) {
    struct { const char *kw; int len; TokenType type; } keywords[] = {
        {"hanshu", 6, TOK_HANSHU},
        {"fanhui", 6, TOK_FANHUI},
        {"ling",   4, TOK_LING},
        {"ruguo",  5, TOK_RUGUO},
        {"fouze",  5, TOK_FOUZE},
        {"dang",   4, TOK_DANG},
        {"shuchu", 6, TOK_SHUCHU},
    };
    for (int i = 0; i < 7; i++) {
        if (length == keywords[i].len && memcmp(start, keywords[i].kw, length) == 0) {
            return keywords[i].type;
        }
    }
    return TOK_IDENT;
}

static Token read_number(Lexer *lexer) {
    while (is_digit(peek(lexer))) advance(lexer);
    Token t = make_token(lexer, TOK_INT_LIT);
    t.int_val = 0;
    const char *p = t.start;
    for (int i = 0; i < t.length; i++) {
        t.int_val = t.int_val * 10 + (p[i] - '0');
    }
    return t;
}

static Token read_string(Lexer *lexer) {
    int start_line = lexer->line;
    int start_col = lexer->col - 1;
    while (peek(lexer) != '"' && peek(lexer) != '\0') {
        if (peek(lexer) == '\\') advance(lexer);
        advance(lexer);
    }
    if (peek(lexer) == '\0') {
        error("未结束的字符串, 位置 %d:%d", start_line, start_col);
    }
    advance(lexer); // 跳过右引号
    Token t;
    t.type = TOK_STRING_LIT;
    t.start = lexer->start + 1; // 跳过左引号
    t.length = (int)(lexer->current - lexer->start - 2);
    t.line = start_line;
    t.col = start_col;
    t.int_val = 0;
    return t;
}

static Token read_ident(Lexer *lexer) {
    while (is_alnum(peek(lexer))) advance(lexer);
    Token t = make_token(lexer, TOK_IDENT);
    t.type = check_keyword(t.start, t.length);
    return t;
}

static void skip_whitespace(Lexer *lexer) {
    for (;;) {
        char c = peek(lexer);
        switch (c) {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                advance(lexer);
                break;
            case '/':
                if (peek_next(lexer) == '/') {
                    while (peek(lexer) != '\n' && peek(lexer) != '\0') advance(lexer);
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

void lexer_init(Lexer *lexer, const char *source) {
    lexer->source = source;
    lexer->start = source;
    lexer->current = source;
    lexer->line = 1;
    lexer->col = 1;
}

Token lexer_next(Lexer *lexer) {
    skip_whitespace(lexer);
    lexer->start = lexer->current;

    if (*lexer->current == '\0') {
        return make_token(lexer, TOK_EOF);
    }

    char c = advance(lexer);

    if (is_alpha(c)) return read_ident(lexer);
    if (is_digit(c)) return read_number(lexer);
    if (c == '"') return read_string(lexer);

    switch (c) {
        case '(': return make_token(lexer, TOK_LPAREN);
        case ')': return make_token(lexer, TOK_RPAREN);
        case '{': return make_token(lexer, TOK_LBRACE);
        case '}': return make_token(lexer, TOK_RBRACE);
        case ',': return make_token(lexer, TOK_COMMA);
        case ';': return make_token(lexer, TOK_SEMICOLON);
        case '+': return make_token(lexer, TOK_PLUS);
        case '*': return make_token(lexer, TOK_STAR);
        case '/': return make_token(lexer, TOK_SLASH);
        case '-': return make_token(lexer, TOK_MINUS);
        case '=':
            if (match(lexer, '=')) return make_token(lexer, TOK_EQ);
            return make_token(lexer, TOK_ASSIGN);
        case '!':
            if (match(lexer, '=')) return make_token(lexer, TOK_NEQ);
            error("意外的字符 '!', 位置 %d:%d", lexer->line, lexer->col - 1);
            break;
        case '<':
            if (match(lexer, '=')) return make_token(lexer, TOK_LE);
            return make_token(lexer, TOK_LT);
        case '>':
            if (match(lexer, '=')) return make_token(lexer, TOK_GE);
            return make_token(lexer, TOK_GT);
    }

    error("意外的字符 '%c', 位置 %d:%d", c, lexer->line, lexer->col - 1);
    Token t;
    t.type = TOK_ERROR;
    return t;
}

Token lexer_peek(Lexer *lexer) {
    Lexer saved = *lexer;
    Token t = lexer_next(lexer);
    *lexer = saved;
    return t;
}

const char *token_type_name(TokenType type) {
    switch (type) {
        case TOK_EOF: return "EOF";
        case TOK_ERROR: return "错误";
        case TOK_IDENT: return "标识符";
        case TOK_INT_LIT: return "整数";
        case TOK_STRING_LIT: return "字符串";
        case TOK_HANSHU: return "hanshu";
        case TOK_FANHUI: return "fanhui";
        case TOK_LING: return "ling";
        case TOK_RUGUO: return "ruguo";
        case TOK_FOUZE: return "fouze";
        case TOK_DANG: return "dang";
        case TOK_SHUCHU: return "shuchu";
        case TOK_PLUS: return "+";
        case TOK_MINUS: return "-";
        case TOK_STAR: return "*";
        case TOK_SLASH: return "/";
        case TOK_ASSIGN: return "=";
        case TOK_EQ: return "==";
        case TOK_NEQ: return "!=";
        case TOK_LT: return "<";
        case TOK_GT: return ">";
        case TOK_LE: return "<=";
        case TOK_GE: return ">=";
        case TOK_LPAREN: return "(";
        case TOK_RPAREN: return ")";
        case TOK_LBRACE: return "{";
        case TOK_RBRACE: return "}";
        case TOK_COMMA: return ",";
        case TOK_SEMICOLON: return ";";
    }
    return "未知";
}
