%option noyywrap
%option nounput
%option noinput

%{
/* Mini-JavaScript Lexer
 * JavaScript 스타일 키워드와 토큰 정의
 */
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"

/* 문자열 버퍼에서 파싱하기 위한 변수 */
static const char *input_string = NULL;
static int input_pos = 0;

#undef YY_INPUT
#define YY_INPUT(buf, result, max_size) \
    do { \
        if (input_string) { \
            int c = input_string[input_pos]; \
            if (c == '\0') { \
                result = YY_NULL; \
            } else { \
                buf[0] = c; \
                input_pos++; \
                result = 1; \
            } \
        } else { \
            int c = fgetc(yyin); \
            result = (c == EOF) ? YY_NULL : (buf[0] = c, 1); \
        } \
    } while (0)

void yy_scan_string_custom(const char *str) {
    input_string = str;
    input_pos = 0;
    YY_FLUSH_BUFFER;
}

void yy_reset_input(void) {
    input_string = NULL;
    input_pos = 0;
}
%}

%%

[ \t\r]+        { /* 공백 무시 */ }
\n              { /* 개행 무시 */ }

 /* JavaScript 키워드 */
"function"      { return FUNCTION; }
"let"           { return LET; }
"var"           { return VAR; }
"const"         { return CONST; }
"return"        { return RETURN; }
"if"            { return IF; }
"else"          { return ELSE; }
"while"         { return WHILE; }
"for"           { return FOR; }
"console"       { return CONSOLE; }
"log"           { return LOG; }
"true"          { yylval.int_value = 1; return NUMBER; }
"false"         { yylval.int_value = 0; return NUMBER; }

 /* 숫자 리터럴 */
[0-9]+          { yylval.int_value = atoi(yytext); return NUMBER; }

 /* 문자열 리터럴 (double quote, single quote, backtick) */
\"([^\"\\]|\\.)*\"  {
                    /* 따옴표 제거 후 저장 */
                    int len = strlen(yytext) - 2;
                    yylval.ident = (char *)malloc(len + 1);
                    strncpy(yylval.ident, yytext + 1, len);
                    yylval.ident[len] = '\0';
                    return STRING;
                }
\'([^\'\\]|\\.)*\'  {
                    int len = strlen(yytext) - 2;
                    yylval.ident = (char *)malloc(len + 1);
                    strncpy(yylval.ident, yytext + 1, len);
                    yylval.ident[len] = '\0';
                    return STRING;
                }
\`([^\`\\]|\\.)*\`  {
                    int len = strlen(yytext) - 2;
                    yylval.ident = (char *)malloc(len + 1);
                    strncpy(yylval.ident, yytext + 1, len);
                    yylval.ident[len] = '\0';
                    return STRING;
                }

 /* 식별자 */
[a-zA-Z_][a-zA-Z0-9_]* {
                    yylval.ident = strdup(yytext);
                    return IDENT;
                }

 /* 비교 및 논리 연산자 */
"=="            { return EQ; }
"!="            { return NE; }
"<="            { return LE; }
">="            { return GE; }
"&&"            { return AND; }
"||"            { return OR; }

 /* 단일 문자 토큰 */
"{"             { return '{'; }
"}"             { return '}'; }
"("             { return '('; }
")"             { return ')'; }
"["             { return '['; }
"]"             { return ']'; }
";"             { return ';'; }
","             { return ','; }
"."             { return '.'; }
"+"             { return '+'; }
"-"             { return '-'; }
"*"             { return '*'; }
"/"             { return '/'; }
"%"             { return '%'; }
"<"             { return '<'; }
">"             { return '>'; }
"="             { return '='; }
"!"             { return '!'; }

 /* 주석 처리 */
"//".*          { /* 한 줄 주석 무시 */ }
"/*"([^*]|\*+[^*/])*\*+"/"  { /* 여러 줄 주석 무시 */ }

 /* 기타 문자 */
.               { return yytext[0]; }

%%
