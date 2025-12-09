// Test 05: While Loop
// Purpose: Test while loop with counter and accumulator
// Expected: 55 (sum of 1 to 10)

function main() {
    let sum = 0;
    let i = 1;

    while (i <= 10) {
        sum = sum + i;
        i = i + 1;
    }

    console.log(sum);  // 55

    // Test countdown
    let count = 5;
    while (count > 0) {
        console.log(count);  // 5, 4, 3, 2, 1
        count = count - 1;
    }

    return 0;
}
