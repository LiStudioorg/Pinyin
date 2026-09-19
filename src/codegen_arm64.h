#ifndef CODEGEN_ARM64_H
#define CODEGEN_ARM64_H

#include "ast.h"
#include <stdio.h>

typedef struct {
    FILE *out;
    int label_count;
    int stack_size;
    int var_count;
    /* 局部变量映射：名字 -> 栈偏移 */
    struct { char *name; int offset; } vars[256];
    /* 当前函数名 */
    const char *func_name;
} Codegen;

void codegen_init(Codegen *cg, FILE *out);
void codegen_program(Codegen *cg, AstNode *prog);

#endif
