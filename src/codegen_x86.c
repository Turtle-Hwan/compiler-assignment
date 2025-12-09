#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "ast.h"
#include "codegen_x86.h"

/* Mini-JS x86-64 코드 생성기
 * - 함수 정의 및 호출 (12wk 기반)
 * - 제어문 if/while/for (11wk 기반)
 * - console.log() 출력
 */

/* === 출력 관련 === */
static FILE *out_file = NULL;
static char *out_buffer = NULL;
static int out_bufsize = 0;
static int out_pos = 0;

static void emit(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    if (out_buffer) {
        int remaining = out_bufsize - out_pos;
        if (remaining > 0) {
            int written = vsnprintf(out_buffer + out_pos, remaining, fmt, args);
            if (written > 0) {
                out_pos += (written < remaining) ? written : (remaining - 1);
            }
        }
    } else {
        vfprintf(out_file ? out_file : stdout, fmt, args);
    }

    va_end(args);
}

/* === 심볼 테이블 (지역 변수) === */
typedef struct {
    const char *name;
    int offset;       /* rbp 기준 음수 오프셋 */
    int is_param;
    int param_index;
} Var;

static const char *arg_regs[] = { "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9" };

static int find_var(const Var *vars, int n, const char *name) {
    for (int i = 0; i < n; ++i) {
        if (strcmp(vars[i].name, name) == 0) return i;
    }
    return -1;
}

/* 레이블 카운터 */
static int label_counter = 0;
static int string_counter = 0;

static int new_label(void) {
    return label_counter++;
}

static int new_string_label(void) {
    return string_counter++;
}

/* === 변수 수집 (스택 공간 할당) === */
static void collect_vars_expr(Expr *e);
static void collect_vars_stmt(Stmt *s, Var *vars, int *count, int *offset);

static void collect_vars_expr(Expr *e) {
    /* 표현식 내부의 변수는 이미 선언되어 있어야 함 */
    (void)e;
}

static void collect_vars_stmt(Stmt *s, Var *vars, int *count, int *offset) {
    if (!s) return;

    switch (s->kind) {
        case STMT_VARDECL:
            if (find_var(vars, *count, s->u.vardecl.var_name) < 0) {
                vars[*count].name = s->u.vardecl.var_name;
                vars[*count].offset = *offset;
                vars[*count].is_param = 0;
                vars[*count].param_index = -1;
                (*count)++;
                *offset -= 8;
            }
            break;
        case STMT_IF:
            collect_vars_stmt(s->u.if_stmt.then_stmt, vars, count, offset);
            collect_vars_stmt(s->u.if_stmt.else_stmt, vars, count, offset);
            break;
        case STMT_WHILE:
            collect_vars_stmt(s->u.while_stmt.body, vars, count, offset);
            break;
        case STMT_FOR:
            collect_vars_stmt(s->u.for_stmt.init, vars, count, offset);
            collect_vars_stmt(s->u.for_stmt.body, vars, count, offset);
            collect_vars_stmt(s->u.for_stmt.step, vars, count, offset);
            break;
        case STMT_BLOCK:
            if (s->u.block) {
                Stmt *curr = s->u.block->head;
                while (curr) {
                    collect_vars_stmt(curr, vars, count, offset);
                    curr = curr->next;
                }
            }
            break;
        default:
            break;
    }
}

static void alloc_locals(Function *f, Var *vars, int *var_count, int *stack_size) {
    int count = 0;
    int offset = -8;
    int min_offset = 0;

    /* 매개변수 먼저 */
    int param_index = 0;
    if (f->params) {
        Param *p = f->params->head;
        while (p) {
            vars[count].name = p->name;
            vars[count].offset = offset;
            vars[count].is_param = 1;
            vars[count].param_index = param_index;
            count++;
            min_offset = offset;
            offset -= 8;
            param_index++;
            p = p->next;
        }
    }

    /* 지역변수 수집 */
    if (f->body) {
        Stmt *s = f->body->head;
        while (s) {
            collect_vars_stmt(s, vars, &count, &offset);
            if (offset < min_offset) min_offset = offset;
            s = s->next;
        }
    }

    *var_count = count;
    *stack_size = (count == 0) ? 0 : -min_offset + 8;

    /* 16바이트 정렬 */
    *stack_size = (*stack_size + 15) & ~15;
}

/* === 표현식 코드 생성 === */
static void gen_expr(Expr *e, Var *vars, int var_count);

static void load_var_to_rax(const char *name, Var *vars, int var_count) {
    int idx = find_var(vars, var_count, name);
    if (idx < 0) {
        emit("    # ERROR: Unknown variable: %s\n", name);
        emit("    movq $0, %%rax\n");
        return;
    }
    emit("    movq %d(%%rbp), %%rax    # load %s\n", vars[idx].offset, name);
}

static void gen_binop(Expr *e, Var *vars, int var_count) {
    /* rhs 먼저, lhs 나중 */
    gen_expr(e->u.binop.rhs, vars, var_count);
    emit("    pushq %%rax\n");
    gen_expr(e->u.binop.lhs, vars, var_count);
    emit("    popq %%rcx\n");  /* rcx = rhs */

    switch (e->u.binop.op) {
        case BIN_ADD:
            emit("    addq %%rcx, %%rax    # add\n");
            break;
        case BIN_SUB:
            emit("    subq %%rcx, %%rax    # sub\n");
            break;
        case BIN_MUL:
            emit("    imulq %%rcx, %%rax   # mul\n");
            break;
        case BIN_DIV:
            emit("    cqto\n");
            emit("    idivq %%rcx          # div\n");
            break;
        case BIN_MOD:
            emit("    cqto\n");
            emit("    idivq %%rcx\n");
            emit("    movq %%rdx, %%rax    # mod (remainder)\n");
            break;
        case BIN_LT:
            emit("    cmpq %%rcx, %%rax\n");
            emit("    setl %%al\n");
            emit("    movzbq %%al, %%rax   # less than\n");
            break;
        case BIN_GT:
            emit("    cmpq %%rcx, %%rax\n");
            emit("    setg %%al\n");
            emit("    movzbq %%al, %%rax   # greater than\n");
            break;
        case BIN_LE:
            emit("    cmpq %%rcx, %%rax\n");
            emit("    setle %%al\n");
            emit("    movzbq %%al, %%rax   # less or equal\n");
            break;
        case BIN_GE:
            emit("    cmpq %%rcx, %%rax\n");
            emit("    setge %%al\n");
            emit("    movzbq %%al, %%rax   # greater or equal\n");
            break;
        case BIN_EQ:
            emit("    cmpq %%rcx, %%rax\n");
            emit("    sete %%al\n");
            emit("    movzbq %%al, %%rax   # equal\n");
            break;
        case BIN_NE:
            emit("    cmpq %%rcx, %%rax\n");
            emit("    setne %%al\n");
            emit("    movzbq %%al, %%rax   # not equal\n");
            break;
        case BIN_AND:
            emit("    testq %%rax, %%rax\n");
            emit("    setne %%al\n");
            emit("    testq %%rcx, %%rcx\n");
            emit("    setne %%cl\n");
            emit("    andb %%cl, %%al\n");
            emit("    movzbq %%al, %%rax   # logical and\n");
            break;
        case BIN_OR:
            emit("    orq %%rcx, %%rax\n");
            emit("    testq %%rax, %%rax\n");
            emit("    setne %%al\n");
            emit("    movzbq %%al, %%rax   # logical or\n");
            break;
        default:
            emit("    # unknown binop\n");
            break;
    }
}

static void gen_call(Expr *e, Var *vars, int var_count) {
    /* 인자 평가 및 스택에 저장 */
    int argc = 0;
    ExprList *arg = e->u.call.args;
    while (arg) {
        gen_expr(arg->expr, vars, var_count);
        emit("    pushq %%rax\n");
        argc++;
        arg = arg->next;
    }

    if (argc > 6) {
        emit("    # WARNING: More than 6 arguments not fully supported\n");
    }

    /* 스택에서 인자 레지스터로 이동 (역순) */
    for (int i = argc - 1; i >= 0; --i) {
        if (i < 6) {
            emit("    popq %s\n", arg_regs[i]);
        } else {
            emit("    popq %%rax    # extra arg discarded\n");
        }
    }

    emit("    call %s\n", e->u.call.func_name);
}

static void gen_unary(Expr *e, Var *vars, int var_count) {
    gen_expr(e->u.unary.operand, vars, var_count);

    switch (e->u.unary.op) {
        case UNARY_NEG:
            emit("    negq %%rax           # negate\n");
            break;
        case UNARY_NOT:
            emit("    testq %%rax, %%rax\n");
            emit("    sete %%al\n");
            emit("    movzbq %%al, %%rax   # logical not\n");
            break;
    }
}

/* 문자열 이스케이프 처리 (어셈블리 출력용) */
static void emit_escaped_string(const char *str) {
    emit("    .string \"");
    for (const char *p = str; *p; p++) {
        switch (*p) {
            case '\n': emit("\\n"); break;
            case '\t': emit("\\t"); break;
            case '\r': emit("\\r"); break;
            case '\\': emit("\\\\"); break;
            case '"':  emit("\\\""); break;
            default:   emit("%c", *p); break;
        }
    }
    emit("\"\n");
}

static void gen_expr(Expr *e, Var *vars, int var_count) {
    if (!e) return;

    switch (e->kind) {
        case EXPR_INT:
            emit("    movq $%d, %%rax\n", e->u.int_value);
            break;
        case EXPR_STRING:
            /* 문자열은 STMT_PRINT에서 별도 처리 */
            emit("    movq $0, %%rax    # string (handled in print)\n");
            break;
        case EXPR_VAR:
            load_var_to_rax(e->u.var_name, vars, var_count);
            break;
        case EXPR_BINOP:
            gen_binop(e, vars, var_count);
            break;
        case EXPR_CALL:
            gen_call(e, vars, var_count);
            break;
        case EXPR_UNARY:
            gen_unary(e, vars, var_count);
            break;
        default:
            emit("    # unknown expr kind\n");
            break;
    }
}

/* === 문장 코드 생성 === */
static void gen_stmt(Stmt *s, Var *vars, int var_count, const char *end_label);

static void gen_stmt(Stmt *s, Var *vars, int var_count, const char *end_label) {
    if (!s) return;

    switch (s->kind) {
        case STMT_VARDECL:
            emit("    # let %s\n", s->u.vardecl.var_name);
            if (s->u.vardecl.init_value) {
                gen_expr(s->u.vardecl.init_value, vars, var_count);
                int idx = find_var(vars, var_count, s->u.vardecl.var_name);
                if (idx >= 0) {
                    emit("    movq %%rax, %d(%%rbp)   # %s = init\n",
                         vars[idx].offset, s->u.vardecl.var_name);
                }
            }
            break;

        case STMT_ASSIGN: {
            gen_expr(s->u.assign.value, vars, var_count);
            int idx = find_var(vars, var_count, s->u.assign.var_name);
            if (idx < 0) {
                emit("    # ERROR: Unknown variable in assign: %s\n", s->u.assign.var_name);
            } else {
                emit("    movq %%rax, %d(%%rbp)   # %s = rax\n",
                     vars[idx].offset, s->u.assign.var_name);
            }
            break;
        }

        case STMT_EXPR:
            gen_expr(s->u.expr, vars, var_count);
            break;

        case STMT_RETURN:
            if (s->u.expr) {
                gen_expr(s->u.expr, vars, var_count);
            } else {
                emit("    movq $0, %%rax\n");
            }
            emit("    jmp %s\n", end_label);
            break;

        case STMT_PRINT:
            if (s->u.expr && s->u.expr->kind == EXPR_STRING) {
                /* 문자열 출력: puts 사용 */
                int str_lbl = new_string_label();
                emit("    .section .rodata\n");
                emit(".Lstr_%d:\n", str_lbl);
                emit_escaped_string(s->u.expr->u.string_value);
                emit("    .text\n");
                emit("    leaq .Lstr_%d(%%rip), %%rdi\n", str_lbl);
                emit("    call puts\n");
            } else {
                /* 정수 출력: printf 사용 */
                gen_expr(s->u.expr, vars, var_count);
                emit("    movq %%rax, %%rsi\n");
                emit("    leaq fmt_int(%%rip), %%rdi\n");
                emit("    movq $0, %%rax\n");
                emit("    call printf\n");
            }
            break;

        case STMT_IF: {
            int lbl_else = new_label();
            int lbl_end = new_label();

            gen_expr(s->u.if_stmt.cond, vars, var_count);
            emit("    cmpq $0, %%rax\n");

            if (s->u.if_stmt.else_stmt) {
                emit("    je .Lelse_%d\n", lbl_else);
                gen_stmt(s->u.if_stmt.then_stmt, vars, var_count, end_label);
                emit("    jmp .Lend_%d\n", lbl_end);
                emit(".Lelse_%d:\n", lbl_else);
                gen_stmt(s->u.if_stmt.else_stmt, vars, var_count, end_label);
                emit(".Lend_%d:\n", lbl_end);
            } else {
                emit("    je .Lend_%d\n", lbl_end);
                gen_stmt(s->u.if_stmt.then_stmt, vars, var_count, end_label);
                emit(".Lend_%d:\n", lbl_end);
            }
            break;
        }

        case STMT_WHILE: {
            int lbl_begin = new_label();
            int lbl_end = new_label();

            emit(".Lbegin_%d:\n", lbl_begin);
            gen_expr(s->u.while_stmt.cond, vars, var_count);
            emit("    cmpq $0, %%rax\n");
            emit("    je .Lend_%d\n", lbl_end);
            gen_stmt(s->u.while_stmt.body, vars, var_count, end_label);
            emit("    jmp .Lbegin_%d\n", lbl_begin);
            emit(".Lend_%d:\n", lbl_end);
            break;
        }

        case STMT_FOR: {
            int lbl_begin = new_label();
            int lbl_end = new_label();

            /* 초기화 */
            if (s->u.for_stmt.init) {
                gen_stmt(s->u.for_stmt.init, vars, var_count, end_label);
            }

            emit(".Lbegin_%d:\n", lbl_begin);

            /* 조건 (없으면 항상 true) */
            if (s->u.for_stmt.cond) {
                gen_expr(s->u.for_stmt.cond, vars, var_count);
                emit("    cmpq $0, %%rax\n");
                emit("    je .Lend_%d\n", lbl_end);
            }

            /* 본문 */
            gen_stmt(s->u.for_stmt.body, vars, var_count, end_label);

            /* 스텝 */
            if (s->u.for_stmt.step) {
                gen_stmt(s->u.for_stmt.step, vars, var_count, end_label);
            }

            emit("    jmp .Lbegin_%d\n", lbl_begin);
            emit(".Lend_%d:\n", lbl_end);
            break;
        }

        case STMT_BLOCK:
            if (s->u.block) {
                Stmt *curr = s->u.block->head;
                while (curr) {
                    gen_stmt(curr, vars, var_count, end_label);
                    curr = curr->next;
                }
            }
            break;

        default:
            emit("    # unknown stmt kind\n");
            break;
    }
}

/* === 함수 코드 생성 === */
static void gen_function(Function *f) {
    Var vars[128];
    int var_count = 0;
    int stack_size = 0;

    alloc_locals(f, vars, &var_count, &stack_size);

    int is_main = (strcmp(f->name, "main") == 0);

    emit("\n");
    emit("    .globl %s\n", f->name);
    emit("%s:\n", f->name);

    /* 프롤로그 */
    emit("    pushq %%rbp\n");
    emit("    movq %%rsp, %%rbp\n");
    if (stack_size > 0) {
        emit("    subq $%d, %%rsp\n", stack_size);
    }

    /* 매개변수를 스택에 저장 */
    for (int i = 0; i < var_count; ++i) {
        if (vars[i].is_param) {
            int pi = vars[i].param_index;
            if (pi < 6) {
                emit("    movq %s, %d(%%rbp)   # param %s\n",
                     arg_regs[pi], vars[i].offset, vars[i].name);
            }
        }
    }

    /* 종료 레이블 */
    char end_label[64];
    snprintf(end_label, sizeof(end_label), ".Lend_%s", f->name);

    /* 본문 실행 */
    if (f->body) {
        Stmt *s = f->body->head;
        while (s) {
            gen_stmt(s, vars, var_count, end_label);
            s = s->next;
        }
    }

    /* 기본 반환값 */
    if (is_main) {
        emit("    movq $0, %%rax\n");
    }

    /* 에필로그 */
    emit("%s:\n", end_label);
    emit("    leave\n");
    emit("    ret\n");
}

/* === 프로그램 전체 코드 생성 === */
void gen_x86_program(FunctionList *prog) {
    if (!prog) {
        fprintf(stderr, "No program to generate.\n");
        return;
    }

    out_file = stdout;
    out_buffer = NULL;
    label_counter = 0;
    string_counter = 0;

    /* 데이터 섹션 */
    emit("    .section .rodata\n");
    emit("fmt_int:\n");
    emit("    .string \"%%ld\\n\"\n");

    /* 코드 섹션 */
    emit("    .text\n");

    /* 모든 함수 생성 */
    Function *f = prog->head;
    while (f) {
        gen_function(f);
        f = f->next;
    }
}

/* 버퍼로 출력 (Wasm용) */
int gen_x86_to_buffer(FunctionList *prog, char *buffer, int bufsize) {
    if (!prog || !buffer || bufsize <= 0) {
        return 0;
    }

    out_file = NULL;
    out_buffer = buffer;
    out_bufsize = bufsize;
    out_pos = 0;
    label_counter = 0;
    string_counter = 0;

    /* 데이터 섹션 */
    emit("    .section .rodata\n");
    emit("fmt_int:\n");
    emit("    .string \"%%ld\\n\"\n");

    /* 코드 섹션 */
    emit("    .text\n");

    /* 모든 함수 생성 */
    Function *f = prog->head;
    while (f) {
        gen_function(f);
        f = f->next;
    }

    /* 널 종료 보장 */
    if (out_pos < bufsize) {
        buffer[out_pos] = '\0';
    } else {
        buffer[bufsize - 1] = '\0';
    }

    int result = out_pos;
    out_buffer = NULL;
    out_bufsize = 0;
    out_pos = 0;

    return result;
}
