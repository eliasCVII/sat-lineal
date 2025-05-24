#!/bin/bash

# Uso: ./run_tests.sh [executable] [--show-input]

EXECUTABLE=${1:-./tarea1.exe}
TEST_DIR=tests
SHOW_INPUT=0

# Check if --show-input flag is provided
if [ "$2" = "--show-input" ]; then
    SHOW_INPUT=1
fi

echo "Running tests with executable: $EXECUTABLE"
echo "----------------------------------------"

# Count tests
total_tests=$(ls $TEST_DIR/*.txt | wc -l)
passed_tests=0

# Run each test
for test_file in $TEST_DIR/*.txt; do
    test_name=$(basename "$test_file" .txt)
    echo "Running test $test_name..."

    # Display input expression if requested
    if [ $SHOW_INPUT -eq 1 ]; then
        echo "Input expression:"
        echo "\n----------------"
        cat "$test_file"
    fi

    # Run the test and capture output
    echo -n "Solver output: "
    output=$($EXECUTABLE < "$test_file" 2>&1)
    echo "$output"

    # Check if the output contains SATISFACIBLE, NO-SATISFACIBLE, or NO-SOLUTION
    if echo "$output" | grep -q "SATISFACIBLE\|NO-SATISFACIBLE\|NO-SOLUTION"; then
        echo "Test result: PASSED"
        ((passed_tests++))
    else
        echo "Test result: FAILED"
    fi
    echo ""
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
