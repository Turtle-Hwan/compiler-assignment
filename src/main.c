#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "codegen_x86.h"
#include "eval.h"

/* 외부 파서 함수 */
extern int yyparse(void);
extern FILE *yyin;

/* 전역 프로그램 (parser.y에서 설정) */
extern FunctionList *g_program;

void print_usage(const char *prog) {
    fprintf(stderr, "Usage: %s [options] <input.js>\n", prog);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -e, --eval     Interpret and execute the program\n");
    fprintf(stderr, "  -c, --compile  Generate x86-64 assembly (default)\n");
    fprintf(stderr, "  -o <file>      Output file (default: out.s for compile)\n");
    fprintf(stderr, "  -h, --help     Show this help message\n");
}

int main(int argc, char *argv[]) {
    const char *input_file = NULL;
    const char *output_file = "out.s";
    int mode_eval = 0;  /* 0: compile, 1: eval */

    /* 인자 파싱 */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--eval") == 0) {
            mode_eval = 1;
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--compile") == 0) {
            mode_eval = 0;
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                output_file = argv[++i];
            } else {
                fprintf(stderr, "Error: -o requires an argument\n");
                return 1;
            }
        } else if (argv[i][0] != '-') {
            input_file = argv[i];
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    /* 입력 파일 열기 */
    if (input_file) {
        yyin = fopen(input_file, "r");
        if (!yyin) {
            fprintf(stderr, "Error: Cannot open file '%s'\n", input_file);
            return 1;
        }
    } else {
        yyin = stdin;
        fprintf(stderr, "Reading from stdin...\n");
    }

    /* 파싱 */
    g_program = NULL;
    if (yyparse() != 0) {
        fprintf(stderr, "Parse failed.\n");
        if (input_file) fclose(yyin);
        return 1;
    }

    if (input_file) fclose(yyin);

    if (!g_program) {
        fprintf(stderr, "No program parsed.\n");
        return 1;
    }

    if (mode_eval) {
        /* 인터프리터 모드 */
        printf("=== Mini-JS Interpreter ===\n");
        int result = eval_program(g_program);
        printf("=== Return Value: %d ===\n", result);
    } else {
        /* 컴파일러 모드 */
        FILE *out = fopen(output_file, "w");
        if (!out) {
            fprintf(stderr, "Error: Cannot open output file '%s'\n", output_file);
            free_program(g_program);
            return 1;
        }

        /* stdout을 임시로 변경 */
        FILE *old_stdout = stdout;
        stdout = out;

        gen_x86_program(g_program);

        stdout = old_stdout;
        fclose(out);

        printf("Assembly written to '%s'\n", output_file);
    }

    /* 메모리 해제 */
    free_program(g_program);
    g_program = NULL;

    return 0;
}
