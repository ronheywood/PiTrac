#!/bin/bash
# clean-test-environment.sh
# This script cleans up temporary test files and directories

# Default settings
REMOVE_ALL=false
REMOVE_TEMP_ONLY=false
REMOVE_COVERAGE_ONLY=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
  key="$1"
  case $key in
    --all)
    REMOVE_ALL=true
    shift
    ;;
    --temp-only)
    REMOVE_TEMP_ONLY=true
    shift
    ;;
    --coverage-only)
    REMOVE_COVERAGE_ONLY=true
    shift
    ;;
    *)
    # unknown option
    shift
    ;;
  esac
done

# Get the location of this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEMP_BOOST_DIR="$SCRIPT_DIR/temp_boost_test"
TEST_RESULTS_DIR="$SCRIPT_DIR/test-results"

echo -e "\033[0;36mCleaning up test environment...\033[0m"

# Remove temporary files (compilation artifacts, etc.) but keep the main structure
remove_temp_files() {
    # Clean temp_boost_test temporary files
    if [ -d "$TEMP_BOOST_DIR" ]; then
        echo -e "\033[0;33mCleaning temporary files in $TEMP_BOOST_DIR...\033[0m"
        rm -f "$TEMP_BOOST_DIR"/*.o 2>/dev/null
        rm -f "$TEMP_BOOST_DIR"/*.out 2>/dev/null
        rm -f "$TEMP_BOOST_DIR"/*.gcno 2>/dev/null
        rm -f "$TEMP_BOOST_DIR"/*.gcda 2>/dev/null
        
        # Clean build directory
        if [ -d "$TEMP_BOOST_DIR/build" ]; then
            find "$TEMP_BOOST_DIR/build" -name "*.o" -delete 2>/dev/null
            find "$TEMP_BOOST_DIR/build" -name "*.a" -delete 2>/dev/null
            find "$TEMP_BOOST_DIR/build" -name "*.so" -delete 2>/dev/null
            find "$TEMP_BOOST_DIR/build" -name "CMakeCache.txt" -delete 2>/dev/null
            rm -rf "$TEMP_BOOST_DIR/build/CMakeFiles" 2>/dev/null
        fi
    fi
}

# Remove coverage reports
remove_coverage_reports() {
    # Remove coverage reports in temp_boost_test
    if [ -d "$TEMP_BOOST_DIR/build/coverage_report" ]; then
        echo -e "\033[0;33mRemoving coverage reports in $TEMP_BOOST_DIR/build/coverage_report...\033[0m"
        rm -rf "$TEMP_BOOST_DIR/build/coverage_report" 2>/dev/null
    fi
    
    # Remove coverage XML files
    rm -f "$TEMP_BOOST_DIR"/*.xml 2>/dev/null
    
    # Remove coverage reports in test-results
    if [ -d "$TEST_RESULTS_DIR" ]; then
        find "$TEST_RESULTS_DIR" -type d -name "coverage" -exec rm -rf {} \; 2>/dev/null
    fi
}

# Process based on parameters
if [ "$REMOVE_COVERAGE_ONLY" = true ]; then
    remove_coverage_reports
elif [ "$REMOVE_TEMP_ONLY" = true ]; then
    remove_temp_files
elif [ "$REMOVE_ALL" = true ]; then
    # Remove all test directories and files
    if [ -d "$TEMP_BOOST_DIR" ]; then
        echo -e "\033[0;33mRemoving $TEMP_BOOST_DIR...\033[0m"
        rm -rf "$TEMP_BOOST_DIR" 2>/dev/null
    fi
    
    if [ -d "$TEST_RESULTS_DIR" ]; then
        echo -e "\033[0;33mRemoving $TEST_RESULTS_DIR...\033[0m"
        rm -rf "$TEST_RESULTS_DIR" 2>/dev/null
    fi
else
    # Default behavior: clean temporary files and coverage reports
    remove_temp_files
    remove_coverage_reports
fi

echo -e "\033[0;32mCleanup completed!\033[0m"
