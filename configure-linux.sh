#!/bin/bash
# configure-linux.sh
# Configure Linux/WSL Ubuntu for PiTrac Development
# This script installs necessary dependencies including OpenCV, Boost, and sets up environment variables.

# Exit on any error
set -e

echo "--- PiTrac Linux Configuration Script ---"

# Function to check if a package is installed
is_installed() {
    dpkg -l | grep -q $1
    return $?
}

# Function to install required packages
install_packages() {
    echo "Installing required packages..."
    sudo apt-get update
    
    # Essential build tools
    sudo apt-get install -y build-essential cmake git pkg-config
    
    # Image I/O libraries
    sudo apt-get install -y libjpeg-dev libpng-dev libtiff-dev
    
    # Video I/O libraries
    sudo apt-get install -y libavcodec-dev libavformat-dev libswscale-dev
    sudo apt-get install -y libv4l-dev libxvidcore-dev libx264-dev
    
    # GUI libraries (for OpenCV highgui module)
    sudo apt-get install -y libgtk-3-dev
    
    # Other optimization libraries
    sudo apt-get install -y libatlas-base-dev gfortran
    
    # Python dev packages (if needed)
    sudo apt-get install -y python3-dev
    
    # Install Boost
    sudo apt-get install -y libboost1.74-all-dev
}

# Function to set environment variables
set_environment_vars() {
    echo "Setting up environment variables..."
    
    # Set PITRAC_ROOT environment variable
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    PITRAC_ROOT="$SCRIPT_DIR/Software/LMSourceCode"
    
    # Append to ~/.bashrc if not already present
    if ! grep -q "export PITRAC_ROOT=" ~/.bashrc; then
        echo "# PiTrac environment variables" >> ~/.bashrc
        echo "export PITRAC_ROOT=\"$PITRAC_ROOT\"" >> ~/.bashrc
    fi
    
    # Create a boost.pc file for pkg-config if not exists
    if [ ! -f /usr/share/pkgconfig/boost.pc ]; then
        echo "Creating boost.pc file for pkg-config..."
        sudo bash -c 'cat > /usr/share/pkgconfig/boost.pc << EOL
# Package Information for pkg-config
# Path to where Boost is installed
prefix=/usr
# Path to where libraries are
libdir=\${prefix}/lib
# Path to where include files are
includedir=\${prefix}/boost
Name: Boost
Description: Boost provides free peer-reviewed portable C++ source libraries
Version: 1.74.0
Libs: -L\${libdir} -lboost_filesystem -lboost_system -lboost_timer -lboost_log -lboost_chrono -lboost_regex -lboost_thread -lboost_program_options
Cflags: -isystem \${includedir}
EOL'
    fi
    
    # Fix the boost awaitable.hpp issue mentioned in your documentation
    if ! grep -q "#include <utility>" /usr/include/boost/asio/awaitable.hpp; then
        echo "Fixing boost awaitable.hpp for C++20 compatibility..."
        # Find the line before "namespace boost" and add the include
        sudo sed -i '/^namespace boost/i #include <utility>' /usr/include/boost/asio/awaitable.hpp
    fi
    
    echo "Environment variables set successfully."
}

# Function to create build and run script
create_build_script() {
    echo "Creating build-and-run-linux.sh script..."
    cat > "$SCRIPT_DIR/build-and-run-linux.sh" << 'EOL'
#!/bin/bash
# build-and-run-linux.sh
# Builds the PiTrac solution and runs the ImageProcessing executable

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_DIR="$SCRIPT_DIR/Software/LMSourceCode/ImageProcessing"
BUILD_DIR="$SOURCE_DIR/build"

# Create build directory if it doesn't exist
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure and build
echo -e "\033[0;36mBuilding PiTrac...\033[0m"
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)

# Check if build succeeded
if [ $? -ne 0 ]; then
    echo -e "\033[0;31mBuild failed!\033[0m"
    exit 1
else
    echo -e "\033[0;32mBuild succeeded!\033[0m"
fi

# Run the program
echo -e "\033[0;36mRunning PiTrac...\033[0m"
LD_LIBRARY_PATH="$BUILD_DIR:$LD_LIBRARY_PATH" ./pitrac_lm --show_images 1 \
    --lm_comparison_mode=0 \
    --logging_level info \
    --artifact_save_level=all \
    --wait_keys 0 \
    --system_mode camera1_test_standalone \
    --search_center_x 800 \
    --search_center_y 550
EOL

    chmod +x "$SCRIPT_DIR/build-and-run-linux.sh"
}

# Function to create test script
create_test_script() {
    echo "Creating run-tests-linux.sh script..."
    cat > "$SCRIPT_DIR/run-tests-linux.sh" << 'EOL'
#!/bin/bash
# run-tests-linux.sh
# Runs the automated tests for PiTrac

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_DIR="$SCRIPT_DIR/Software/LMSourceCode/ImageProcessing"
BUILD_DIR="$SOURCE_DIR/build"
EXECUTABLE="$BUILD_DIR/pitrac_lm"

# Ensure environment variables are set
export PITRAC_ROOT="$SCRIPT_DIR/Software/LMSourceCode"

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
EOL

    chmod +x "$SCRIPT_DIR/run-tests-linux.sh"
}

# Main execution
install_packages
set_environment_vars
create_build_script
create_test_script

echo "Setup complete!"
echo "You can now build and run the solution with: ./build-and-run-linux.sh"
echo "Or run automated tests with: ./run-tests-linux.sh"

# Source the bashrc to make environment variables available in the current session
source ~/.bashrc
