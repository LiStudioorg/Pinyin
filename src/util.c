#include "util.h"

void error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "错误: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(1);
}

void *safe_alloc(size_t size) {
    void *p = malloc(size);
    if (!p) {
        fprintf(stderr, "错误: 内存分配失败\n");
        exit(1);
    }
    memset(p, 0, size);
    return p;
}
