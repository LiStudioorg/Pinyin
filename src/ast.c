#include "ast.h"
#include "util.h"

char *str_copy(const char *s) {
    size_t len = strlen(s);
    char *p = safe_alloc(len + 1);
    memcpy(p, s, len);
    p[len] = '\0';
    return p;
}

AstNode *ast_new(NodeType type, int line, int col) {
    AstNode *n = safe_alloc(sizeof(AstNode));
    n->type = type;
    n->line = line;
    n->col = col;
    return n;
}

AstNode *ast_new_num(int32_t val, int line, int col) {
    AstNode *n = ast_new(N_NUM, line, col);
    n->as.num_val = val;
    return n;
}

AstNode *ast_new_var(const char *name, int line, int col) {
    AstNode *n = ast_new(N_VAR, line, col);
    n->as.var.name = str_copy(name);
    return n;
}

AstNode *ast_new_binop(TokenType op, AstNode *left, AstNode *right, int line, int col) {
    AstNode *n = ast_new(N_BINOP, line, col);
    n->as.binop.op = op;
    n->as.binop.left = left;
    n->as.binop.right = right;
    return n;
}

AstNode *ast_new_unary(TokenType op, AstNode *operand, int line, int col) {
    AstNode *n = ast_new(N_UNARY, line, col);
    n->as.unary.op = op;
    n->as.unary.operand = operand;
    return n;
}

AstNode *ast_new_ling(const char *name, AstNode *init, int line, int col) {
    AstNode *n = ast_new(N_LING, line, col);
    n->as.ling.name = str_copy(name);
    n->as.ling.init = init;
    return n;
}

AstNode *ast_new_assign(const char *name, AstNode *value, int line, int col) {
    AstNode *n = ast_new(N_ASSIGN, line, col);
    n->as.assign.name = str_copy(name);
    n->as.assign.value = value;
    return n;
}

AstNode *ast_new_shuchu(AstNode *arg, int line, int col) {
    AstNode *n = ast_new(N_SHUCHU, line, col);
    n->as.shuchu.arg = arg;
    return n;
}

AstNode *ast_new_func_def(const char *name, char **params, int param_count, AstNode *body, int line, int col) {
    AstNode *n = ast_new(N_FUNC_DEF, line, col);
    n->as.func_def.name = str_copy(name);
    n->as.func_def.params = params;
    n->as.func_def.param_count = param_count;
    n->as.func_def.body = body;
    return n;
}

AstNode *ast_new_func_call(const char *name, AstNode **args, int arg_count, int line, int col) {
    AstNode *n = ast_new(N_FUNC_CALL, line, col);
    n->as.func_call.name = str_copy(name);
    n->as.func_call.args = args;
    n->as.func_call.arg_count = arg_count;
    return n;
}

AstNode *ast_new_fanhui(AstNode *value, int line, int col) {
    AstNode *n = ast_new(N_FANHUI, line, col);
    n->as.fanhui.value = value;
    return n;
}

AstNode *ast_new_ruguo(AstNode *cond, AstNode *then_body, AstNode *else_body, int line, int col) {
    AstNode *n = ast_new(N_RUGUO, line, col);
    n->as.ruguo.cond = cond;
    n->as.ruguo.then_body = then_body;
    n->as.ruguo.else_body = else_body;
    return n;
}

AstNode *ast_new_dang(AstNode *cond, AstNode *body, int line, int col) {
    AstNode *n = ast_new(N_DANG, line, col);
    n->as.dang.cond = cond;
    n->as.dang.body = body;
    return n;
}

AstNode *ast_new_stmts(AstNode **stmts, int count, int line, int col) {
    AstNode *n = ast_new(N_STMTS, line, col);
    n->as.stmts.stmts = stmts;
    n->as.stmts.count = count;
    return n;
}

AstNode *ast_new_prog(AstNode **funcs, int count, int line, int col) {
    AstNode *n = ast_new(N_PROG, line, col);
    n->as.prog.funcs = funcs;
    n->as.prog.count = count;
    return n;
}
