/* Mini-JS 인터프리터
 * AST를 재귀적으로 순회하며 실행
 *
 * 수정: 10wk symtab.c 재사용 (Env 구조체 대신)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "ast.h"
#include "eval.h"
#include "symtab.h"  /* 10wk 기반 심볼 테이블 */

/* === 출력 버퍼 === */
static char *output_buffer = NULL;
static int output_bufsize = 0;
static int output_pos = 0;

void eval_set_output_buffer(char *buffer, int bufsize) {
    output_buffer = buffer;
    output_bufsize = bufsize;
    output_pos = 0;
    if (buffer && bufsize > 0) {
        buffer[0] = '\0';
    }
}

void eval_reset_output_buffer(void) {
    output_buffer = NULL;
    output_bufsize = 0;
    output_pos = 0;
}

const char *eval_get_output(void) {
    return output_buffer ? output_buffer : "";
}

static void print_output(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    if (output_buffer) {
        int remaining = output_bufsize - output_pos;
        if (remaining > 0) {
            int written = vsnprintf(output_buffer + output_pos, remaining, fmt, args);
            if (written > 0) {
                output_pos += (written < remaining) ? written : (remaining - 1);
            }
        }
    } else {
        vprintf(fmt, args);
    }

    va_end(args);
}

/* === 전역 함수 리스트 === */
static FunctionList *g_functions = NULL;

static Function *find_function(const char *name) {
    if (!g_functions) return NULL;
    Function *f = g_functions->head;
    while (f) {
        if (strcmp(f->name, name) == 0) return f;
        f = f->next;
    }
    return NULL;
}

/* 함수 등록 (실행 중 동적 등록) - 12wk function_list_append 재사용 */
static void register_function(Function *func) {
    if (!func) return;
    g_functions = function_list_append(g_functions, func);
}

/* === 반환값 처리 === */
typedef struct {
    int has_return;
    long return_value;
} EvalResult;

/* === 전방 선언 (10wk symtab 사용으로 Env 매개변수 제거) === */
static long eval_expr(Expr *e);
static EvalResult eval_stmt(Stmt *s);
static long eval_call(const char *func_name, ExprList *args);

/* === 표현식 평가 (10wk sym_get 사용) === */
static long eval_expr(Expr *e) {
    if (!e) return 0;

    switch (e->kind) {
        case EXPR_INT:
            return e->u.int_value;

        case EXPR_STRING:
            /* 문자열은 console.log에서 별도 처리 */
            return 0;

        case EXPR_VAR: {
            long val;
            /* 10wk symtab 인터페이스 사용 */
            if (!sym_get(e->u.var_name, &val)) {
                print_output("Error: undefined variable '%s'\n", e->u.var_name);
                return 0;
            }
            return val;
        }

        case EXPR_BINOP: {
            long lhs = eval_expr(e->u.binop.lhs);
            long rhs = eval_expr(e->u.binop.rhs);

            switch (e->u.binop.op) {
                case BIN_ADD: return lhs + rhs;
                case BIN_SUB: return lhs - rhs;
                case BIN_MUL: return lhs * rhs;
                case BIN_DIV:
                    if (rhs == 0) {
                        print_output("Error: division by zero\n");
                        return 0;
                    }
                    return lhs / rhs;
                case BIN_MOD:
                    if (rhs == 0) {
                        print_output("Error: modulo by zero\n");
                        return 0;
                    }
                    return lhs % rhs;
                case BIN_LT: return lhs < rhs ? 1 : 0;
                case BIN_GT: return lhs > rhs ? 1 : 0;
                case BIN_LE: return lhs <= rhs ? 1 : 0;
                case BIN_GE: return lhs >= rhs ? 1 : 0;
                case BIN_EQ: return lhs == rhs ? 1 : 0;
                case BIN_NE: return lhs != rhs ? 1 : 0;
                case BIN_AND: return (lhs && rhs) ? 1 : 0;
                case BIN_OR: return (lhs || rhs) ? 1 : 0;
            }
            break;
        }

        case EXPR_CALL:
            return eval_call(e->u.call.func_name, e->u.call.args);

        case EXPR_UNARY: {
            long val = eval_expr(e->u.unary.operand);
            switch (e->u.unary.op) {
                case UNARY_NEG: return -val;
                case UNARY_NOT: return val ? 0 : 1;
            }
            break;
        }
    }

    return 0;
}

/* === 함수 호출 (10wk symtab 스코프 사용) === */
static long eval_call(const char *func_name, ExprList *args) {
    Function *f = find_function(func_name);
    if (!f) {
        print_output("Error: undefined function '%s'\n", func_name);
        return 0;
    }

    /* 인자값을 먼저 평가 (현재 스코프에서) */
    long arg_values[16];
    int arg_count = 0;
    ExprList *arg = args;
    while (arg && arg_count < 16) {
        arg_values[arg_count++] = eval_expr(arg->expr);
        arg = arg->next;
    }

    /* 새 스코프 시작 (10wk symtab 확장) */
    sym_push_scope();

    /* 매개변수에 인자값 바인딩 (현재 스코프에 선언) */
    Param *param = f->params ? f->params->head : NULL;
    int i = 0;
    while (param && i < arg_count) {
        sym_declare(param->name, arg_values[i]);
        param = param->next;
        i++;
    }

    /* 함수 본문 실행 */
    long result = 0;
    if (f->body) {
        Stmt *s = f->body->head;
        while (s) {
            EvalResult r = eval_stmt(s);
            if (r.has_return) {
                result = r.return_value;
                break;
            }
            s = s->next;
        }
    }

    /* 스코프 종료 (10wk symtab 확장) */
    sym_pop_scope();

    return result;
}

/* === 문장 실행 (10wk symtab 사용) === */
static EvalResult eval_stmt(Stmt *s) {
    EvalResult result = {0, 0};
    if (!s) return result;

    switch (s->kind) {
        case STMT_VARDECL: {
            long val = 0;
            if (s->u.vardecl.init_value) {
                val = eval_expr(s->u.vardecl.init_value);
            }
            sym_declare(s->u.vardecl.var_name, val);
            break;
        }

        case STMT_ASSIGN: {
            /* 10wk sym_set 사용 */
            long val = eval_expr(s->u.assign.value);
            sym_set(s->u.assign.var_name, val);
            break;
        }

        case STMT_EXPR:
            eval_expr(s->u.expr);
            break;

        case STMT_RETURN:
            result.has_return = 1;
            result.return_value = s->u.expr ? eval_expr(s->u.expr) : 0;
            return result;

        case STMT_PRINT: {
            /* 문자열 리터럴인 경우 문자열 출력 */
            if (s->u.expr && s->u.expr->kind == EXPR_STRING) {
                print_output("%s\n", s->u.expr->u.string_value);
            } else {
                long val = eval_expr(s->u.expr);
                print_output("%ld\n", val);
            }
            break;
        }

        case STMT_IF: {
            long cond = eval_expr(s->u.if_stmt.cond);
            if (cond) {
                result = eval_stmt(s->u.if_stmt.then_stmt);
            } else if (s->u.if_stmt.else_stmt) {
                result = eval_stmt(s->u.if_stmt.else_stmt);
            }
            break;
        }

        case STMT_WHILE: {
            while (eval_expr(s->u.while_stmt.cond)) {
                result = eval_stmt(s->u.while_stmt.body);
                if (result.has_return) break;
            }
            break;
        }

        case STMT_FOR: {
            /* 새 스코프 (for문 변수용) - 10wk symtab 사용 */
            sym_push_scope();

            /* 초기화 */
            if (s->u.for_stmt.init) {
                eval_stmt(s->u.for_stmt.init);
            }

            /* 반복 */
            while (1) {
                /* 조건 (없으면 항상 true) */
                if (s->u.for_stmt.cond) {
                    long cond = eval_expr(s->u.for_stmt.cond);
                    if (!cond) break;
                }

                /* 본문 */
                result = eval_stmt(s->u.for_stmt.body);
                if (result.has_return) break;

                /* 스텝 */
                if (s->u.for_stmt.step) {
                    eval_stmt(s->u.for_stmt.step);
                }
            }

            /* 스코프 종료 */
            sym_pop_scope();
            break;
        }

        case STMT_BLOCK:
            if (s->u.block) {
                /* 새 스코프 - 10wk symtab 사용 */
                sym_push_scope();

                Stmt *curr = s->u.block->head;
                while (curr) {
                    result = eval_stmt(curr);
                    if (result.has_return) break;
                    curr = curr->next;
                }

                /* 스코프 종료 */
                sym_pop_scope();
            }
            break;
    }

    return result;
}

/* === 프로그램 실행 (순차 처리) === */
int eval_program(Program *prog) {
    if (!prog) {
        print_output("Error: No program to execute\n");
        return -1;
    }

    /* 함수 테이블 초기화 */
    g_functions = NULL;

    /* 10wk symtab 초기화 */
    sym_init();

    /* Top-level 항목 순차 처리 */
    long result = 0;
    Item *item = prog->items;

    while (item) {
        if (item->kind == ITEM_FUNCTION) {
            /* 함수 정의 → 테이블에 등록 (12wk function_list_append 재사용) */
            register_function(item->u.function);
        } else if (item->kind == ITEM_STMT) {
            /* 문장 → 즉시 실행 */
            EvalResult r = eval_stmt(item->u.stmt);
            if (r.has_return) {
                result = r.return_value;
                break;
            }
        }
        item = item->next;
    }

    g_functions = NULL;
    return (int)result;
}
