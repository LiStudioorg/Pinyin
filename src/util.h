#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

void error(const char *fmt, ...);
void *safe_alloc(size_t size);

#endif
