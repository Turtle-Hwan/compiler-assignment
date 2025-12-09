#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

/* 전역 프로그램 루트 */
Program *g_program = NULL;

/* 문자열 복제 헬퍼 */
static char *strdup_safe(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char *p = (char *)malloc(len);
    if (!p) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }
    memcpy(p, s, len);
    return p;
}

/* === 표현식 생성 함수 === */

Expr *new_int_expr(int value) {
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    e->kind = EXPR_INT;
    e->u.int_value = value;
    return e;
}

Expr *new_string_expr(const char *value) {
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    e->kind = EXPR_STRING;
    e->u.string_value = strdup_safe(value);
    return e;
}

Expr *new_var_expr(const char *name) {
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    e->kind = EXPR_VAR;
    e->u.var_name = strdup_safe(name);
    return e;
}

Expr *new_binop_expr(BinOpKind op, Expr *lhs, Expr *rhs) {
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    e->kind = EXPR_BINOP;
    e->u.binop.op = op;
    e->u.binop.lhs = lhs;
    e->u.binop.rhs = rhs;
    return e;
}

Expr *new_call_expr(const char *func_name, ExprList *args) {
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    e->kind = EXPR_CALL;
    e->u.call.func_name = strdup_safe(func_name);
    e->u.call.args = args;
    return e;
}

Expr *new_unary_expr(UnaryOpKind op, Expr *operand) {
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    e->kind = EXPR_UNARY;
    e->u.unary.op = op;
    e->u.unary.operand = operand;
    return e;
}

/* 표현식 리스트 추가 */
ExprList *expr_list_append(ExprList *list, Expr *expr) {
    ExprList *node = (ExprList *)calloc(1, sizeof(ExprList));
    node->expr = expr;
    node->next = NULL;

    if (!list) return node;

    ExprList *cur = list;
    while (cur->next) cur = cur->next;
    cur->next = node;
    return list;
}

/* === 문장 생성 함수 === */

Stmt *new_expr_stmt(Expr *e) {
    Stmt *s = (Stmt *)calloc(1, sizeof(Stmt));
    s->kind = STMT_EXPR;
    s->u.expr = e;
    return s;
}

Stmt *new_return_stmt(Expr *e) {
    Stmt *s = (Stmt *)calloc(1, sizeof(Stmt));
    s->kind = STMT_RETURN;
    s->u.expr = e;
    return s;
}

Stmt *new_vardecl_stmt(const char *name, Expr *init) {
    Stmt *s = (Stmt *)calloc(1, sizeof(Stmt));
    s->kind = STMT_VARDECL;
    s->u.vardecl.var_name = strdup_safe(name);
    s->u.vardecl.init_value = init;
    return s;
}

Stmt *new_assign_stmt(const char *name, Expr *value) {
    Stmt *s = (Stmt *)calloc(1, sizeof(Stmt));
    s->kind = STMT_ASSIGN;
    s->u.assign.var_name = strdup_safe(name);
    s->u.assign.value = value;
    return s;
}

Stmt *new_print_stmt(Expr *e) {
    Stmt *s = (Stmt *)calloc(1, sizeof(Stmt));
    s->kind = STMT_PRINT;
    s->u.expr = e;
    return s;
}

Stmt *new_if_stmt(Expr *cond, Stmt *then_stmt, Stmt *else_stmt) {
    Stmt *s = (Stmt *)calloc(1, sizeof(Stmt));
    s->kind = STMT_IF;
    s->u.if_stmt.cond = cond;
    s->u.if_stmt.then_stmt = then_stmt;
    s->u.if_stmt.else_stmt = else_stmt;
    return s;
}

Stmt *new_while_stmt(Expr *cond, Stmt *body) {
    Stmt *s = (Stmt *)calloc(1, sizeof(Stmt));
    s->kind = STMT_WHILE;
    s->u.while_stmt.cond = cond;
    s->u.while_stmt.body = body;
    return s;
}

Stmt *new_for_stmt(Stmt *init, Expr *cond, Stmt *step, Stmt *body) {
    Stmt *s = (Stmt *)calloc(1, sizeof(Stmt));
    s->kind = STMT_FOR;
    s->u.for_stmt.init = init;
    s->u.for_stmt.cond = cond;
    s->u.for_stmt.step = step;
    s->u.for_stmt.body = body;
    return s;
}

Stmt *new_block_stmt(StmtList *stmts) {
    Stmt *s = (Stmt *)calloc(1, sizeof(Stmt));
    s->kind = STMT_BLOCK;
    s->u.block = stmts;
    return s;
}

/* 문장 리스트 추가 */
StmtList *stmt_list_append(StmtList *list, Stmt *stmt) {
    if (!stmt) return list;
    if (!list) {
        list = (StmtList *)calloc(1, sizeof(StmtList));
        list->head = list->tail = NULL;
    }
    if (!list->head) {
        list->head = list->tail = stmt;
    } else {
        list->tail->next = stmt;
        list->tail = stmt;
    }
    return list;
}

/* === 함수 및 매개변수 === */

ParamList *param_list_append(ParamList *list, const char *name) {
    Param *p = (Param *)calloc(1, sizeof(Param));
    p->name = strdup_safe(name);
    p->next = NULL;

    if (!list) {
        list = (ParamList *)calloc(1, sizeof(ParamList));
        list->head = list->tail = NULL;
    }
    if (!list->head) {
        list->head = list->tail = p;
    } else {
        list->tail->next = p;
        list->tail = p;
    }
    return list;
}

Function *new_function(const char *name, ParamList *params, StmtList *body) {
    Function *f = (Function *)calloc(1, sizeof(Function));
    f->name = strdup_safe(name);
    f->params = params;
    f->body = body;
    f->next = NULL;
    return f;
}

FunctionList *function_list_append(FunctionList *list, Function *func) {
    if (!func) return list;
    if (!list) {
        list = (FunctionList *)calloc(1, sizeof(FunctionList));
        list->head = list->tail = NULL;
    }
    if (!list->head) {
        list->head = list->tail = func;
    } else {
        list->tail->next = func;
        list->tail = func;
    }
    return list;
}

/* === 프로그램 생성/조작 === */

Program *new_program(void) {
    Program *p = (Program *)calloc(1, sizeof(Program));
    p->items = NULL;
    p->items_tail = NULL;
    return p;
}

static Item *new_item(ItemKind kind) {
    Item *item = (Item *)calloc(1, sizeof(Item));
    item->kind = kind;
    item->next = NULL;
    return item;
}

void program_add_function(Program *prog, Function *func) {
    if (!prog || !func) return;
    Item *item = new_item(ITEM_FUNCTION);
    item->u.function = func;
    if (prog->items_tail) {
        prog->items_tail->next = item;
    } else {
        prog->items = item;
    }
    prog->items_tail = item;
}

void program_add_stmt(Program *prog, Stmt *stmt) {
    if (!prog || !stmt) return;
    Item *item = new_item(ITEM_STMT);
    item->u.stmt = stmt;
    if (prog->items_tail) {
        prog->items_tail->next = item;
    } else {
        prog->items = item;
    }
    prog->items_tail = item;
}

/* === 메모리 해제 함수 === */

void free_expr(Expr *e) {
    if (!e) return;
    switch (e->kind) {
        case EXPR_STRING:
            free(e->u.string_value);
            break;
        case EXPR_VAR:
            free(e->u.var_name);
            break;
        case EXPR_BINOP:
            free_expr(e->u.binop.lhs);
            free_expr(e->u.binop.rhs);
            break;
        case EXPR_CALL: {
            free(e->u.call.func_name);
            ExprList *arg = e->u.call.args;
            while (arg) {
                ExprList *next = arg->next;
                free_expr(arg->expr);
                free(arg);
                arg = next;
            }
            break;
        }
        case EXPR_UNARY:
            free_expr(e->u.unary.operand);
            break;
        default:
            break;
    }
    free(e);
}

void free_stmt(Stmt *s) {
    if (!s) return;
    switch (s->kind) {
        case STMT_EXPR:
        case STMT_RETURN:
        case STMT_PRINT:
            free_expr(s->u.expr);
            break;
        case STMT_VARDECL:
            free(s->u.vardecl.var_name);
            free_expr(s->u.vardecl.init_value);
            break;
        case STMT_ASSIGN:
            free(s->u.assign.var_name);
            free_expr(s->u.assign.value);
            break;
        case STMT_IF:
            free_expr(s->u.if_stmt.cond);
            free_stmt(s->u.if_stmt.then_stmt);
            free_stmt(s->u.if_stmt.else_stmt);
            break;
        case STMT_WHILE:
            free_expr(s->u.while_stmt.cond);
            free_stmt(s->u.while_stmt.body);
            break;
        case STMT_FOR:
            free_stmt(s->u.for_stmt.init);
            free_expr(s->u.for_stmt.cond);
            free_stmt(s->u.for_stmt.step);
            free_stmt(s->u.for_stmt.body);
            break;
        case STMT_BLOCK: {
            StmtList *block = s->u.block;
            if (block) {
                Stmt *curr = block->head;
                while (curr) {
                    Stmt *next = curr->next;
                    curr->next = NULL;
                    free_stmt(curr);
                    curr = next;
                }
                free(block);
            }
            break;
        }
    }
    free(s);
}

void free_function(Function *f) {
    if (!f) return;
    free(f->name);
    if (f->params) {
        Param *p = f->params->head;
        while (p) {
            Param *next = p->next;
            free(p->name);
            free(p);
            p = next;
        }
        free(f->params);
    }
    if (f->body) {
        Stmt *s = f->body->head;
        while (s) {
            Stmt *next = s->next;
            s->next = NULL;
            free_stmt(s);
            s = next;
        }
        free(f->body);
    }
    free(f);
}

void free_program(Program *prog) {
    if (!prog) return;
    Item *item = prog->items;
    while (item) {
        Item *next = item->next;
        if (item->kind == ITEM_FUNCTION) {
            free_function(item->u.function);
        } else if (item->kind == ITEM_STMT) {
            free_stmt(item->u.stmt);
        }
        free(item);
        item = next;
    }
    free(prog);
}

/* === AST 시각화 === */

#include <stdarg.h>

/* 버퍼 관리 (static 상태) */
static char *ast_buf = NULL;
static int ast_bufsize = 0;
static int ast_pos = 0;

/* 버퍼에 출력 */
static void ast_emit(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (ast_buf) {
        int remaining = ast_bufsize - ast_pos;
        if (remaining > 0) {
            int written = vsnprintf(ast_buf + ast_pos, remaining, fmt, args);
            if (written > 0) {
                ast_pos += (written < remaining) ? written : (remaining - 1);
            }
        }
    }
    va_end(args);
}

/* 들여쓰기 (depth * 2칸) */
static void ast_emit_indent(int depth) {
    for (int i = 0; i < depth; i++) {
        ast_emit("  ");
    }
}

/* 이항 연산자 -> 문자열 */
static const char* binop_to_string(BinOpKind op) {
    switch (op) {
        case BIN_ADD: return "+";
        case BIN_SUB: return "-";
        case BIN_MUL: return "*";
        case BIN_DIV: return "/";
        case BIN_MOD: return "%";
        case BIN_LT:  return "<";
        case BIN_GT:  return ">";
        case BIN_LE:  return "<=";
        case BIN_GE:  return ">=";
        case BIN_EQ:  return "==";
        case BIN_NE:  return "!=";
        case BIN_AND: return "&&";
        case BIN_OR:  return "||";
        default:      return "?";
    }
}

/* 단항 연산자 -> 문자열 */
static const char* unary_to_string(UnaryOpKind op) {
    switch (op) {
        case UNARY_NEG: return "-";
        case UNARY_NOT: return "!";
        default:        return "?";
    }
}

/* 전방 선언 */
static void print_expr(Expr *e, int depth);
static void print_stmt(Stmt *s, int depth);

/* 표현식 노드 출력 */
static void print_expr(Expr *e, int depth) {
    if (!e) return;

    ast_emit_indent(depth);

    switch (e->kind) {
        case EXPR_INT:
            ast_emit("INT: %d\n", e->u.int_value);
            break;

        case EXPR_STRING:
            ast_emit("STRING: \"%s\"\n", e->u.string_value ? e->u.string_value : "");
            break;

        case EXPR_VAR:
            ast_emit("VAR: %s\n", e->u.var_name);
            break;

        case EXPR_BINOP:
            ast_emit("BINOP: %s\n", binop_to_string(e->u.binop.op));
            print_expr(e->u.binop.lhs, depth + 1);
            print_expr(e->u.binop.rhs, depth + 1);
            break;

        case EXPR_CALL:
            ast_emit("CALL: %s\n", e->u.call.func_name);
            for (ExprList *arg = e->u.call.args; arg; arg = arg->next) {
                print_expr(arg->expr, depth + 1);
            }
            break;

        case EXPR_UNARY:
            ast_emit("UNARY: %s\n", unary_to_string(e->u.unary.op));
            print_expr(e->u.unary.operand, depth + 1);
            break;
    }
}

/* 문장 노드 출력 */
static void print_stmt(Stmt *s, int depth) {
    if (!s) return;

    ast_emit_indent(depth);

    switch (s->kind) {
        case STMT_EXPR:
            ast_emit("EXPR_STMT\n");
            print_expr(s->u.expr, depth + 1);
            break;

        case STMT_RETURN:
            ast_emit("RETURN\n");
            if (s->u.expr) {
                print_expr(s->u.expr, depth + 1);
            }
            break;

        case STMT_VARDECL:
            ast_emit("VARDECL: %s\n", s->u.vardecl.var_name);
            if (s->u.vardecl.init_value) {
                print_expr(s->u.vardecl.init_value, depth + 1);
            }
            break;

        case STMT_ASSIGN:
            ast_emit("ASSIGN: %s\n", s->u.assign.var_name);
            print_expr(s->u.assign.value, depth + 1);
            break;

        case STMT_PRINT:
            ast_emit("PRINT\n");
            print_expr(s->u.expr, depth + 1);
            break;

        case STMT_IF:
            ast_emit("IF\n");
            ast_emit_indent(depth + 1);
            ast_emit("COND:\n");
            print_expr(s->u.if_stmt.cond, depth + 2);
            ast_emit_indent(depth + 1);
            ast_emit("THEN:\n");
            print_stmt(s->u.if_stmt.then_stmt, depth + 2);
            if (s->u.if_stmt.else_stmt) {
                ast_emit_indent(depth + 1);
                ast_emit("ELSE:\n");
                print_stmt(s->u.if_stmt.else_stmt, depth + 2);
            }
            break;

        case STMT_WHILE:
            ast_emit("WHILE\n");
            ast_emit_indent(depth + 1);
            ast_emit("COND:\n");
            print_expr(s->u.while_stmt.cond, depth + 2);
            ast_emit_indent(depth + 1);
            ast_emit("BODY:\n");
            print_stmt(s->u.while_stmt.body, depth + 2);
            break;

        case STMT_FOR:
            ast_emit("FOR\n");
            if (s->u.for_stmt.init) {
                ast_emit_indent(depth + 1);
                ast_emit("INIT:\n");
                print_stmt(s->u.for_stmt.init, depth + 2);
            }
            if (s->u.for_stmt.cond) {
                ast_emit_indent(depth + 1);
                ast_emit("COND:\n");
                print_expr(s->u.for_stmt.cond, depth + 2);
            }
            if (s->u.for_stmt.step) {
                ast_emit_indent(depth + 1);
                ast_emit("STEP:\n");
                print_stmt(s->u.for_stmt.step, depth + 2);
            }
            ast_emit_indent(depth + 1);
            ast_emit("BODY:\n");
            print_stmt(s->u.for_stmt.body, depth + 2);
            break;

        case STMT_BLOCK:
            ast_emit("BLOCK\n");
            if (s->u.block) {
                for (Stmt *curr = s->u.block->head; curr; curr = curr->next) {
                    print_stmt(curr, depth + 1);
                }
            }
            break;
    }
}

/* 함수 노드 출력 */
static void print_function(Function *f, int depth) {
    if (!f) return;

    ast_emit_indent(depth);
    ast_emit("Function: %s(", f->name);

    /* 매개변수 */
    if (f->params) {
        Param *p = f->params->head;
        int first = 1;
        while (p) {
            if (!first) ast_emit(", ");
            ast_emit("%s", p->name);
            first = 0;
            p = p->next;
        }
    }
    ast_emit(")\n");

    /* 함수 본문 */
    if (f->body) {
        for (Stmt *s = f->body->head; s; s = s->next) {
            print_stmt(s, depth + 1);
        }
    }
}

/* 공개 API: AST를 버퍼에 출력 */
int ast_to_buffer(Program *prog, char *buffer, int bufsize) {
    if (!buffer || bufsize <= 0) {
        return 0;
    }

    /* 버퍼 상태 초기화 */
    ast_buf = buffer;
    ast_bufsize = bufsize;
    ast_pos = 0;
    buffer[0] = '\0';

    if (!prog || !prog->items) {
        ast_emit("(No program)\n");
    } else {
        ast_emit("Program\n");
        for (Item *item = prog->items; item; item = item->next) {
            if (item->kind == ITEM_FUNCTION) {
                print_function(item->u.function, 1);
            } else if (item->kind == ITEM_STMT) {
                ast_emit_indent(1);
                ast_emit("TopLevel Statement:\n");
                print_stmt(item->u.stmt, 2);
            }
        }
    }

    /* null 종료 보장 */
    if (ast_pos < bufsize) {
        buffer[ast_pos] = '\0';
    } else {
        buffer[bufsize - 1] = '\0';
    }

    int result = ast_pos;

    /* 상태 초기화 */
    ast_buf = NULL;
    ast_bufsize = 0;
    ast_pos = 0;

    return result;
}
