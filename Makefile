# Mini-JS Compiler Makefile
# Build for desktop and WebAssembly

# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -g -I./include -I./parser
LDFLAGS =

# Flex/Bison
FLEX = flex
BISON = bison

# Emscripten (for Wasm)
EMCC = emcc
EMFLAGS = -O2 -s WASM=1 -s EXPORTED_RUNTIME_METHODS='["cwrap","ccall"]' \
          -s EXPORTED_FUNCTIONS='["_compile_mini_js","_compile_to_asm","_execute_mini_js","_get_version"]' \
          -s ALLOW_MEMORY_GROWTH=1 \
          -I./include -I./parser

# Directories
SRC_DIR = src
PARSER_DIR = parser
INCLUDE_DIR = include
DOCS_DIR = docs
BUILD_DIR = build

# Source files (symtab.c 추가 - 10wk 기반)
SRCS = $(SRC_DIR)/ast.c $(SRC_DIR)/codegen_x86.c $(SRC_DIR)/eval.c $(SRC_DIR)/symtab.c
MAIN_SRC = $(SRC_DIR)/main.c
WEB_SRC = $(SRC_DIR)/web_driver.c

# Generated files
LEXER_C = $(PARSER_DIR)/lex.yy.c
PARSER_C = $(PARSER_DIR)/parser.tab.c
PARSER_H = $(PARSER_DIR)/parser.tab.h

# Object files (symtab.o 추가)
OBJS = $(BUILD_DIR)/ast.o $(BUILD_DIR)/codegen_x86.o $(BUILD_DIR)/eval.o \
       $(BUILD_DIR)/symtab.o $(BUILD_DIR)/lex.yy.o $(BUILD_DIR)/parser.tab.o

# Targets
TARGET = minijs
WASM_TARGET = $(DOCS_DIR)/minijs.js

.PHONY: all clean desktop wasm test

all: desktop

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Generate lexer
$(LEXER_C): $(PARSER_DIR)/scanner.l $(PARSER_H)
	$(FLEX) -o $@ $<

# Generate parser
$(PARSER_C) $(PARSER_H): $(PARSER_DIR)/parser.y
	$(BISON) -d -o $(PARSER_C) $<

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/lex.yy.o: $(LEXER_C) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/parser.tab.o: $(PARSER_C) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Desktop build
desktop: $(BUILD_DIR) $(LEXER_C) $(PARSER_C) $(OBJS) $(BUILD_DIR)/main.o
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(BUILD_DIR)/main.o $(LDFLAGS)
	@echo "Built: $(TARGET)"

$(BUILD_DIR)/main.o: $(MAIN_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# WebAssembly build
wasm: $(LEXER_C) $(PARSER_C)
	@which $(EMCC) > /dev/null 2>&1 || { \
		echo ""; \
		echo "========================================"; \
		echo "  ERROR: emcc (Emscripten) not found!"; \
		echo "========================================"; \
		echo ""; \
		echo "Emscripten is required for WebAssembly build."; \
		echo ""; \
		echo "Install Emscripten:"; \
		echo "  git clone https://github.com/emscripten-core/emsdk.git"; \
		echo "  cd emsdk"; \
		echo "  ./emsdk install latest"; \
		echo "  ./emsdk activate latest"; \
		echo "  source ./emsdk_env.sh"; \
		echo ""; \
		echo "Then run 'make wasm' again."; \
		echo ""; \
		exit 1; \
	}
	$(EMCC) $(EMFLAGS) -o $(WASM_TARGET) \
		$(SRCS) $(WEB_SRC) $(LEXER_C) $(PARSER_C)
	@echo "Built: $(WASM_TARGET)"

# Test build (web_driver standalone)
test-driver: $(LEXER_C) $(PARSER_C)
	$(CC) $(CFLAGS) -o test_driver \
		$(SRCS) $(WEB_SRC) $(LEXER_C) $(PARSER_C) $(LDFLAGS)
	@echo "Built: test_driver"

# Run tests
test: desktop
	@echo "=== Running Tests ==="
	@echo "Test 1: Basic arithmetic"
	@echo 'function main() { return 1 + 2 * 3; }' | ./$(TARGET) -e
	@echo ""
	@echo "Test 2: Variables"
	@echo 'function main() { let x = 10; let y = 20; return x + y; }' | ./$(TARGET) -e
	@echo ""
	@echo "=== Tests Complete ==="

# Clean
clean:
	rm -rf $(BUILD_DIR) $(TARGET) test_driver
	rm -f $(LEXER_C) $(PARSER_C) $(PARSER_H)
	rm -f $(DOCS_DIR)/minijs.js $(DOCS_DIR)/minijs.wasm

# Help
help:
	@echo "Mini-JS Compiler Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all       - Build desktop version (default)"
	@echo "  desktop   - Build desktop compiler"
	@echo "  wasm      - Build WebAssembly version"
	@echo "  test      - Run basic tests"
	@echo "  clean     - Remove build artifacts"
	@echo "  help      - Show this message"
	@echo ""
	@echo "Usage:"
	@echo "  make              # Build desktop version"
	@echo "  make wasm         # Build Wasm (requires emscripten)"
	@echo "  ./minijs -e file.js    # Interpret"
	@echo "  ./minijs -c file.js    # Compile to assembly"
