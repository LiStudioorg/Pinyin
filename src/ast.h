#ifndef AST_H
#define AST_H

#include "token.h"
#include <stdint.h>

typedef enum {
    N_NUM,
    N_VAR,
    N_BINOP,
    N_UNARY,
    N_LING,
    N_ASSIGN,
    N_SHUCHU,
    N_FUNC_DEF,
    N_FUNC_CALL,
    N_FANHUI,
    N_RUGUO,
    N_DANG,
    N_STMTS,
    N_PROG,
} NodeType;

typedef struct AstNode {
    NodeType type;
    int line;
    int col;
    union {
        /* N_NUM */
        int32_t num_val;

        /* N_VAR */
        struct { char *name; } var;

        /* N_BINOP */
        struct { TokenType op; struct AstNode *left; struct AstNode *right; } binop;

        /* N_UNARY */
        struct { TokenType op; struct AstNode *operand; } unary;

        /* N_LING: ling name = init */
        struct { char *name; struct AstNode *init; } ling;

        /* N_ASSIGN: name = expr */
        struct { char *name; struct AstNode *value; } assign;

        /* N_SHUCHU: shuchu(expr) */
        struct { struct AstNode *arg; } shuchu;

        /* N_FUNC_DEF */
        struct {
            char *name;
            char **params;
            int param_count;
            struct AstNode *body;
        } func_def;

        /* N_FUNC_CALL */
        struct {
            char *name;
            struct AstNode **args;
            int arg_count;
        } func_call;

        /* N_FANHUI */
        struct { struct AstNode *value; } fanhui;

        /* N_RUGUO */
        struct {
            struct AstNode *cond;
            struct AstNode *then_body;
            struct AstNode *else_body;
        } ruguo;

        /* N_DANG */
        struct {
            struct AstNode *cond;
            struct AstNode *body;
        } dang;

        /* N_STMTS */
        struct {
            struct AstNode **stmts;
            int count;
        } stmts;

        /* N_PROG */
        struct {
            struct AstNode **funcs;
            int count;
        } prog;
    } as;
} AstNode;

AstNode *ast_new(NodeType type, int line, int col);
AstNode *ast_new_num(int32_t val, int line, int col);
AstNode *ast_new_var(const char *name, int line, int col);
AstNode *ast_new_binop(TokenType op, AstNode *left, AstNode *right, int line, int col);
AstNode *ast_new_unary(TokenType op, AstNode *operand, int line, int col);
AstNode *ast_new_ling(const char *name, AstNode *init, int line, int col);
AstNode *ast_new_assign(const char *name, AstNode *value, int line, int col);
AstNode *ast_new_shuchu(AstNode *arg, int line, int col);
AstNode *ast_new_func_def(const char *name, char **params, int param_count, AstNode *body, int line, int col);
AstNode *ast_new_func_call(const char *name, AstNode **args, int arg_count, int line, int col);
AstNode *ast_new_fanhui(AstNode *value, int line, int col);
AstNode *ast_new_ruguo(AstNode *cond, AstNode *then_body, AstNode *else_body, int line, int col);
AstNode *ast_new_dang(AstNode *cond, AstNode *body, int line, int col);
AstNode *ast_new_stmts(AstNode **stmts, int count, int line, int col);
AstNode *ast_new_prog(AstNode **funcs, int count, int line, int col);
char *str_copy(const char *s);

#endif
