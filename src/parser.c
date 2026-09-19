#include "parser.h"
#include "util.h"

static void next_token(Parser *p) {
    p->current = lexer_next(&p->lexer);
}

static Token expect(Parser *p, TokenType type) {
    Token t = p->current;
    if (t.type != type) {
        error("期望 '%s', 得到 '%s', 位置 %d:%d",
              token_type_name(type), token_type_name(t.type), t.line, t.col);
    }
    next_token(p);
    return t;
}

static int match_token(Parser *p, TokenType type) {
    if (p->current.type == type) {
        next_token(p);
        return 1;
    }
    return 0;
}

static char *tok_str(Token *t) {
    char *s = safe_alloc(t->length + 1);
    memcpy(s, t->start, t->length);
    s[t->length] = '\0';
    return s;
}

static AstNode *parse_expr(Parser *p);

static AstNode *parse_primary(Parser *p) {
    Token t = p->current;

    if (t.type == TOK_INT_LIT) {
        next_token(p);
        return ast_new_num(t.int_val, t.line, t.col);
    }

    if (t.type == TOK_STRING_LIT) {
        next_token(p);
        t.start--; /* 包含左引号 */
        t.length += 2;
        return ast_new_var(tok_str(&t), t.line, t.col);
    }

    if (t.type == TOK_MINUS) {
        next_token(p);
        AstNode *operand = parse_primary(p);
        return ast_new_unary(TOK_MINUS, operand, t.line, t.col);
    }

    if (t.type == TOK_LPAREN) {
        next_token(p);
        AstNode *expr = parse_expr(p);
        expect(p, TOK_RPAREN);
        return expr;
    }

    if (t.type == TOK_IDENT) {
        next_token(p);
        if (p->current.type == TOK_LPAREN) {
            /* 函数调用 */
            next_token(p);
            AstNode **args = NULL;
            int arg_count = 0;
            int arg_cap = 0;
            if (p->current.type != TOK_RPAREN) {
                do {
                    if (arg_count >= arg_cap) {
                        arg_cap = arg_cap ? arg_cap * 2 : 4;
                        args = realloc(args, sizeof(AstNode*) * arg_cap);
                    }
                    args[arg_count++] = parse_expr(p);
                } while (match_token(p, TOK_COMMA));
            }
            expect(p, TOK_RPAREN);
            return ast_new_func_call(tok_str(&t), args, arg_count, t.line, t.col);
        }
        return ast_new_var(tok_str(&t), t.line, t.col);
    }

    error("意外的标记 '%s', 位置 %d:%d", token_type_name(t.type), t.line, t.col);
    return NULL;
}

static AstNode *parse_mul_div(Parser *p) {
    AstNode *left = parse_primary(p);
    for (;;) {
        Token t = p->current;
        if (t.type == TOK_STAR || t.type == TOK_SLASH) {
            next_token(p);
            AstNode *right = parse_primary(p);
            left = ast_new_binop(t.type, left, right, t.line, t.col);
        } else {
            break;
        }
    }
    return left;
}

static AstNode *parse_add_sub(Parser *p) {
    AstNode *left = parse_mul_div(p);
    for (;;) {
        Token t = p->current;
        if (t.type == TOK_PLUS || t.type == TOK_MINUS) {
            next_token(p);
            AstNode *right = parse_mul_div(p);
            left = ast_new_binop(t.type, left, right, t.line, t.col);
        } else {
            break;
        }
    }
    return left;
}

static AstNode *parse_comparison(Parser *p) {
    AstNode *left = parse_add_sub(p);
    Token t = p->current;
    if (t.type == TOK_EQ || t.type == TOK_NEQ ||
        t.type == TOK_LT || t.type == TOK_GT ||
        t.type == TOK_LE || t.type == TOK_GE) {
        next_token(p);
        AstNode *right = parse_add_sub(p);
        left = ast_new_binop(t.type, left, right, t.line, t.col);
    }
    return left;
}

static AstNode *parse_expr(Parser *p) {
    return parse_comparison(p);
}

static AstNode *parse_stmts(Parser *p, TokenType end);

static AstNode *parse_stmt(Parser *p) {
    Token t = p->current;

    if (t.type == TOK_LING) {
        next_token(p);
        Token name = expect(p, TOK_IDENT);
        expect(p, TOK_ASSIGN);
        AstNode *init = parse_expr(p);
        match_token(p, TOK_SEMICOLON);
        return ast_new_ling(tok_str(&name), init, t.line, t.col);
    }

    if (t.type == TOK_RUGUO) {
        next_token(p);
        AstNode *cond = parse_expr(p);
        expect(p, TOK_LBRACE);
        AstNode *then_body = parse_stmts(p, TOK_RBRACE);
        expect(p, TOK_RBRACE);
        AstNode *else_body = NULL;
        if (p->current.type == TOK_FOUZE) {
            next_token(p);
            expect(p, TOK_LBRACE);
            else_body = parse_stmts(p, TOK_RBRACE);
            expect(p, TOK_RBRACE);
        }
        return ast_new_ruguo(cond, then_body, else_body, t.line, t.col);
    }

    if (t.type == TOK_DANG) {
        next_token(p);
        AstNode *cond = parse_expr(p);
        expect(p, TOK_LBRACE);
        AstNode *body = parse_stmts(p, TOK_RBRACE);
        expect(p, TOK_RBRACE);
        return ast_new_dang(cond, body, t.line, t.col);
    }

    if (t.type == TOK_SHUCHU) {
        next_token(p);
        expect(p, TOK_LPAREN);
        AstNode *arg = parse_expr(p);
        expect(p, TOK_RPAREN);
        match_token(p, TOK_SEMICOLON);
        return ast_new_shuchu(arg, t.line, t.col);
    }

    if (t.type == TOK_FANHUI) {
        next_token(p);
        AstNode *val = parse_expr(p);
        match_token(p, TOK_SEMICOLON);
        return ast_new_fanhui(val, t.line, t.col);
    }

    if (t.type == TOK_IDENT) {
        Token saved = t;
        next_token(p);
        if (p->current.type == TOK_LPAREN) {
            next_token(p);
            AstNode **args = NULL;
            int arg_count = 0;
            int arg_cap = 0;
            if (p->current.type != TOK_RPAREN) {
                do {
                    if (arg_count >= arg_cap) {
                        arg_cap = arg_cap ? arg_cap * 2 : 4;
                        args = realloc(args, sizeof(AstNode*) * arg_cap);
                    }
                    args[arg_count++] = parse_expr(p);
                } while (match_token(p, TOK_COMMA));
            }
            expect(p, TOK_RPAREN);
            match_token(p, TOK_SEMICOLON);
            return ast_new_func_call(tok_str(&saved), args, arg_count, saved.line, saved.col);
        }
        if (p->current.type == TOK_ASSIGN) {
            next_token(p);
            AstNode *value = parse_expr(p);
            match_token(p, TOK_SEMICOLON);
            return ast_new_assign(tok_str(&saved), value, saved.line, saved.col);
        }
        error("意外的标记 '%s', 位置 %d:%d",
              token_type_name(p->current.type), p->current.line, p->current.col);
    }

    error("意外的标记 '%s', 位置 %d:%d", token_type_name(t.type), t.line, t.col);
    return NULL;
}

static AstNode *parse_stmts(Parser *p, TokenType end) {
    AstNode **stmts = NULL;
    int count = 0;
    int cap = 0;
    while (p->current.type != end && p->current.type != TOK_EOF) {
        if (count >= cap) {
            cap = cap ? cap * 2 : 8;
            stmts = realloc(stmts, sizeof(AstNode*) * cap);
        }
        stmts[count++] = parse_stmt(p);
    }
    if (count == 1) return stmts[0];
    return ast_new_stmts(stmts, count, 0, 0);
}

static AstNode *parse_func_def(Parser *p) {
    Token t = p->current;
    expect(p, TOK_HANSHU);
    Token name = expect(p, TOK_IDENT);
    expect(p, TOK_LPAREN);

    char **params = NULL;
    int param_count = 0;
    int param_cap = 0;
    if (p->current.type != TOK_RPAREN) {
        do {
            Token pname = expect(p, TOK_IDENT);
            if (param_count >= param_cap) {
                param_cap = param_cap ? param_cap * 2 : 4;
                params = realloc(params, sizeof(char*) * param_cap);
            }
            params[param_count++] = tok_str(&pname);
        } while (match_token(p, TOK_COMMA));
    }
    expect(p, TOK_RPAREN);
    expect(p, TOK_LBRACE);

    AstNode *body = parse_stmts(p, TOK_RBRACE);
    expect(p, TOK_RBRACE);

    return ast_new_func_def(tok_str(&name), params, param_count, body, t.line, t.col);
}

AstNode *parser_parse(Parser *p) {
    next_token(p);
    AstNode **funcs = NULL;
    int count = 0;
    int cap = 0;
    while (p->current.type != TOK_EOF) {
        if (count >= cap) {
            cap = cap ? cap * 2 : 4;
            funcs = realloc(funcs, sizeof(AstNode*) * cap);
        }
        funcs[count++] = parse_func_def(p);
    }
    return ast_new_prog(funcs, count, 0, 0);
}

void parser_init(Parser *p, const char *source) {
    lexer_init(&p->lexer, source);
}
