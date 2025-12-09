%{
/* Mini-JavaScript Parser
 * JavaScript 스타일 문법 지원
 */
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

int yylex(void);
void yyerror(const char *s);
%}

%union {
    int int_value;
    char *ident;
    Expr *expr;
    ExprList *expr_list;
    Stmt *stmt;
    StmtList *stmt_list;
    ParamList *param_list;
    Function *function;
    FunctionList *function_list;
}

/* 토큰 정의 */
%token FUNCTION LET VAR CONST RETURN
%token IF ELSE WHILE FOR
%token CONSOLE LOG
%token EQ NE LE GE AND OR
%token <int_value> NUMBER
%token <ident> IDENT
%token <ident> STRING

/* 타입 정의 */
%type <expr> expr primary call_expr opt_expr
%type <expr_list> arg_list arg_list_opt
%type <stmt> stmt vardecl assign_stmt single_stmt opt_for_init opt_for_step
%type <stmt_list> stmt_list stmt_list_opt compound_stmt
%type <param_list> param_list param_list_opt
%type <function> function

/* 연산자 우선순위 (낮은 것부터) */
%left OR
%left AND
%left EQ NE
%left '<' '>' LE GE
%left '+' '-'
%left '*' '/' '%'
%right UMINUS UNOT

%%

/* 프로그램: top-level 항목들 */
program
    : program_items            { /* g_program은 이미 조작됨 */ }
    ;

program_items
    : program_items program_item  { /* 누적 */ }
    | program_item                { /* 첫 항목 */ }
    | /* empty */                 { /* 빈 프로그램 허용 */ }
    ;

program_item
    : function  { program_add_function(g_program, $1); }
    | stmt      { program_add_stmt(g_program, $1); }
    ;

/* 함수 정의: function name(params) { body } */
function
    : FUNCTION IDENT '(' param_list_opt ')' compound_stmt
        { $$ = new_function($2, $4, $6); free($2); }
    ;

param_list_opt
    : /* empty */              { $$ = NULL; }
    | param_list               { $$ = $1; }
    ;

/* 매개변수 리스트: a, b, c */
param_list
    : IDENT                    { $$ = param_list_append(NULL, $1); free($1); }
    | param_list ',' IDENT     { $$ = param_list_append($1, $3); free($3); }
    ;

/* 복합문: { stmt_list } */
compound_stmt
    : '{' stmt_list_opt '}'    { $$ = $2; }
    ;

stmt_list_opt
    : /* empty */              { $$ = NULL; }
    | stmt_list                { $$ = $1; }
    ;

stmt_list
    : stmt_list stmt           { $$ = stmt_list_append($1, $2); }
    | stmt                     { $$ = stmt_list_append(NULL, $1); }
    ;

/* 문장 */
stmt
    : vardecl ';'              { $$ = $1; }
    | assign_stmt ';'          { $$ = $1; }
    | RETURN expr ';'          { $$ = new_return_stmt($2); }
    | RETURN ';'               { $$ = new_return_stmt(NULL); }
    | CONSOLE '.' LOG '(' expr ')' ';'
        { $$ = new_print_stmt($5); }
    | IF '(' expr ')' single_stmt ELSE single_stmt
        { $$ = new_if_stmt($3, $5, $7); }
    | IF '(' expr ')' single_stmt
        { $$ = new_if_stmt($3, $5, NULL); }
    | WHILE '(' expr ')' single_stmt
        { $$ = new_while_stmt($3, $5); }
    | FOR '(' opt_for_init ';' opt_expr ';' opt_for_step ')' single_stmt
        { $$ = new_for_stmt($3, $5, $7, $9); }
    | compound_stmt            { $$ = new_block_stmt($1); }
    | expr ';'                 { $$ = new_expr_stmt($1); }
    ;

/* for문 초기화 */
opt_for_init
    : /* empty */              { $$ = NULL; }
    | LET IDENT '=' expr       { $$ = new_vardecl_stmt($2, $4); free($2); }
    | VAR IDENT '=' expr       { $$ = new_vardecl_stmt($2, $4); free($2); }
    | IDENT '=' expr           { $$ = new_assign_stmt($1, $3); free($1); }
    ;

/* for문 조건 */
opt_expr
    : /* empty */              { $$ = NULL; }
    | expr                     { $$ = $1; }
    ;

/* for문 스텝 */
opt_for_step
    : /* empty */              { $$ = NULL; }
    | IDENT '=' expr           { $$ = new_assign_stmt($1, $3); free($1); }
    ;

/* 단일 문장 (if/while/for 바디용) */
single_stmt
    : compound_stmt            { $$ = new_block_stmt($1); }
    | vardecl ';'              { $$ = $1; }
    | assign_stmt ';'          { $$ = $1; }
    | RETURN expr ';'          { $$ = new_return_stmt($2); }
    | CONSOLE '.' LOG '(' expr ')' ';'
        { $$ = new_print_stmt($5); }
    | expr ';'                 { $$ = new_expr_stmt($1); }
    ;

/* 변수 선언 */
vardecl
    : LET IDENT                { $$ = new_vardecl_stmt($2, NULL); free($2); }
    | LET IDENT '=' expr       { $$ = new_vardecl_stmt($2, $4); free($2); }
    | VAR IDENT                { $$ = new_vardecl_stmt($2, NULL); free($2); }
    | VAR IDENT '=' expr       { $$ = new_vardecl_stmt($2, $4); free($2); }
    | CONST IDENT '=' expr     { $$ = new_vardecl_stmt($2, $4); free($2); }
    ;

/* 대입문 */
assign_stmt
    : IDENT '=' expr           { $$ = new_assign_stmt($1, $3); free($1); }
    ;

/* 표현식 */
expr
    : expr '+' expr            { $$ = new_binop_expr(BIN_ADD, $1, $3); }
    | expr '-' expr            { $$ = new_binop_expr(BIN_SUB, $1, $3); }
    | expr '*' expr            { $$ = new_binop_expr(BIN_MUL, $1, $3); }
    | expr '/' expr            { $$ = new_binop_expr(BIN_DIV, $1, $3); }
    | expr '%' expr            { $$ = new_binop_expr(BIN_MOD, $1, $3); }
    | expr '<' expr            { $$ = new_binop_expr(BIN_LT, $1, $3); }
    | expr '>' expr            { $$ = new_binop_expr(BIN_GT, $1, $3); }
    | expr LE expr             { $$ = new_binop_expr(BIN_LE, $1, $3); }
    | expr GE expr             { $$ = new_binop_expr(BIN_GE, $1, $3); }
    | expr EQ expr             { $$ = new_binop_expr(BIN_EQ, $1, $3); }
    | expr NE expr             { $$ = new_binop_expr(BIN_NE, $1, $3); }
    | expr AND expr            { $$ = new_binop_expr(BIN_AND, $1, $3); }
    | expr OR expr             { $$ = new_binop_expr(BIN_OR, $1, $3); }
    | '-' expr %prec UMINUS    { $$ = new_unary_expr(UNARY_NEG, $2); }
    | '!' expr %prec UNOT      { $$ = new_unary_expr(UNARY_NOT, $2); }
    | primary                  { $$ = $1; }
    ;

/* 기본 표현식 */
primary
    : NUMBER                   { $$ = new_int_expr($1); }
    | STRING                   { $$ = new_string_expr($1); free($1); }
    | IDENT                    { $$ = new_var_expr($1); free($1); }
    | call_expr                { $$ = $1; }
    | '(' expr ')'             { $$ = $2; }
    ;

/* 함수 호출 */
call_expr
    : IDENT '(' arg_list_opt ')'
        { $$ = new_call_expr($1, $3); free($1); }
    ;

arg_list_opt
    : /* empty */              { $$ = NULL; }
    | arg_list                 { $$ = $1; }
    ;

arg_list
    : expr                     { $$ = expr_list_append(NULL, $1); }
    | arg_list ',' expr        { $$ = expr_list_append($1, $3); }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Parse error: %s\n", s);
}
