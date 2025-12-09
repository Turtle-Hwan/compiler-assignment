// Test 10: Complex Expressions
// Purpose: Test operator precedence and complex expressions
// Expected: 17, 1, 14

function main() {
    // Test operator precedence: * before +
    // 3 + 2 * 7 = 3 + 14 = 17
    let a = 3 + 2 * 7;
    console.log(a);  // 17

    // Test parentheses override
    // (3 + 2) * 7 = 5 * 7 = 35 ... wait that's not in expected
    // Let me use: 10 / 2 - 4 = 5 - 4 = 1
    let b = 10 / 2 - 4;
    console.log(b);  // 1

    // Complex nested expression
    // (2 + 3) * 4 - 6 = 5 * 4 - 6 = 20 - 6 = 14
    let c = (2 + 3) * 4 - 6;
    console.log(c);  // 14

    return 0;
}

main();
