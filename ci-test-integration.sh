#!/bin/bash
# ci-test-integration.sh
# This script runs all tests and formats the results for CI/CD integration
# It generates JUnit-compatible test reports for Linux environments

# Default settings
INCLUDE_BOOST_TESTS=true
INCLUDE_MAIN_TESTS=true
COVERAGE=true
OUTPUT_DIR="$(pwd)/test-results"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
  key="$1"
  case $key in
    --no-boost)
    INCLUDE_BOOST_TESTS=false
    shift
    ;;
    --no-main)
    INCLUDE_MAIN_TESTS=false
    shift
    ;;
    --no-coverage)
    COVERAGE=false
    shift
    ;;
    --output=*)
    OUTPUT_DIR="${1#*=}"
    shift
    ;;
    *)
    # unknown option
    shift
    ;;
  esac
done

# Create output directory if it doesn't exist
mkdir -p "$OUTPUT_DIR"

# Initialize variables for tracking status
ALL_TESTS_PASSED=true
TEST_SUMMARY=()

# Run Boost tests if specified
if [ "$INCLUDE_BOOST_TESTS" = true ]; then
    echo -e "\033[0;36mRunning Boost tests...\033[0m"
    
    # Create Boost test results directory
    BOOST_RESULTS_DIR="$OUTPUT_DIR/boost-tests"
    mkdir -p "$BOOST_RESULTS_DIR"
    
    # Build the Boost test arguments
    BOOST_TEST_ARGS=""
    
    if [ "$COVERAGE" = true ]; then
        BOOST_TEST_ARGS="--boost-tests --coverage"
    else
        BOOST_TEST_ARGS="--boost-tests"
    fi
    
    # Run the tests
    ./run-tests-linux.sh $BOOST_TEST_ARGS
    
    # Check if tests passed
    if [ $? -eq 0 ]; then
        TEST_SUMMARY+=("Boost tests: Passed")
    else
        TEST_SUMMARY+=("Boost tests: Failed with exit code $?")
        ALL_TESTS_PASSED=false
    fi
    
    # Copy coverage report if it exists
    if [ -d "temp_boost_test/build/coverage_report" ]; then
        COVERAGE_OUTPUT_DIR="$BOOST_RESULTS_DIR/coverage"
        mkdir -p "$COVERAGE_OUTPUT_DIR"
        
        # Copy the coverage report
        cp -R temp_boost_test/build/coverage_report/* "$COVERAGE_OUTPUT_DIR/"
        TEST_SUMMARY+=("Boost test coverage report generated at: $COVERAGE_OUTPUT_DIR")
    fi
fi

# Run main application tests if specified
if [ "$INCLUDE_MAIN_TESTS" = true ]; then
    echo -e "\033[0;36mRunning main application tests...\033[0m"
    
    # Create main test results directory
    MAIN_RESULTS_DIR="$OUTPUT_DIR/main-tests"
    mkdir -p "$MAIN_RESULTS_DIR"
    
    # Run the tests
    ./run-tests-linux.sh
    
    # Check if tests passed
    if [ $? -eq 0 ]; then
        TEST_SUMMARY+=("Main application tests: Passed")
    else
        TEST_SUMMARY+=("Main application tests: Failed with exit code $?")
        ALL_TESTS_PASSED=false
    fi
fi

# Write test summary
echo -e "\n\033[0;36mTest Summary:\033[0m"
for result in "${TEST_SUMMARY[@]}"; do
    if [[ "$result" == *"Passed"* ]]; then
        echo -e "\033[0;32m✅ $result\033[0m"
    else
        echo -e "\033[0;31m❌ $result\033[0m"
    fi
done

# Create summary file for CI systems
SUMMARY_FILE="$OUTPUT_DIR/test-summary.md"
echo "# PiTrac Test Results" > "$SUMMARY_FILE"
echo -e "\n## Summary\n" >> "$SUMMARY_FILE"

for result in "${TEST_SUMMARY[@]}"; do
    if [[ "$result" == *"Passed"* ]]; then
        echo "- ✅ $result" >> "$SUMMARY_FILE"
    else
        echo "- ❌ $result" >> "$SUMMARY_FILE"
    fi
done

# Set the CI exit code based on the test results
if [ "$ALL_TESTS_PASSED" = true ]; then
    echo -e "\n\033[0;32mAll tests passed! 🎉\033[0m"
    exit 0
else
    echo -e "\n\033[0;31mSome tests failed. 😢\033[0m"
    exit 1
fi
