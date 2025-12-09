#ifndef EVAL_H
#define EVAL_H

#include "ast.h"

/* Mini-JS 인터프리터
 * AST를 직접 실행하여 결과를 반환
 */

/* 프로그램 실행 (top-level 항목 순차 실행)
 * - prog: 프로그램 (함수 + 문장들)
 * - 반환: 실행 결과 (return문 값 또는 0)
 */
int eval_program(Program *prog);

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
