CC = clang
CFLAGS = -Wall -Wextra -O2 -std=c99
SRC = src/main.c src/lexer.c src/parser.c src/codegen_arm64.c src/util.c
TARGET = pnyc

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET) *.o

test: $(TARGET)
	bash tests/run_tests.sh

.PHONY: all clean test
