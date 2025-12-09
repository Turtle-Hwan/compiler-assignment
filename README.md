# MINI-JS Compiler

## 1. 개요 및 설계 의도

### 1.1 개요

평소 자주 쓰던 JavaScript와 비슷한 인터프리터를 직접 구현해보고 싶어 시작했습니다.

### 1.2 설계 의도

JavaScript의 특징에 주안점을 두어 구현하였습니다

- `console.log`로 문자열 출력
- Top-level statement로 바로 함수나 변수 선언 및 활용, `console.log` 등의 문을 실행할 수 있는 것

### 1.3 Web Assembly (WASM) 적용

컴파일 과정(AST, Assembly, 실행 결과 등)을 웹 상에서 편하게 보고 싶어서 방법을 찾던 도중, WebAssembly(wasm)으로도 컴파일한 후 브라우저에서 렌더링하여 웹에서 볼 수 있다는 것을 확인했고, 이 방법으로 웹에서 컴파일러가 작동하도록 구현해보았습니다.

### 1.4 의존성 (필요 도구)

- GCC (또는 MinGW)
- Flex (lexer 생성)
- Bison (parser 생성)
- Emscripten (WebAssembly 빌드용, 선택사항)

### 1.5 빌드 방법

```bash
make            # 데스크톱 버전 빌드
make wasm       # WebAssembly 버전 빌드
make test       # 테스트 실행
make clean      # 정리
```

### 1.6 실행 방법

```bash
# 인터프리터 모드 (실행)
./minijs -e input.js

# 컴파일 모드 (어셈블리 생성)
./minijs -c input.js -o output.s
```

### 1.7 웹 버전 실행

1. `make wasm`으로 빌드
2. `docs/index.html`을 웹 브라우저로 열기
3. 코드 입력 후 "Compile & Run" 클릭

### 1.8 프로젝트 구조

```
mini_js/
├── include/
│   ├── ast.h           # AST 정의
│   ├── eval.h          # Interpreter 인터페이스
│   ├── codegen_x86.h   # 코드 생성기 인터페이스
│   └── symtab.h        # 심볼 테이블
├── src/
│   ├── ast.c           # AST 구현
│   ├── eval.c          # Interpreter 구현
│   ├── codegen_x86.c   # x86-64 코드 생성
│   ├── symtab.c        # 심볼 테이블 (스코프 지원)
│   ├── main.c          # 메인 프로그램 (CLI)
│   └── web_driver.c    # 웹 인터페이스 (Wasm)
├── parser/
│   ├── scanner.l       # Flex Lexer
│   └── parser.y        # Bison Parser
├── examples/           # 테스트 파일 15개
│   ├── *.js
│   ├── expected/       # 예상 출력
│   └── TESTS.md        # 테스트 문서
├── docs/
│   └── index.html      # 웹 프론트엔드
├── Makefile
└── README.md
```

---

## 2. 문법 정의 (Grammar)

### EBNF 표기

```ebnf
program       ::= program_item*
program_item  ::= function | stmt

function      ::= 'function' IDENT '(' param_list? ')' compound_stmt
param_list    ::= IDENT (',' IDENT)*

compound_stmt ::= '{' stmt* '}'

stmt          ::= vardecl ';'
                | assign_stmt ';'
                | 'return' expr? ';'
                | 'console' '.' 'log' '(' expr ')' ';'
                | 'if' '(' expr ')' stmt ('else' stmt)?
                | 'while' '(' expr ')' stmt
                | 'for' '(' for_init? ';' expr? ';' for_step? ')' stmt
                | compound_stmt
                | expr ';'

vardecl       ::= ('let' | 'var' | 'const') IDENT ('=' expr)?
assign_stmt   ::= IDENT '=' expr

for_init      ::= vardecl | assign_stmt
for_step      ::= assign_stmt

expr          ::= expr binop expr
                | unop expr
                | primary

binop         ::= '+' | '-' | '*' | '/' | '%'
                | '<' | '>' | '<=' | '>=' | '==' | '!='
                | '&&' | '||'

unop          ::= '-' | '!'

primary       ::= NUMBER | STRING | IDENT | call_expr | '(' expr ')'
call_expr     ::= IDENT '(' arg_list? ')'
arg_list      ::= expr (',' expr)*
```

### 연산자 우선순위 (낮음 → 높음)

| 우선순위 | 연산자            | 결합성 |
| -------- | ----------------- | ------ |
| 1        | `\|\|`            | 좌측   |
| 2        | `&&`              | 좌측   |
| 3        | `==` `!=`         | 좌측   |
| 4        | `<` `>` `<=` `>=` | 좌측   |
| 5        | `+` `-`           | 좌측   |
| 6        | `*` `/` `%`       | 좌측   |
| 7        | `-` (단항) `!`    | 우측   |

---

## 3. 전체 구조 (컴파일 흐름도)

```
┌─────────────────────────────────────────────────────────────┐
│                    Source Code (.js)                        │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                 Lexer (scanner.l - Flex)                    │
│  • 토큰 분리: 키워드, 식별자, 리터럴, 연산자                │
│  • 문자열 리터럴: "", '', `` 세 가지 지원                   │
│  • 주석 처리: //, /* */                                     │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
                          Tokens
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                 Parser (parser.y - Bison)                   │
│  • 문법 검증 및 AST 생성                                    │
│  • Top-level: 함수 정의 + 문장 혼합 가능                    │
│  • 연산자 우선순위 처리                                     │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
                      AST (Abstract Syntax Tree)
                              │
              ┌───────────────┴───────────────┐
              ▼                               ▼
┌──────────────────────────┐    ┌──────────────────────────┐
│   Interpreter (eval.c)   │    │  Code Generator          │
│                          │    │  (codegen_x86.c)         │
│  • AST 직접 실행         │    │                          │
│  • 심볼 테이블 관리      │    │  • x86-64 어셈블리 생성  │
│  • 스코프 지원           │    │  • Linux System V ABI    │
└──────────────────────────┘    └──────────────────────────┘
              │                               │
              ▼                               ▼
         실행 결과                      Assembly (.s)
```

### 3.1 WebAssembly (WASM) 컴파일 흐름도

Emscripten을 사용하여 브라우저에서 실행 가능한 WASM으로 컴파일하는 경우의 흐름입니다.

```
┌─────────────────────────────────────────────────────────────┐
│                    Source Code (.js)                        │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                 Lexer (scanner.l - Flex)                    │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
                          Tokens
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                 Parser (parser.y - Bison)                   │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
                      AST (Abstract Syntax Tree)
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                   Interpreter (eval.c)                      │
│              • C 소스 코드로 AST 해석 실행                  │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│              Emscripten (emcc 컴파일러)                     │
│  • C 소스 → LLVM IR → WebAssembly 변환                     │
│  • Flex/Bison 생성 코드 포함 전체 컴파일                   │
│  • JavaScript 글루 코드 자동 생성                          │
└─────────────────────────────────────────────────────────────┘
                              │
              ┌───────────────┴───────────────┐
              ▼                               ▼
┌──────────────────────────┐    ┌──────────────────────────┐
│    mini_js.wasm          │    │    mini_js.js            │
│  • WebAssembly 바이너리  │    │  • JavaScript 글루 코드  │
│  • 브라우저에서 실행     │    │  • WASM 로딩/초기화      │
└──────────────────────────┘    └──────────────────────────┘
              │                               │
              └───────────────┬───────────────┘
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    웹 브라우저 실행                         │
│  • index.html에서 WASM 모듈 로드                           │
│  • 사용자 입력 → Mini-JS 파싱/실행 → 결과 출력             │
└─────────────────────────────────────────────────────────────┘
```

---

## 4. 문자열 리터럴 처리 (`""`, `''`, `` ` ` ``)

JavaScript와 동일하게 세 가지 문자열 리터럴 형식을 지원합니다.

### Lexer 구현 (scanner.l)

```c
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
```

### 사용 예시

```javascript
function main() {
  console.log("Hello, World!"); // Double quotes
  console.log("Single quotes work"); // Single quotes
  console.log(`Backticks also work`); // Backticks
  return 0;
}
```

### console.log 출력 처리 (eval.c)

```c
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
```

---

## 5. Top-level Code Execution (함수 호이스팅)

실제 JavaScript처럼 함수 밖에서도 코드를 실행할 수 있으며, 함수 선언 이전에 호출해도 동작합니다.

### 기존 Mini-C와의 차이점

| 특성           | Mini-C             | Mini-JS                       |
| -------------- | ------------------ | ----------------------------- |
| 프로그램 시작  | `main()` 함수 필수 | Top-level 문장 순차 실행      |
| 함수 위치      | 선언 후 호출       | 어디서든 선언 가능 (호이스팅) |
| Top-level 코드 | 불가능             | 가능                          |

### 구현 방식

#### 1) AST 구조 변경 (ast.h)

```c
/* Top-level 항목 종류 */
typedef enum {
    ITEM_FUNCTION,  /* 함수 정의 */
    ITEM_STMT       /* 문장 */
} ItemKind;

/* Top-level 항목 */
struct Item {
    ItemKind kind;
    union {
        Function *function;  /* ITEM_FUNCTION */
        Stmt *stmt;          /* ITEM_STMT */
    } u;
    Item *next;
};

/* 프로그램 구조체 */
struct Program {
    Item *items;        /* Top-level 항목들 */
    Item *items_tail;   /* append용 */
};
```

#### 2) Parser 문법 변경 (parser.y)

```yacc
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
```

#### 3) Interpreter 실행 방식 변경 (eval.c)

```c
/* === 프로그램 실행 (순차 처리) === */
int eval_program(Program *prog) {
    /* Top-level 항목 순차 처리 */
    Item *item = prog->items;

    while (item) {
        if (item->kind == ITEM_FUNCTION) {
            /* 함수 정의 → 테이블에 등록 (호이스팅) */
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

    return (int)result;
}
```

### 사용 예시

```javascript
// 함수 선언 전에 호출 가능 (호이스팅)
console.log(add(3, 5)); // 8 출력

function add(a, b) {
  return a + b;
}

// Top-level 변수 선언 및 사용
let x = 10;
console.log(x + 20); // 30 출력
```

---

## 6. 구현된 기능

### 변수

- `let`, `var`, `const` 키워드로 변수 선언
- 초기값 지정 가능 (`let x = 10;`)
- 블록 스코프 지원

### 연산자

- **산술**: `+`, `-`, `*`, `/`, `%`
- **비교**: `<`, `>`, `<=`, `>=`, `==`, `!=`
- **논리**: `&&`, `||`, `!`
- **단항**: `-` (음수)

### 제어문

- **조건문**: `if`, `if-else`, 중첩 조건문
- **반복문**: `while`, `for`

### 함수

- 함수 정의 및 호출
- 매개변수 전달
- 재귀 호출 지원
- 함수 호이스팅

### 출력

- `console.log(expr)` - 정수 또는 문자열 출력

---

## 7. 미구현/제한 사항

| 기능                           | 상태            |
| ------------------------------ | --------------- |
| 배열                           | 미지원          |
| 객체                           | 미지원          |
| 클로저                         | 미지원          |
| 화살표 함수                    | 미지원          |
| 문자열 연산                    | 미지원 (정수만) |
| 부동소수점                     | 미지원          |
| 증감 연산자 (`++`, `--`)       | 미지원          |
| `break`, `continue`            | 미지원          |
| 템플릿 리터럴 보간 (`${expr}`) | 미지원          |

---

## 8. 테스트 케이스

| #   | 파일명                     | 목적                                  |
| --- | -------------------------- | ------------------------------------- |
| 00  | `00_hello_string.js`       | 문자열 리터럴 (`""`, `''`, `` ` ` ``) |
| 01  | `01_basic_arithmetic.js`   | 산술 연산자                           |
| 02  | `02_comparison_ops.js`     | 비교 연산자                           |
| 03  | `03_logical_ops.js`        | 논리 연산자                           |
| 04  | `04_nested_if.js`          | 중첩 조건문                           |
| 05  | `05_while_loop.js`         | while 반복문                          |
| 06  | `06_for_loop.js`           | for 반복문                            |
| 07  | `07_multiple_functions.js` | 다중 함수 호출                        |
| 08  | `08_gcd.js`                | GCD 재귀 알고리즘                     |
| 09  | `09_variable_scope.js`     | 변수 스코프                           |
| 10  | `10_complex_expr.js`       | 연산자 우선순위                       |
| 11  | `11_fibonacci.js`          | 피보나치 재귀                         |
| 12  | `12_factorial.js`          | 팩토리얼 재귀                         |
| 13  | `13_sum.js`                | 합계 계산                             |
| 14  | `14_prime.js`              | 소수 판별                             |

자세한 테스트 설명은 [examples/TESTS.md](examples/TESTS.md)를 참고하세요.

---

## 9. 예제 코드

### Hello World

```javascript
function main() {
  console.log("Hello, World!");
  return 0;
}

main();
```

### Fibonacci (재귀)

```javascript
function fib(n) {
  if (n <= 1) {
    return n;
  }
  return fib(n - 1) + fib(n - 2);
}

// Top-level 실행
let i = 0;
while (i < 10) {
  console.log(fib(i));
  i = i + 1;
}
```

### Top-level Execution + 호이스팅

```javascript
// 함수 선언 전에 호출
console.log(square(5)); // 25

function square(x) {
  return x * x;
}

// Top-level 변수
let result = square(10);
console.log(result); // 100
```
