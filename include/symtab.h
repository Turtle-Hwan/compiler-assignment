/* 심볼 테이블 (10wk/minic_exec 기반 + 스코프 지원)
 * 변수 이름 → 값 매핑 관리
 */
#ifndef SYMTAB_H
#define SYMTAB_H

/* === 10wk 원본 인터페이스 === */

/* 심볼 테이블 초기화 */
void sym_init(void);

/* 변수 선언 (현재 스코프에 생성, 이미 있으면 갱신)
 * 성공 시 1, 실패 시 0 */
int sym_declare(const char *name, long value);

/* 변수 설정 (가장 가까운 스코프의 변수를 갱신, 없으면 현재 스코프에 생성)
 * 성공 시 1, 실패 시 0 */
int sym_set(const char *name, long value);

/* 변수 조회
 * 찾으면 1 (out에 값 저장), 못 찾으면 0 */
int sym_get(const char *name, long *out);

/* === 스코프 지원 (확장) === */

/* 새 스코프 시작 (함수 호출, 블록 진입 시) */
void sym_push_scope(void);

/* 스코프 종료 (함수 반환, 블록 종료 시)
 * 현재 스코프의 모든 변수 제거 */
void sym_pop_scope(void);

/* 현재 스코프 레벨 반환 */
int sym_get_scope_level(void);

#endif /* SYMTAB_H */
