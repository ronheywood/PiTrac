#!/bin/bash
# run-boost-tests-linux.sh
# Runs the Boost Test framework tests for PiTrac

set -e  # Exit on error

# Parse command line arguments
VERBOSE=""
COVERAGE=""
TEST_FILTER=""
USE_ENHANCED=""

# Process arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        --verbose)
        VERBOSE="--log_level=all"
        shift
        ;;
        --coverage)
        COVERAGE="--coverage"
        shift
        ;;
        --test-filter=*)
        TEST_FILTER="--run_test=${1#*=}"
        shift
        ;;
        --enhanced)
        USE_ENHANCED="1"
        shift
        ;;
        *)
        # Unknown option
        shift
        ;;
    esac
done

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_DIR="$SCRIPT_DIR/Software/LMSourceCode/ImageProcessing"
BUILD_DIR="$SOURCE_DIR/build"
EXECUTABLE="$BUILD_DIR/pitrac_tests"

# Ensure environment variables are set
export PITRAC_ROOT="$SCRIPT_DIR/Software/LMSourceCode"

# Create build directory if it doesn't exist
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Copy enhanced test file if requested
if [ -n "$USE_ENHANCED" ] && [ -f "$SOURCE_DIR/boost_tests_enhanced.cpp" ]; then
    echo -e "\033[0;36mUsing enhanced Boost tests file\033[0m"
    cp "$SOURCE_DIR/boost_tests_enhanced.cpp" "$SOURCE_DIR/boost_tests.cpp"
fi

# Configure and build tests
echo -e "\033[0;36mBuilding PiTrac tests...\033[0m"
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=$([[ -n "$COVERAGE" ]] && echo "ON" || echo "OFF")
make -j$(nproc) pitrac_tests

# Check if build succeeded and test executable exists
if [ ! -f "$EXECUTABLE" ]; then
    echo -e "\033[0;31mTest executable not found: $EXECUTABLE\033[0m"
    exit 1
fi

# Set LD_LIBRARY_PATH to ensure shared libraries are found
export LD_LIBRARY_PATH="$BUILD_DIR:$LD_LIBRARY_PATH"

# Run the tests
echo -e "\033[0;36mRunning Boost tests...\033[0m"

# Set default log level if not specified
if [ -z "$VERBOSE" ]; then
    VERBOSE="--log_level=test_suite"
fi

# Run tests with or without coverage
if [ -n "$COVERAGE" ]; then
    # Check if gcovr is installed
    if ! command -v gcovr &> /dev/null; then
        echo -e "\033[0;33mInstalling gcovr for coverage reporting\033[0m"
        pip install gcovr
    fi
    
    # Run tests
    "$EXECUTABLE" $VERBOSE $TEST_FILTER
    EXIT_CODE=$?
    
    # Generate coverage report if tests passed
    mkdir -p "$BUILD_DIR/coverage_report"
    echo -e "\033[0;36mGenerating coverage report\033[0m"
    gcovr -r "$SOURCE_DIR" --html --html-details -o "$BUILD_DIR/coverage_report/index.html"
    
    echo -e "\033[0;36mCoverage report generated at $BUILD_DIR/coverage_report/index.html\033[0m"
else
    # Run without coverage
    "$EXECUTABLE" $VERBOSE $TEST_FILTER
    EXIT_CODE=$?
fi

# Check exit code
if [ $EXIT_CODE -eq 0 ]; then
    echo -e "\033[0;32mBoost tests completed successfully!\033[0m"
else
    echo -e "\033[0;31mBoost tests failed with exit code $EXIT_CODE\033[0m"
fi

exit $EXIT_CODE
