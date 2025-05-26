#!/bin/bash
# run-tests-linux.sh
# Runs the automated tests for PiTrac

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_DIR="$SCRIPT_DIR/Software/LMSourceCode/ImageProcessing"
BUILD_DIR="$SOURCE_DIR/build"
EXECUTABLE="$BUILD_DIR/pitrac_lm"

# Parse command line arguments
TEST_TYPE="main"  # Default to main application tests
VERBOSE=""
COVERAGE=""
TEST_FILTER=""

# Process all arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        --boost-tests)
        TEST_TYPE="boost"
        shift
        ;;
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
        *)
        # Unknown option
        shift
        ;;
    esac
done

# Ensure environment variables are set
export PITRAC_ROOT="$SCRIPT_DIR/Software/LMSourceCode"

if [ "$TEST_TYPE" = "boost" ]; then
    # Run Boost tests
    echo -e "\033[0;36mRunning Boost unit tests...\033[0m"
    
    BOOST_TEST_DIR="$SCRIPT_DIR/temp_boost_test"
    BOOST_EXECUTABLE="$BOOST_TEST_DIR/build/simple_tests"
    
    # Check if we need to set up the Boost test environment
    if [ ! -f "$BOOST_TEST_DIR/build-and-run.sh" ]; then
        echo -e "\033[0;33mSetting up Boost test environment...\033[0m"
        "$SCRIPT_DIR/configure-boost-tests-linux.sh"
    fi
    
    # Run the tests
    pushd "$BOOST_TEST_DIR" > /dev/null
    
    # Use enhanced build script if it exists
    if [ -f "build-and-run-enhanced.sh" ]; then
        chmod +x build-and-run-enhanced.sh
        ./build-and-run-enhanced.sh $VERBOSE $TEST_FILTER $COVERAGE
    else
        ./build-and-run.sh $VERBOSE
    fi
    TEST_RESULT=$?
    popd > /dev/null
    
    exit $TEST_RESULT
else
    # Run main application tests
    # Check if executable exists
    if [ ! -f "$EXECUTABLE" ]; then
        echo -e "\033[0;31mExecutable not found: $EXECUTABLE\033[0m"
        echo -e "\033[0;33mPlease build the solution first using build-and-run-linux.sh\033[0m"
        exit 1
    fi
    
    # Change to the directory where the executable is located
    cd "$BUILD_DIR"
    
    # Set LD_LIBRARY_PATH to ensure shared libraries are found
    export LD_LIBRARY_PATH="$BUILD_DIR:$LD_LIBRARY_PATH"
    
    # Run the tests
    echo -e "\033[0;36mRunning PiTrac automated tests...\033[0m"
    
    "$EXECUTABLE" \
        --logging_level info \
        --artifact_save_level all \
        --system_mode automated_testing \
        --test_overrides 1  # Force test mode even if not debug build
    
    # Check exit code
    EXIT_CODE=$?
    if [ $EXIT_CODE -eq 0 ]; then
        echo -e "\033[0;32mTests completed successfully!\033[0m"
    else
        echo -e "\033[0;31mTests failed with exit code $EXIT_CODE\033[0m"
    fi
    
    exit $EXIT_CODE
fi
