#ifndef EVAL_H
#define EVAL_H

#include "ast.h"

/* Mini-JS 인터프리터
 * AST를 직접 실행하여 결과를 반환
 */

/* 프로그램 실행 (main 함수 찾아서 실행)
 * - prog: 함수 리스트
 * - 반환: main 함수의 반환값
 */
int eval_program(FunctionList *prog);

/* 출력 버퍼 설정 (Wasm용)
 * - buffer: 출력 버퍼
 * - bufsize: 버퍼 크기
 */
void eval_set_output_buffer(char *buffer, int bufsize);

/* 출력 버퍼 리셋 */
void eval_reset_output_buffer(void);

/* 출력 버퍼 내용 가져오기 */
const char *eval_get_output(void);

#endif /* EVAL_H */
