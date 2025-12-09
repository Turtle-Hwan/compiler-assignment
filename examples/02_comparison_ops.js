// Test 02: Comparison Operators
// Purpose: Test all comparison operators (<, >, <=, >=, ==, !=)
// Expected: 1, 0, 1, 0, 1, 1

function main() {
    let x = 10;
    let y = 20;
    let z = 10;

    // Less than
    if (x < y) {
        console.log(1);   // true: 10 < 20
    } else {
        console.log(0);
    }

    // Greater than
    if (x > y) {
        console.log(1);
    } else {
        console.log(0);   // false: 10 > 20
    }

    // Less than or equal
    if (x <= z) {
        console.log(1);   // true: 10 <= 10
    } else {
        console.log(0);
    }

    // Greater than or equal
    if (x >= y) {
        console.log(1);
    } else {
        console.log(0);   // false: 10 >= 20
    }

    // Equal
    if (x == z) {
        console.log(1);   // true: 10 == 10
    } else {
        console.log(0);
    }

    // Not equal
    if (x != y) {
        console.log(1);   // true: 10 != 20
    } else {
        console.log(0);
    }

    return 0;
}
