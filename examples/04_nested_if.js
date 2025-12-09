// Test 04: Nested If-Else Statements
// Purpose: Test nested conditional logic
// Expected: 2 (medium category)

function categorize(score) {
    if (score >= 90) {
        return 3;  // excellent
    } else {
        if (score >= 70) {
            return 2;  // medium
        } else {
            if (score >= 50) {
                return 1;  // pass
            } else {
                return 0;  // fail
            }
        }
    }
}

function main() {
    let score = 85;
    let result = categorize(score);
    console.log(result);  // 2 (medium)

    // Test other cases
    console.log(categorize(95));  // 3 (excellent)
    console.log(categorize(65));  // 1 (pass)
    console.log(categorize(40));  // 0 (fail)

    return 0;
}
