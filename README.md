# Mini-JS Compiler

WebAssembly 기반 JavaScript-like 언어 컴파일러

## 개요

Mini-JS는 JavaScript의 핵심 기능을 지원하는 교육용 컴파일러입니다.
기존 Mini-C 컴파일러를 기반으로 JavaScript 스타일 문법을 지원하도록 개조하였습니다.

## 지원 기능

### 문법

- **함수 정의**: `function name(params) { ... }`
- **변수 선언**: `let`, `var`, `const`
- **리터럴**: 정수, 문자열 (`"..."`, `'...'`, `` `...` ``)
- **출력**: `console.log(expr)`
- **제어문**: `if`, `else`, `while`, `for`
- **연산자**: `+`, `-`, `*`, `/`, `%`, `<`, `>`, `<=`, `>=`, `==`, `!=`, `&&`, `||`

### 백엔드

1. **인터프리터**: AST를 직접 실행하여 결과 출력
2. **코드 생성기**: x86-64 어셈블리 코드 생성

## 빌드 방법

### 필요 도구

- GCC (또는 MinGW)
- Flex (lexer 생성)
- Bison (parser 생성)
- Emscripten (WebAssembly 빌드용, 선택사항)

### 빌드 명령어

```bash
make            # 데스크톱 버전 빌드
make wasm       # WebAssembly 버전 빌드
make test       # 테스트 실행
make clean      # 정리
```

Windows에서는 WSL 또는 MinGW + MSYS2 환경에서 make를 사용하세요.

## 사용 방법

### 명령줄 (데스크톱)

```bash
# 인터프리터 모드 (실행)
./minijs -e input.js

# 컴파일 모드 (어셈블리 생성)
./minijs -c input.js -o output.s
```

### 웹 (WebAssembly)

1. `make wasm`으로 빌드
2. `docs/index.html`을 웹 브라우저로 열기
3. 코드 입력 후 "Compile & Run" 클릭

## 예제 코드

### Hello World (문자열 출력)

```javascript
function main() {
  console.log("Hello, World!");
  console.log('Single quotes work too');
  console.log(`Backticks also supported`);
  return 0;
}
```

### Hello World (숫자 출력)

```javascript
function main() {
  console.log(42);
  return 0;
}
```

### Fibonacci

```javascript
function fib(n) {
  if (n <= 1) {
    return n;
  }
  return fib(n - 1) + fib(n - 2);
}

function main() {
  let i = 0;
  while (i < 10) {
    console.log(fib(i));
    i = i + 1;
  }
  return 0;
}
```

### For 루프

```javascript
function main() {
  let sum = 0;
  for (let i = 1; i <= 100; i = i + 1) {
    sum = sum + i;
  }
  console.log(sum); // 5050
  return sum;
}
```

## 프로젝트 구조

```
my_mini_js/
├── include/
│   ├── ast.h           # AST 정의 (12wk 기반)
│   ├── codegen_x86.h   # 코드 생성기 헤더
│   ├── eval.h          # 인터프리터 헤더
│   └── symtab.h        # 심볼 테이블 (10wk 기반 + 스코프 확장)
├── src/
│   ├── ast.c           # AST 구현 (12wk 기반)
│   ├── codegen_x86.c   # x86-64 코드 생성기 (12wk + 11wk 통합)
│   ├── eval.c          # 인터프리터 (symtab 사용)
│   ├── symtab.c        # 심볼 테이블 (10wk 기반 + 스코프 확장)
│   ├── main.c          # 메인 (데스크톱)
│   └── web_driver.c    # 웹 인터페이스 (Wasm)
├── parser/
│   ├── scanner.l       # Flex 렉서 (JS 문법)
│   └── parser.y        # Bison 파서 (12wk 기반 + JS 문법)
├── docs/
│   └── index.html      # 웹 프론트엔드
├── examples/           # 예제 코드
├── Makefile            # 빌드 스크립트
└── README.md           # 이 파일
```

## 컴파일 과정

```
JavaScript Source
       ↓
   [Lexer (Flex)]
       ↓
     Tokens
       ↓
   [Parser (Bison)]
       ↓
      AST
       ↓
   ┌───┴───┐
   ↓       ↓
[Eval]  [Codegen]
   ↓       ↓
 결과   x86 Assembly
```

## 참고

이 프로젝트는 컴파일러 설계 수업 과제로 제작되었습니다.
기존 Mini-C 코드베이스(10wk, 11wk, 12wk)를 기반으로 합니다.

### 기반 코드 및 재사용 비율

| 컴포넌트          | 기반 코드                   | 재사용률          |
| ----------------- | --------------------------- | ----------------- |
| ast.h/ast.c       | 12wk/minic_x86_call_func2   | 60-65%            |
| codegen_x86.c     | 12wk (함수) + 11wk (제어문) | 80%               |
| parser.y          | 12wk/minic_x86_call_func2   | 50%               |
| symtab.h/symtab.c | 10wk/minic_exec             | 70% (스코프 확장) |
| eval.c            | 10wk symtab 인터페이스 사용 | 40%               |

**전체 코드 재사용률: 약 55%**

### 주요 재사용 내용

- **12wk/minic_x86_call_func2**: AST 구조, 함수 정의/호출, 파서 문법
- **11wk/minic_x86_for_if**: 제어문 코드 생성 (if/while 레이블 점프)
- **10wk/minic_exec**: 심볼 테이블 (sym_set, sym_get) + 스코프 지원 확장
