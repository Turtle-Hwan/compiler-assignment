// Sum from 1 to N using for loop

function sumTo(n) {
    let sum = 0;
    for (let i = 1; i <= n; i = i + 1) {
        sum = sum + i;
    }
    return sum;
}

function main() {
    // Sum 1 to 100 = 5050
    let result = sumTo(100);
    console.log(result);
    return result;
}
