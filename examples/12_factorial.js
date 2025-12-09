// Factorial Calculation
// Prints factorial of 1 to 10

function factorial(n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

function main() {
    let n = 1;
    while (n <= 10) {
        let result = factorial(n);
        console.log(result);
        n = n + 1;
    }
    return 0;
}
