# pnyc - PNY 语言编译器

pnyc 是一个用 C99 编写的 PNY 语言编译器，将 `.pny` 源文件编译为 aarch64 汇编，再调用 clang 生成可执行文件。

## 编译要求

- C99 兼容的编译器（gcc 或 clang）
- aarch64 Linux 环境（如 Termux on Android）
- clang（用于最终链接）

## 编译 pnyc

```bash
make
```

或手动编译：

```bash
clang -Wall -O2 -std=c99 -o pnyc src/*.c
```

## 使用方法

```bash
./pnyc <源文件> -o <输出文件>
```

示例：

```bash
./pnyc examples/hello.pny -o hello
./hello
```

## 支持的特性

- 函数声明和调用（支持递归）
- 变量声明和赋值
- 算术运算：`+` `-` `*` `/`
- 比较运算：`==` `!=` `<` `>` `<=` `>=`
- 条件语句：`ruguo` / `fouze`
- 循环语句：`dang`
- 打印字符串和整数
- 返回值
- 注释（`//`）

## 示例程序

见 `examples/` 目录。

## 测试

```bash
make test
```

## 语言规范

详见 [LANG.md](LANG.md)。

## 许可证

MIT
