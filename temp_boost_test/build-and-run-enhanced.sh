#!/bin/bash
# build-and-run-enhanced.sh
# Enhanced script to build and run PiTrac Boost tests on Linux with 
# improved command line argument handling and code coverage option

# Default settings
LOG_LEVEL="--log_level=test_suite"
TEST_FILTER=""
USE_COVERAGE=0

# Parse command line arguments
while [[ $# -gt 0 ]]; do
  key="$1"
  case $key in
    --log_level=*)
    LOG_LEVEL="$1"
    shift
    ;;
    --run_test=*)
    TEST_FILTER="$1"
    shift
    ;;
    --coverage)
    USE_COVERAGE=1
    shift
    ;;
    *)
    echo "Unknown option: $1"
    shift
    ;;
  esac
done

# Create build directory if it doesn't exist
mkdir -p build
cd build || exit

# Configure and build with CMake
echo "Configuring and building PiTrac tests..."
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . -- -j$(nproc)

if [ $? -ne 0 ]; then
  echo "Build failed!"
  exit 1
fi

# Run tests with or without coverage
if [ $USE_COVERAGE -eq 1 ]; then
  echo "Running tests with coverage..."
  
  # Check if gcovr is installed
  if ! command -v gcovr &> /dev/null; then
    echo "gcovr not found. Installing..."
    pip install gcovr
  fi
  
  # Run with coverage
  ./boost_tests $LOG_LEVEL $TEST_FILTER
  TEST_RESULT=$?
  
  # Generate coverage report
  echo "Generating coverage report..."
  mkdir -p coverage_report
  gcovr -r .. --html --html-details -o coverage_report/index.html
  
  echo "Coverage report generated at $(pwd)/coverage_report/index.html"
else
  # Run tests without coverage
  echo "Running tests..."
  ./boost_tests $LOG_LEVEL $TEST_FILTER
  TEST_RESULT=$?
fi

if [ $TEST_RESULT -eq 0 ]; then
  echo "Tests passed!"
else
  echo "Tests failed with error code $TEST_RESULT"
fi

exit $TEST_RESULT
