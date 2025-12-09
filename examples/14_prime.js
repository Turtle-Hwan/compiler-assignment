// Prime Number Check
// Prints all prime numbers from 2 to 50

function isPrime(n) {
    if (n <= 1) {
        return 0;
    }
    let i = 2;
    while (i * i <= n) {
        if (n % i == 0) {
            return 0;
        }
        i = i + 1;
    }
    return 1;
}

function main() {
    let n = 2;
    while (n <= 50) {
        if (isPrime(n)) {
            console.log(n);
        }
        n = n + 1;
    }
    return 0;
}
