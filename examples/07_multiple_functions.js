// Test 07: Multiple Functions and Function Composition
// Purpose: Test multiple function definitions and calling between functions
// Expected: 25 (5^2), 125 (5^3), 625 (5^4)

function square(x) {
    return x * x;
}

function cube(x) {
    return x * square(x);  // Uses square function
}

function power4(x) {
    return square(square(x));  // Composition: (x^2)^2 = x^4
}

function main() {
    let n = 5;

    console.log(square(n));  // 25
    console.log(cube(n));    // 125
    console.log(power4(n));  // 625

    return 0;
}

main();
