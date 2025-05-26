#!/bin/bash
# configure-boost-tests-linux.sh
# Sets up the boost test environment for Linux

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_DIR="$SCRIPT_DIR/Software/LMSourceCode/ImageProcessing"
TEST_DIR="$SCRIPT_DIR/temp_boost_test"
TEST_EXE="$TEST_DIR/simple_tests"

# Make sure test directory exists
mkdir -p "$TEST_DIR"

# Copy test files
echo "Copying test files..."
cp "$SOURCE_DIR/boost_tests.cpp" "$TEST_DIR/"
cp "$SOURCE_DIR/gs_automated_testing.cpp" "$TEST_DIR/"
cp "$SOURCE_DIR/gs_automated_testing.h" "$TEST_DIR/"
cp "$SOURCE_DIR/gs_config.cpp" "$TEST_DIR/"
cp "$SOURCE_DIR/gs_config.h" "$TEST_DIR/"

# Create CMakeLists.txt
cat > "$TEST_DIR/CMakeLists.txt" << 'EOL'
cmake_minimum_required(VERSION 3.10)
project(PiTracTests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required packages
find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED COMPONENTS unit_test_framework filesystem system)

# Include directories
include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    .
)

# Add test executable
add_executable(simple_tests
    boost_tests.cpp
    gs_automated_testing.cpp
    gs_config.cpp
)

# Link libraries
target_link_libraries(simple_tests
    ${OpenCV_LIBS}
    ${Boost_LIBRARIES}
)

# Add compiler definitions for Boost Test
target_compile_definitions(simple_tests PRIVATE
    BOOST_TEST_DYN_LINK
    BOOST_TEST_MODULE=PiTracTests
)
EOL

# Add a simple build script
cat > "$TEST_DIR/build-and-run.sh" << 'EOL'
#!/bin/bash
set -e

# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake ..

# Build
make -j$(nproc)

# Run tests
./simple_tests --log_level=test_suite

if [ $? -eq 0 ]; then
    echo -e "\033[0;32mTests passed successfully!\033[0m"
else
    echo -e "\033[0;31mTests failed!\033[0m"
    exit 1
fi
EOL

# Make it executable
chmod +x "$TEST_DIR/build-and-run.sh"

echo -e "\033[0;32mBoost test environment configured for Linux.\033[0m"
echo -e "To build and run tests, execute:"
echo -e "\033[0;36mcd $TEST_DIR && ./build-and-run.sh\033[0m"
