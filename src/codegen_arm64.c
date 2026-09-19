#include "codegen_arm64.h"
#include "util.h"
#include <string.h>
#include <stdarg.h>
#include <limits.h>

static int new_label(Codegen *cg) {
    return cg->label_count++;
}

static void emit(Codegen *cg, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(cg->out, "    ");
    vfprintf(cg->out, fmt, args);
    fprintf(cg->out, "\n");
    va_end(args);
}

static void emit_label(Codegen *cg, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(cg->out, fmt, args);
    fprintf(cg->out, ":\n");
    va_end(args);
}

static int find_var(Codegen *cg, const char *name) {
    for (int i = cg->var_count - 1; i >= 0; i--) {
        if (strcmp(cg->vars[i].name, name) == 0) {
            return cg->vars[i].offset;
        }
    }
    return INT_MIN;
}

static int add_var(Codegen *cg, const char *name) {
    int offset = -8 * (cg->var_count + 1);
    cg->vars[cg->var_count].name = strdup(name);
    cg->vars[cg->var_count].offset = offset;
    cg->var_count++;
    return offset;
}

static struct { int id; char *str; } str_table[1024];
static int str_count = 0;

static int add_string(Codegen *cg, const char *s) {
    (void)cg;
    for (int i = 0; i < str_count; i++) {
        if (strcmp(str_table[i].str, s) == 0) return str_table[i].id;
    }
    str_table[str_count].id = str_count;
    str_table[str_count].str = strdup(s);
    str_count++;
    return str_count - 1;
}

static void collect_strings_node(AstNode *node);

static void collect_strings_stmts(AstNode *node) {
    if (!node) return;
    if (node->type == N_STMTS) {
        for (int i = 0; i < node->as.stmts.count; i++)
            collect_strings_node(node->as.stmts.stmts[i]);
    } else {
        collect_strings_node(node);
    }
}

static void collect_strings_node(AstNode *node) {
    if (!node) return;
    switch (node->type) {
        case N_SHUCHU: {
            AstNode *arg = node->as.shuchu.arg;
            if (arg->type == N_VAR && arg->as.var.name[0] == '"') {
                add_string(NULL, arg->as.var.name);
            }
            break;
        }
        case N_RUGUO:
            collect_strings_stmts(node->as.ruguo.then_body);
            collect_strings_stmts(node->as.ruguo.else_body);
            break;
        case N_DANG:
            collect_strings_stmts(node->as.dang.body);
            break;
        case N_FUNC_DEF:
            collect_strings_stmts(node->as.func_def.body);
            break;
        case N_PROG:
            for (int i = 0; i < node->as.prog.count; i++)
                collect_strings_node(node->as.prog.funcs[i]);
            break;
        default:
            break;
    }
}

static void gen_expr(Codegen *cg, AstNode *node);
static void gen_stmt(Codegen *cg, AstNode *node);

static void gen_stmts(Codegen *cg, AstNode *node) {
    if (!node) return;
    if (node->type == N_STMTS) {
        for (int i = 0; i < node->as.stmts.count; i++)
            gen_stmt(cg, node->as.stmts.stmts[i]);
    } else {
        gen_stmt(cg, node);
    }
}

static int count_local_vars(AstNode *node);

static void gen_expr(Codegen *cg, AstNode *node) {
    switch (node->type) {
        case N_NUM:
            emit(cg, "mov w0, #%d", node->as.num_val);
            break;
        case N_VAR: {
            int off = find_var(cg, node->as.var.name);
            if (off == INT_MIN) {
                error("未定义的变量 '%s', 位置 %d:%d",
                      node->as.var.name, node->line, node->col);
            }
            emit(cg, "ldr w0, [x29, #%d]", off);
            break;
        }
        case N_BINOP:
            gen_expr(cg, node->as.binop.left);
            emit(cg, "str w0, [sp, #-16]!");
            gen_expr(cg, node->as.binop.right);
            emit(cg, "ldr w1, [sp], #16");
            switch (node->as.binop.op) {
                case TOK_PLUS:  emit(cg, "add w0, w1, w0"); break;
                case TOK_MINUS: emit(cg, "sub w0, w1, w0"); break;
                case TOK_STAR:  emit(cg, "mul w0, w1, w0"); break;
                case TOK_SLASH:
                    emit(cg, "sxtw x0, w0");
                    emit(cg, "sxtw x1, w1");
                    emit(cg, "sdiv x0, x1, x0");
                    break;
                case TOK_EQ:  emit(cg, "cmp w1, w0"); emit(cg, "cset w0, eq"); break;
                case TOK_NEQ: emit(cg, "cmp w1, w0"); emit(cg, "cset w0, ne"); break;
                case TOK_LT:  emit(cg, "cmp w1, w0"); emit(cg, "cset w0, lt"); break;
                case TOK_GT:  emit(cg, "cmp w1, w0"); emit(cg, "cset w0, gt"); break;
                case TOK_LE:  emit(cg, "cmp w1, w0"); emit(cg, "cset w0, le"); break;
                case TOK_GE:  emit(cg, "cmp w1, w0"); emit(cg, "cset w0, ge"); break;
                default:
                    error("不支持的运算符, 位置 %d:%d", node->line, node->col);
            }
            break;
        case N_UNARY:
            gen_expr(cg, node->as.unary.operand);
            if (node->as.unary.op == TOK_MINUS) {
                emit(cg, "neg w0, w0");
            }
            break;
        case N_FUNC_CALL: {
            int nargs = node->as.func_call.arg_count;
            if (nargs > 8) {
                error("暂不支持超过8个参数, 位置 %d:%d", node->line, node->col);
            }
            for (int i = 0; i < nargs; i++) {
                gen_expr(cg, node->as.func_call.args[i]);
                emit(cg, "mov x%d, x0", i);
            }
            emit(cg, "bl %s", node->as.func_call.name);
            break;
        }
        default:
            error("无效的表达式节点, 位置 %d:%d", node->line, node->col);
    }
}

static int return_label = -1;

static void gen_stmt(Codegen *cg, AstNode *node) {
    switch (node->type) {
        case N_LING: {
            int off = add_var(cg, node->as.ling.name);
            gen_expr(cg, node->as.ling.init);
            emit(cg, "str w0, [x29, #%d]", off);
            break;
        }
        case N_ASSIGN: {
            int off = find_var(cg, node->as.assign.name);
            if (off == INT_MIN) {
                error("未定义的变量 '%s', 位置 %d:%d",
                      node->as.assign.name, node->line, node->col);
            }
            gen_expr(cg, node->as.assign.value);
            emit(cg, "str w0, [x29, #%d]", off);
            break;
        }
        case N_SHUCHU: {
            AstNode *arg = node->as.shuchu.arg;
            if (arg->type == N_VAR && arg->as.var.name[0] == '"') {
                int sid = add_string(cg, arg->as.var.name);
                emit(cg, "adrp x0, .LC%d", sid);
                emit(cg, "add x0, x0, :lo12:.LC%d", sid);
                emit(cg, "bl puts");
            } else {
                gen_expr(cg, arg);
                emit(cg, "sxtw x1, w0");
                emit(cg, "adrp x0, .LCint");
                emit(cg, "add x0, x0, :lo12:.LCint");
                emit(cg, "bl printf");
            }
            break;
        }
        case N_RUGUO: {
            int lbl = new_label(cg);
            gen_expr(cg, node->as.ruguo.cond);
            if (node->as.ruguo.else_body) {
                emit(cg, "cbz w0, .Lelse%d", lbl);
                gen_stmts(cg, node->as.ruguo.then_body);
                emit(cg, "b .Lend%d", lbl);
                emit_label(cg, ".Lelse%d", lbl);
                gen_stmts(cg, node->as.ruguo.else_body);
                emit_label(cg, ".Lend%d", lbl);
            } else {
                emit(cg, "cbz w0, .Lend%d", lbl);
                gen_stmts(cg, node->as.ruguo.then_body);
                emit_label(cg, ".Lend%d", lbl);
            }
            break;
        }
        case N_DANG: {
            int lbl = new_label(cg);
            emit_label(cg, ".Lloop%d", lbl);
            gen_expr(cg, node->as.dang.cond);
            emit(cg, "cbz w0, .Lloopend%d", lbl);
            gen_stmts(cg, node->as.dang.body);
            emit(cg, "b .Lloop%d", lbl);
            emit_label(cg, ".Lloopend%d", lbl);
            break;
        }
        case N_FANHUI:
            gen_expr(cg, node->as.fanhui.value);
            emit(cg, "b .Lreturn%d", return_label);
            break;
        case N_FUNC_CALL:
            gen_expr(cg, node);
            break;
        default:
            error("无效的语句节点, 位置 %d:%d", node->line, node->col);
    }
}

static int count_local_vars(AstNode *node) {
    if (!node) return 0;
    switch (node->type) {
        case N_LING: return 1;
        case N_STMTS: {
            int n = 0;
            for (int i = 0; i < node->as.stmts.count; i++)
                n += count_local_vars(node->as.stmts.stmts[i]);
            return n;
        }
        case N_RUGUO:
            return count_local_vars(node->as.ruguo.then_body) +
                   count_local_vars(node->as.ruguo.else_body);
        case N_DANG:
            return count_local_vars(node->as.dang.body);
        case N_FUNC_DEF:
            return count_local_vars(node->as.func_def.body);
        default: return 0;
    }
}

static void gen_func(Codegen *cg, AstNode *func) {
    cg->var_count = 0;
    cg->func_name = func->as.func_def.name;
    str_count = 0;

    int param_count = func->as.func_def.param_count;
    int local_vars = count_local_vars(func->as.func_def.body);
    int total = local_vars + param_count;
    if (total < 2) total = 2;
    if (total % 2 != 0) total++;
    return_label = new_label(cg);

    emit(cg, "");
    emit(cg, ".globl %s", cg->func_name);
    emit(cg, ".type %s, %%function", cg->func_name);
    emit_label(cg, "%s", cg->func_name);
    emit(cg, "stp x29, x30, [sp, #-16]!");
    emit(cg, "mov x29, sp");
    emit(cg, "sub sp, sp, #%d", 8 * total);

    for (int i = 0; i < param_count; i++) {
        int off = add_var(cg, func->as.func_def.params[i]);
        emit(cg, "str x%d, [x29, #%d]", i, off);
    }

    gen_stmts(cg, func->as.func_def.body);

    emit_label(cg, ".Lreturn%d", return_label);
    emit(cg, "mov sp, x29");
    emit(cg, "ldp x29, x30, [sp], #16");
    emit(cg, "ret");
    emit(cg, ".size %s, .-%s", cg->func_name, cg->func_name);
}

void codegen_init(Codegen *cg, FILE *out) {
    cg->out = out;
    cg->label_count = 0;
    cg->stack_size = 0;
    cg->var_count = 0;
    cg->func_name = NULL;
}

void codegen_program(Codegen *cg, AstNode *prog) {
    str_count = 0;
    collect_strings_node(prog);

    fprintf(cg->out, "    .section .text\n");
    fprintf(cg->out, "    .align 2\n");

    for (int i = 0; i < prog->as.prog.count; i++) {
        gen_func(cg, prog->as.prog.funcs[i]);
    }

    fprintf(cg->out, "\n    .section .rodata\n");
    fprintf(cg->out, "    .align 3\n");
    fprintf(cg->out, ".LCint:\n");
    fprintf(cg->out, "    .string \"%%ld\\n\"\n");

    for (int i = 0; i < str_count; i++) {
        fprintf(cg->out, ".LC%d:\n", i);
        fprintf(cg->out, "    .string %s\n", str_table[i].str);
    }
}
