// Test 08: GCD (Greatest Common Divisor) using Euclidean Algorithm
// Purpose: Test recursive algorithm with modulo operator
// Expected: 6 (GCD of 48 and 18)

function gcd(a, b) {
    if (b == 0) {
        return a;
    }
    return gcd(b, a % b);
}

function main() {
    // GCD(48, 18)
    // = GCD(18, 48 % 18) = GCD(18, 12)
    // = GCD(12, 18 % 12) = GCD(12, 6)
    // = GCD(6, 12 % 6) = GCD(6, 0)
    // = 6
    console.log(gcd(48, 18));  // 6

    // More test cases
    console.log(gcd(100, 25));  // 25
    console.log(gcd(17, 13));   // 1 (coprime)
    console.log(gcd(36, 24));   // 12

    return 0;
}

main();
