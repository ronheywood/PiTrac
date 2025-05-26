# temp_boost_test/build-and-run.sh
#!/bin/bash
# Build and run the Boost Test framework tests

set -e  # Exit on error

# Parse command line arguments
LOG_LEVEL="--log_level=test_suite"
for arg in "$@"
do
    case $arg in
        --log_level=all)
        LOG_LEVEL="--log_level=all"
        shift
        ;;
        *)
        # Unknown option
        ;;
    esac
done

# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake ..

# Build
echo "Building tests..."
make -j$(nproc)

# Run tests
echo "Running tests..."
./simple_tests $LOG_LEVEL

if [ $? -eq 0 ]; then
    echo -e "\033[0;32mTests passed successfully!\033[0m"
else
    echo -e "\033[0;31mTests failed!\033[0m"
    exit 1
fi
