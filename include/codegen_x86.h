#ifndef CODEGEN_X86_H
#define CODEGEN_X86_H

#include "ast.h"

/* x86-64 어셈블리 코드 생성
 * - 파일로 출력
 */
void gen_x86_program(Program *prog);

/* 문자열 버퍼로 출력 (Wasm용)
 * - buffer: 출력 버퍼
 * - bufsize: 버퍼 크기
 * - 반환: 작성된 바이트 수
 */
int gen_x86_to_buffer(Program *prog, char *buffer, int bufsize);

#endif /* CODEGEN_X86_H */
