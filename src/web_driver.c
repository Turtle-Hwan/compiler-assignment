/* Mini-JS Web Driver
 * Emscripten을 통해 웹에서 컴파일러를 실행
 */

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "codegen_x86.h"
#include "eval.h"

/* 외부 파서 함수 */
extern int yyparse(void);
extern void yy_scan_string_custom(const char *str);
extern void yy_reset_input(void);

/* 전역 프로그램 (parser.y에서 설정) */
extern FunctionList *g_program;

/* 결과 버퍼 */
#define RESULT_BUFSIZE 65536
static char result_buffer[RESULT_BUFSIZE];
static char ast_output_buffer[RESULT_BUFSIZE];
static char asm_buffer[RESULT_BUFSIZE];
static char exec_buffer[RESULT_BUFSIZE];

/* 버퍼에 문자열 추가 */
static void append_to_buffer(char *buf, int bufsize, int *pos, const char *str) {
    int len = strlen(str);
    int remaining = bufsize - *pos - 1;
    if (len > remaining) len = remaining;
    if (len > 0) {
        memcpy(buf + *pos, str, len);
        *pos += len;
        buf[*pos] = '\0';
    }
}

/* JavaScript 코드 컴파일 및 실행 */
EMSCRIPTEN_KEEPALIVE
const char *compile_mini_js(const char *js_code) {
    int result_pos = 0;
    result_buffer[0] = '\0';
    asm_buffer[0] = '\0';
    exec_buffer[0] = '\0';

    if (!js_code || strlen(js_code) == 0) {
        strcpy(result_buffer, "Error: Empty input\n");
        return result_buffer;
    }

    /* 이전 프로그램 해제 */
    if (g_program) {
        free_program(g_program);
        g_program = NULL;
    }

    /* 문자열에서 파싱 */
    yy_scan_string_custom(js_code);

    if (yyparse() != 0) {
        yy_reset_input();
        strcpy(result_buffer, "=== Parse Error ===\nFailed to parse the input code.\n");
        return result_buffer;
    }

    yy_reset_input();

    if (!g_program) {
        strcpy(result_buffer, "=== Error ===\nNo program parsed.\n");
        return result_buffer;
    }

    /* AST 시각화 */
    append_to_buffer(result_buffer, RESULT_BUFSIZE, &result_pos,
                     "=== AST ===\n");

    ast_output_buffer[0] = '\0';
    int ast_len = ast_to_buffer(g_program, ast_output_buffer, RESULT_BUFSIZE);
    if (ast_len > 0) {
        append_to_buffer(result_buffer, RESULT_BUFSIZE, &result_pos, ast_output_buffer);
    } else {
        append_to_buffer(result_buffer, RESULT_BUFSIZE, &result_pos,
                         "(No AST generated)\n");
    }

    /* 어셈블리 코드 생성 */
    append_to_buffer(result_buffer, RESULT_BUFSIZE, &result_pos,
                     "\n=== x86-64 Assembly ===\n");

    int asm_len = gen_x86_to_buffer(g_program, asm_buffer, RESULT_BUFSIZE);
    if (asm_len > 0) {
        append_to_buffer(result_buffer, RESULT_BUFSIZE, &result_pos, asm_buffer);
    } else {
        append_to_buffer(result_buffer, RESULT_BUFSIZE, &result_pos,
                         "(Assembly generation failed)\n");
    }

    /* 인터프리터 실행 */
    append_to_buffer(result_buffer, RESULT_BUFSIZE, &result_pos,
                     "\n=== Execution Result ===\n");

    eval_set_output_buffer(exec_buffer, RESULT_BUFSIZE);
    int ret = eval_program(g_program);
    eval_reset_output_buffer();

    /* 실행 출력 추가 */
    if (strlen(exec_buffer) > 0) {
        append_to_buffer(result_buffer, RESULT_BUFSIZE, &result_pos, exec_buffer);
    }

    /* 반환값 추가 */
    char ret_str[64];
    snprintf(ret_str, sizeof(ret_str), "\nReturn Value: %d\n", ret);
    append_to_buffer(result_buffer, RESULT_BUFSIZE, &result_pos, ret_str);

    /* 메모리 해제 */
    free_program(g_program);
    g_program = NULL;

    return result_buffer;
}

/* 어셈블리만 생성 */
EMSCRIPTEN_KEEPALIVE
const char *compile_to_asm(const char *js_code) {
    asm_buffer[0] = '\0';

    if (!js_code || strlen(js_code) == 0) {
        strcpy(asm_buffer, "; Error: Empty input\n");
        return asm_buffer;
    }

    if (g_program) {
        free_program(g_program);
        g_program = NULL;
    }

    yy_scan_string_custom(js_code);

    if (yyparse() != 0) {
        yy_reset_input();
        strcpy(asm_buffer, "; Parse Error\n");
        return asm_buffer;
    }

    yy_reset_input();

    if (!g_program) {
        strcpy(asm_buffer, "; No program\n");
        return asm_buffer;
    }

    gen_x86_to_buffer(g_program, asm_buffer, RESULT_BUFSIZE);

    free_program(g_program);
    g_program = NULL;

    return asm_buffer;
}

/* 인터프리터만 실행 */
EMSCRIPTEN_KEEPALIVE
const char *execute_mini_js(const char *js_code) {
    int result_pos = 0;
    exec_buffer[0] = '\0';

    if (!js_code || strlen(js_code) == 0) {
        strcpy(exec_buffer, "Error: Empty input\n");
        return exec_buffer;
    }

    if (g_program) {
        free_program(g_program);
        g_program = NULL;
    }

    yy_scan_string_custom(js_code);

    if (yyparse() != 0) {
        yy_reset_input();
        strcpy(exec_buffer, "Parse Error\n");
        return exec_buffer;
    }

    yy_reset_input();

    if (!g_program) {
        strcpy(exec_buffer, "No program\n");
        return exec_buffer;
    }

    /* 실행 결과용 임시 버퍼 */
    char temp_buf[RESULT_BUFSIZE];
    temp_buf[0] = '\0';

    eval_set_output_buffer(temp_buf, RESULT_BUFSIZE);
    int ret = eval_program(g_program);
    eval_reset_output_buffer();

    /* 결과 조합 */
    append_to_buffer(exec_buffer, RESULT_BUFSIZE, &result_pos, temp_buf);

    char ret_str[64];
    snprintf(ret_str, sizeof(ret_str), "Return: %d\n", ret);
    append_to_buffer(exec_buffer, RESULT_BUFSIZE, &result_pos, ret_str);

    free_program(g_program);
    g_program = NULL;

    return exec_buffer;
}

/* 버전 정보 */
EMSCRIPTEN_KEEPALIVE
const char *get_version(void) {
    return "Mini-JS Compiler v1.0.0";
}

#ifndef __EMSCRIPTEN__
/* 테스트용 메인 함수 (비-Emscripten 빌드) */
int main(int argc, char *argv[]) {
    const char *test_code =
        "function add(a, b) {\n"
        "    return a + b;\n"
        "}\n"
        "\n"
        "function main() {\n"
        "    let x = 10;\n"
        "    let y = 20;\n"
        "    let sum = add(x, y);\n"
        "    console.log(sum);\n"
        "    return sum;\n"
        "}\n";

    printf("Test Code:\n%s\n", test_code);
    printf("---\n");

    const char *result = compile_mini_js(test_code);
    printf("%s\n", result);

    return 0;
}
#endif
