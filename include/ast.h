#ifndef AST_H
#define AST_H

/* Mini-JS AST 정의
 * 12wk (함수 호출) + 11wk (제어문 if/while/for) + console.log 통합
 */

typedef struct Expr Expr;
typedef struct ExprList ExprList;
typedef struct Stmt Stmt;
typedef struct StmtList StmtList;
typedef struct Param Param;
typedef struct ParamList ParamList;
typedef struct Function Function;
typedef struct FunctionList FunctionList;

/* 표현식 종류 */
typedef enum {
    EXPR_INT,       /* 정수 리터럴 */
    EXPR_STRING,    /* 문자열 리터럴 */
    EXPR_VAR,       /* 변수 참조 */
    EXPR_BINOP,     /* 이항 연산 */
    EXPR_CALL,      /* 함수 호출 */
    EXPR_UNARY      /* 단항 연산 (음수) */
} ExprKind;

/* 이항 연산자 종류 */
typedef enum {
    BIN_ADD,        /* + */
    BIN_SUB,        /* - */
    BIN_MUL,        /* * */
    BIN_DIV,        /* / */
    BIN_MOD,        /* % */
    BIN_LT,         /* < */
    BIN_GT,         /* > */
    BIN_LE,         /* <= */
    BIN_GE,         /* >= */
    BIN_EQ,         /* == */
    BIN_NE,         /* != */
    BIN_AND,        /* && */
    BIN_OR          /* || */
} BinOpKind;

/* 단항 연산자 종류 */
typedef enum {
    UNARY_NEG,      /* - (음수) */
    UNARY_NOT       /* ! (논리 부정) */
} UnaryOpKind;

/* 표현식 노드 */
struct Expr {
    ExprKind kind;
    union {
        int int_value;                  /* EXPR_INT */
        char *string_value;             /* EXPR_STRING */
        char *var_name;                 /* EXPR_VAR */
        struct {                        /* EXPR_BINOP */
            BinOpKind op;
            Expr *lhs;
            Expr *rhs;
        } binop;
        struct {                        /* EXPR_CALL */
            char *func_name;
            ExprList *args;
        } call;
        struct {                        /* EXPR_UNARY */
            UnaryOpKind op;
            Expr *operand;
        } unary;
    } u;
};

/* 표현식 리스트 (함수 인자용) */
struct ExprList {
    Expr *expr;
    ExprList *next;
};

/* 문장 종류 */
typedef enum {
    STMT_EXPR,      /* 표현식 문장 */
    STMT_RETURN,    /* return 문 */
    STMT_VARDECL,   /* 변수 선언 (let/var/const) */
    STMT_ASSIGN,    /* 대입문 */
    STMT_PRINT,     /* console.log() */
    STMT_IF,        /* if 문 */
    STMT_WHILE,     /* while 문 */
    STMT_FOR,       /* for 문 */
    STMT_BLOCK      /* 블록 { } */
} StmtKind;

/* 문장 노드 */
struct Stmt {
    StmtKind kind;
    union {
        Expr *expr;                     /* STMT_EXPR, STMT_RETURN, STMT_PRINT */
        struct {                        /* STMT_VARDECL */
            char *var_name;
            Expr *init_value;           /* 초기값 (NULL 가능) */
        } vardecl;
        struct {                        /* STMT_ASSIGN */
            char *var_name;
            Expr *value;
        } assign;
        struct {                        /* STMT_IF */
            Expr *cond;
            Stmt *then_stmt;
            Stmt *else_stmt;            /* NULL 가능 */
        } if_stmt;
        struct {                        /* STMT_WHILE */
            Expr *cond;
            Stmt *body;
        } while_stmt;
        struct {                        /* STMT_FOR */
            Stmt *init;                 /* 초기화 (NULL 가능) */
            Expr *cond;                 /* 조건 (NULL 가능) */
            Stmt *step;                 /* 반복 (NULL 가능) */
            Stmt *body;
        } for_stmt;
        StmtList *block;                /* STMT_BLOCK */
    } u;
    Stmt *next;                         /* 연결 리스트용 */
};

/* 문장 리스트 */
struct StmtList {
    Stmt *head;
    Stmt *tail;
};

/* 매개변수 */
struct Param {
    char *name;
    Param *next;
};

/* 매개변수 리스트 */
struct ParamList {
    Param *head;
    Param *tail;
};

/* 함수 정의 */
struct Function {
    char *name;
    ParamList *params;
    StmtList *body;
    Function *next;
};

/* 함수 리스트 (프로그램) */
struct FunctionList {
    Function *head;
    Function *tail;
};

/* === 표현식 생성 함수 === */
Expr *new_int_expr(int value);
Expr *new_string_expr(const char *value);
Expr *new_var_expr(const char *name);
Expr *new_binop_expr(BinOpKind op, Expr *lhs, Expr *rhs);
Expr *new_call_expr(const char *func_name, ExprList *args);
Expr *new_unary_expr(UnaryOpKind op, Expr *operand);

/* 표현식 리스트 */
ExprList *expr_list_append(ExprList *list, Expr *expr);

/* === 문장 생성 함수 === */
Stmt *new_expr_stmt(Expr *e);
Stmt *new_return_stmt(Expr *e);
Stmt *new_vardecl_stmt(const char *name, Expr *init);
Stmt *new_assign_stmt(const char *name, Expr *value);
Stmt *new_print_stmt(Expr *e);
Stmt *new_if_stmt(Expr *cond, Stmt *then_stmt, Stmt *else_stmt);
Stmt *new_while_stmt(Expr *cond, Stmt *body);
Stmt *new_for_stmt(Stmt *init, Expr *cond, Stmt *step, Stmt *body);
Stmt *new_block_stmt(StmtList *stmts);

/* 문장 리스트 */
StmtList *stmt_list_append(StmtList *list, Stmt *stmt);

/* === 함수 및 매개변수 === */
ParamList *param_list_append(ParamList *list, const char *name);
Function *new_function(const char *name, ParamList *params, StmtList *body);
FunctionList *function_list_append(FunctionList *list, Function *func);

/* 전역 프로그램 루트 */
extern FunctionList *g_program;

/* 메모리 해제 */
void free_expr(Expr *e);
void free_stmt(Stmt *s);
void free_function(Function *f);
void free_program(FunctionList *prog);

#endif /* AST_H */
