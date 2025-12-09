// Test 09: Variable Scope
// Purpose: Test variable scoping in functions and blocks
// Expected: 10, 100, 10, 50

function modifyLocal(x) {
    x = 100;  // Local modification
    return x;
}

function useOuter() {
    let outer = 50;
    return outer;
}

function main() {
    let global = 10;

    console.log(global);  // 10 (original)

    let result = modifyLocal(global);
    console.log(result);  // 100 (returned from function)

    console.log(global);  // 10 (unchanged - pass by value)

    console.log(useOuter());  // 50

    return 0;
}
