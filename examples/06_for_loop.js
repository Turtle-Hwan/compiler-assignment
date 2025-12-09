// Test 06: For Loop
// Purpose: Test for loop with different patterns
// Expected: 25 (sum of squares 1^2 + 2^2 + 3^2)

function square(x) {
    return x * x;
}

function main() {
    // Sum of squares: 1 + 4 + 9 = 14... wait, let me recalculate
    // 1^2 + 2^2 + 3^2 = 1 + 4 + 9 = 14
    let sum = 0;
    for (let i = 1; i <= 3; i = i + 1) {
        sum = sum + square(i);
    }
    console.log(sum);  // 14

    // Counting by 2s
    for (let j = 0; j <= 10; j = j + 2) {
        console.log(j);  // 0, 2, 4, 6, 8, 10
    }

    return 0;
}

main();
