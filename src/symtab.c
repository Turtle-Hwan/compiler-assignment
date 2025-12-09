/* 심볼 테이블 (10wk/minic_exec 기반 + 스코프 지원)
 * 원본: 10wk/minic_exec/src/symtab.c
 */
#include <string.h>
#include <stdio.h>
#include "symtab.h"

#define MAX_SYMS 1024

/* 10wk 원본 구조 + scope_level 추가 */
typedef struct {
    char name[32];
    long value;
    int in_use;
    int initialized;
    int scope_level;    /* 확장: 변수가 속한 스코프 레벨 */
} Sym;

static Sym table[MAX_SYMS];
static int current_scope = 0;   /* 확장: 현재 스코프 레벨 */

/* 10wk 원본: 테이블 초기화 */
void sym_init(void)
{
    memset(table, 0, sizeof(table));
    current_scope = 0;
}

/* 10wk 원본: 이름으로 심볼 찾기 (현재 스코프부터 상위로) */
static Sym *find_sym(const char *name)
{
    /* 현재 스코프부터 상위 스코프까지 검색 */
    for (int scope = current_scope; scope >= 0; scope--) {
        for (int i = 0; i < MAX_SYMS; ++i) {
            if (table[i].in_use &&
                table[i].scope_level == scope &&
                strcmp(table[i].name, name) == 0) {
                return &table[i];
            }
        }
    }
    return NULL;
}

/* 현재 스코프에서만 심볼 찾기 (변수 섀도잉 허용) */
static Sym *find_sym_in_current_scope(const char *name)
{
    for (int i = 0; i < MAX_SYMS; ++i) {
        if (table[i].in_use &&
            table[i].scope_level == current_scope &&
            strcmp(table[i].name, name) == 0) {
            return &table[i];
        }
    }
    return NULL;
}

static Sym *create_sym_in_current_scope(const char *name)
{
    for (int i = 0; i < MAX_SYMS; ++i) {
        if (!table[i].in_use) {
            table[i].in_use = 1;
            table[i].initialized = 0;
            table[i].scope_level = current_scope;
            strncpy(table[i].name, name, sizeof(table[i].name) - 1);
            table[i].name[sizeof(table[i].name) - 1] = '\0';
            return &table[i];
        }
    }
    fprintf(stderr, "symbol table full\n");
    return NULL;
}

int sym_declare(const char *name, long value)
{
    Sym *s = find_sym_in_current_scope(name);
    if (!s) {
        s = create_sym_in_current_scope(name);
    }
    if (!s) return 0;
    s->value = value;
    s->initialized = 1;
    return 1;
}

/* 10wk 기반: 변수 설정 (수정됨)
 * 가장 가까운 스코프의 변수를 갱신하고, 없으면 현재 스코프에 생성 */
int sym_set(const char *name, long value)
{
    Sym *s = find_sym(name);
    if (!s) {
        s = create_sym_in_current_scope(name);
    }
    if (!s) return 0;
    s->value = value;
    s->initialized = 1;
    return 1;
}

/* 10wk 원본: 변수 조회 */
int sym_get(const char *name, long *out)
{
    Sym *s = find_sym(name);
    if (!s || !s->initialized) return 0;
    if (out) *out = s->value;
    return 1;
}

/* === 스코프 지원 (확장) === */

/* 새 스코프 시작 */
void sym_push_scope(void)
{
    current_scope++;
}

/* 스코프 종료: 현재 스코프의 모든 변수 제거 */
void sym_pop_scope(void)
{
    for (int i = 0; i < MAX_SYMS; ++i) {
        if (table[i].in_use && table[i].scope_level == current_scope) {
            table[i].in_use = 0;
            table[i].initialized = 0;
        }
    }
    if (current_scope > 0) {
        current_scope--;
    }
}

/* 현재 스코프 레벨 반환 */
int sym_get_scope_level(void)
{
    return current_scope;
}
