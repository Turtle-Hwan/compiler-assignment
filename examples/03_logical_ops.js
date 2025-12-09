// Test 03: Logical Operators
// Purpose: Test logical AND (&&) and OR (||) operators
// Expected: 0, 1, 1, 0

function main() {
    let a = 1;  // true
    let b = 0;  // false

    // AND: true && false = false
    if (a && b) {
        console.log(1);
    } else {
        console.log(0);   // 0
    }

    // OR: true || false = true
    if (a || b) {
        console.log(1);   // 1
    } else {
        console.log(0);
    }

    // Complex: (true && true) || false = true
    let c = 1;
    if ((a && c) || b) {
        console.log(1);   // 1
    } else {
        console.log(0);
    }

    // Complex: false && (true || true) = false
    if (b && (a || c)) {
        console.log(1);
    } else {
        console.log(0);   // 0
    }

    return 0;
}

main();
