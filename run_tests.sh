#!/bin/bash

# Run all tests in the tests directory
# Usage: ./run_tests.sh [executable]

EXECUTABLE=${1:-./tarea1.exe}
TEST_DIR=tests

echo "Running tests with executable: $EXECUTABLE"
echo "----------------------------------------"

# Count tests
total_tests=$(ls $TEST_DIR/*.expr | wc -l)
passed_tests=0

# Run each test
for test_file in $TEST_DIR/*.expr; do
    test_name=$(basename "$test_file" .expr)
    echo -n "Running test $test_name... "

    # Run the test and capture output
    output=$($EXECUTABLE < "$test_file" 2>&1)

    # Check if the output contains SATISFACIBLE, NO-SATISFACIBLE, or NO-SOLUTION
    if echo "$output" | grep -q "SATISFACIBLE\|NO-SATISFACIBLE\|NO-SOLUTION"; then
        echo "PASSED"
        ((passed_tests++))
    else
        echo "FAILED"
        echo "Output: $output"
    fi
done

echo "----------------------------------------"
echo "Tests passed: $passed_tests / $total_tests"

if [ $passed_tests -eq $total_tests ]; then
    echo "All tests passed!"
    exit 0
else
    echo "Some tests failed."
    exit 1
fi
