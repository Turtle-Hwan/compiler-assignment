# Mini-JS 테스트 프로그램 목록

이 문서는 Mini-JS 컴파일러의 테스트 프로그램들에 대한 설명입니다.

## 테스트 요약

| 번호 | 파일명                     | 테스트 목적         | 주요 기능                        |
| ---- | -------------------------- | ------------------- | -------------------------------- |
| 00   | `00_hello_string.js`       | 문자열 출력 테스트  | `"..."`, `'...'`, `` `...` ``    |
| 01   | `01_basic_arithmetic.js`   | 산술 연산자 테스트  | `+`, `-`, `*`, `/`, `%`          |
| 02   | `02_comparison_ops.js`     | 비교 연산자 테스트  | `<`, `>`, `<=`, `>=`, `==`, `!=` |
| 03   | `03_logical_ops.js`        | 논리 연산자 테스트  | `&&`, `\|\|`                     |
| 04   | `04_nested_if.js`          | 중첩 조건문 테스트  | if-else 중첩                     |
| 05   | `05_while_loop.js`         | while 반복문 테스트 | while                            |
| 06   | `06_for_loop.js`           | for 반복문 테스트   | for                              |
| 07   | `07_multiple_functions.js` | 함수 조합 테스트    | 함수 간 호출                     |
| 08   | `08_gcd.js`                | GCD 알고리즘        | 재귀 + 나머지 연산               |
| 09   | `09_variable_scope.js`     | 변수 스코프 테스트  | 지역/전역 변수                   |
| 10   | `10_complex_expr.js`       | 복합 표현식 테스트  | 연산자 우선순위                  |
| 11   | `11_fibonacci.js`          | 피보나치 수열       | 재귀 함수 + while                |
| 12   | `12_factorial.js`          | 팩토리얼 계산       | 재귀 함수                        |
| 13   | `13_sum.js`                | 합계 계산           | for 반복문                       |
| 14   | `14_prime.js`              | 소수 판별           | while + 조건문                   |

---

## 상세 설명

### 00. Hello String (`00_hello_string.js`)

**목적**: 세 가지 문자열 리터럴 형식 테스트

**테스트 내용**:

- Double quotes: `"Hello, World!"`
- Single quotes: `'Single quotes work too'`
- Backticks: `` `Backticks also supported` ``

**기대 출력**:

```
Hello, World!
Single quotes work too
Backticks also supported
```

---

### 01. Basic Arithmetic (`01_basic_arithmetic.js`)

**목적**: 모든 산술 연산자의 정확한 동작 확인

**테스트 내용**:

- 덧셈: `10 + 5 = 15`
- 뺄셈: `10 - 5 = 5`
- 곱셈: `10 * 5 = 50`
- 나눗셈: `10 / 5 = 2`
- 나머지: `10 % 5 = 0`

**기대 출력**:

```
15
5
50
2
0
```

---

### 02. Comparison Operators (`02_comparison_ops.js`)

**목적**: 비교 연산자 및 조건 분기 동작 확인

**테스트 내용**:

- `<`: 10 < 20 → true (1)
- `>`: 10 > 20 → false (0)
- `<=`: 10 <= 10 → true (1)
- `>=`: 10 >= 20 → false (0)
- `==`: 10 == 10 → true (1)
- `!=`: 10 != 20 → true (1)

**기대 출력**:

```
1
0
1
0
1
1
```

---

### 03. Logical Operators (`03_logical_ops.js`)

**목적**: 논리 연산자 AND, OR 동작 확인

**테스트 내용**:

- AND: true && false → false (0)
- OR: true || false → true (1)
- 복합: (true && true) || false → true (1)
- 복합: false && (true || true) → false (0)

**기대 출력**:

```
0
1
1
0
```

---

### 04. Nested If-Else (`04_nested_if.js`)

**목적**: 중첩 조건문의 정확한 분기 처리 확인

**테스트 내용**:

- score >= 90: 3 (excellent)
- score >= 70: 2 (medium)
- score >= 50: 1 (pass)
- score < 50: 0 (fail)

**기대 출력**:

```
2
3
1
0
```

---

### 05. While Loop (`05_while_loop.js`)

**목적**: while 반복문의 반복 및 종료 조건 확인

**테스트 내용**:

- 1부터 10까지 합: 55
- 5부터 1까지 카운트다운

**기대 출력**:

```
55
5
4
3
2
1
```

---

### 06. For Loop (`06_for_loop.js`)

**목적**: for 반복문과 함수 호출 결합 테스트

**테스트 내용**:

- 제곱의 합: 1² + 2² + 3² = 14
- 2씩 증가: 0, 2, 4, 6, 8, 10

**기대 출력**:

```
14
0
2
4
6
8
10
```

---

### 07. Multiple Functions (`07_multiple_functions.js`)

**목적**: 함수 간 호출 및 함수 조합 테스트

**테스트 내용**:

- square(5) = 25
- cube(5) = 125 (uses square)
- power4(5) = 625 (square(square(5)))

**기대 출력**:

```
25
125
625
```

---

### 08. GCD Algorithm (`08_gcd.js`)

**목적**: 유클리드 알고리즘을 통한 재귀 + 나머지 연산 테스트

**테스트 내용**:

- GCD(48, 18) = 6
- GCD(100, 25) = 25
- GCD(17, 13) = 1 (서로소)
- GCD(36, 24) = 12

**기대 출력**:

```
6
25
1
12
```

---

### 09. Variable Scope (`09_variable_scope.js`)

**목적**: 함수 내 지역 변수와 값 전달 방식 확인

**테스트 내용**:

- 함수에 전달된 값 수정이 원본에 영향 없음 (값 전달)
- 각 함수의 지역 변수는 독립적

**기대 출력**:

```
10
100
10
50
```

---

### 10. Complex Expressions (`10_complex_expr.js`)

**목적**: 연산자 우선순위 및 괄호 처리 확인

**테스트 내용**:

- `3 + 2 * 7 = 17` (곱셈 우선)
- `10 / 2 - 4 = 1`
- `(2 + 3) * 4 - 6 = 14` (괄호 우선)

**기대 출력**:

```
17
1
14
```

---

### 11. Fibonacci Sequence (`11_fibonacci.js`)

**목적**: 재귀 함수 및 while 반복문 결합 테스트

**테스트 내용**:

- fib(n) = fib(n-1) + fib(n-2)
- 0부터 9까지의 피보나치 수 출력

**기대 출력**:

```
0
1
1
2
3
5
8
13
21
34
```

---

### 12. Factorial (`12_factorial.js`)

**목적**: 재귀 함수 테스트

**테스트 내용**:

- factorial(n) = n \* factorial(n-1)
- 1!부터 10!까지 출력

**기대 출력**:

```
1
2
6
24
120
720
5040
40320
362880
3628800
```

---

### 13. Sum Calculation (`13_sum.js`)

**목적**: for 반복문을 이용한 누적 합계 계산

**테스트 내용**:

- sumTo(100) = 1 + 2 + ... + 100 = 5050

**기대 출력**:

```
5050
```

---

### 14. Prime Number Check (`14_prime.js`)

**목적**: 조건문과 반복문을 결합한 복잡한 알고리즘 테스트

**테스트 내용**:

- isPrime(n) 함수로 소수 판별
- 2부터 50까지의 소수 출력

**기대 출력**:

```
2
3
5
7
11
13
17
19
23
29
31
37
41
43
47
```

---

## 실행 방법

```bash
# 전체 예제 자동 실행 (빌드 후)
make test

# 또는 직접 스크립트를 호출
sh tests/run_examples.sh ./minijs
```

```bash
# 인터프리터 모드로 실행
./minijs -e examples/01_basic_arithmetic.js

# 모든 테스트 실행 (Linux/Mac)
for f in examples/*.js; do echo "=== $f ==="; ./minijs -e "$f"; done

# Windows
for %f in (examples\*.js) do @echo === %f === && minijs.exe -e %f
```

## 커버리지 요약

이 테스트 프로그램들은 다음 기능들을 모두 커버합니다:

- ✅ **문자열 리터럴**: `"..."`, `'...'`, `` `...` ``
- ✅ **변수 선언 및 대입**: `let`, `var`, `const`
- ✅ **산술 연산**: `+`, `-`, `*`, `/`, `%`
- ✅ **비교 연산**: `<`, `>`, `<=`, `>=`, `==`, `!=`
- ✅ **논리 연산**: `&&`, `||`
- ✅ **조건문**: `if`, `else`, 중첩 조건문
- ✅ **반복문**: `while`, `for`
- ✅ **함수**: 정의, 호출, 재귀, 다중 함수
- ✅ **출력**: `console.log()`
- ✅ **연산자 우선순위**: 괄호 및 우선순위 처리
- ✅ **변수 스코프**: 지역 변수, 값 전달
