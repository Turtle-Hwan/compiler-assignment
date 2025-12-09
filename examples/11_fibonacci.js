// Fibonacci Sequence
// Prints first 10 Fibonacci numbers

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

main();
