#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "codegen_arm64.h"
#include "util.h"

static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "错误: 无法打开文件 '%s'\n", path);
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(size + 1);
    if (!buf) {
        fprintf(stderr, "错误: 内存分配失败\n");
        exit(1);
    }
    fread(buf, 1, size, f);
    buf[size] = '\0';
    fclose(f);
    return buf;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "用法: pnyc <源文件> [-o 输出文件]\n");
        return 1;
    }

    const char *input = argv[1];
    const char *output = "a.out";

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output = argv[++i];
        }
    }

    /* 汇编文件放在输出文件同目录 */
    char asm_file[1024];
    snprintf(asm_file, sizeof(asm_file), "%s.s", output);

    char *source = read_file(input);

    Parser parser;
    parser_init(&parser, source);
    AstNode *prog = parser_parse(&parser);

    FILE *f = fopen(asm_file, "w");
    if (!f) {
        fprintf(stderr, "错误: 无法创建汇编文件\n");
        return 1;
    }

    Codegen cg;
    codegen_init(&cg, f);
    codegen_program(&cg, prog);
    fclose(f);

    /* 调用 clang 汇编并链接 */
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "clang -o %s %s", output, asm_file);
    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "错误: clang 链接失败\n");
        return 1;
    }

    free(source);
    return 0;
}
