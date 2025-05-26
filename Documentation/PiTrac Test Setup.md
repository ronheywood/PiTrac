# PiTrac Testing Implementation

This document describes the testing implementation for the PiTrac project.

## Overview

We've implemented two testing approaches for the PiTrac codebase:

1. **Main Application Testing**: Using the `--system_mode automated_testing` flag with test data
2. **Boost Test Framework**: Unit tests for specific functions independent of the main application

## Main Application Testing

The main application testing works by:

1. Setting the application to automated_testing mode
2. Loading pre-captured images from a test dataset
3. Running the ball detection and analysis algorithms
4. Comparing results with expected values

### Running Main Application Tests on Windows

```powershell
# Run the tests
./run-tests-windows.ps1
```

### Running Main Application Tests on Linux

```bash
# Run the tests
./run-tests-linux.sh
```

## Boost Test Framework Setup

We've also implemented a Boost Test framework integration for testing individual functions separately from the main application.

### Windows Setup

The Windows test environment is configured using PowerShell scripts:

1. `configure-boost-tests-windows.ps1`: Sets up the test environment and creates test files
2. `temp_boost_test\compile.cmd`: Compiles and runs the tests using Visual Studio compiler

#### Running Boost Tests on Windows

```powershell
# Run the tests using the test runner script
./run-tests-windows.ps1 -BoostTests

# Filter specific tests
./run-tests-windows.ps1 -BoostTests -TestFilter="SampleTest"

# Run with verbose logging
./run-tests-windows.ps1 -BoostTests -Verbose

# Run with code coverage analysis
./run-tests-windows.ps1 -BoostTests -Coverage

# Then run the tests
cd temp_boost_test
./compile.cmd
```

### Linux Setup

The Linux test environment is configured using Bash scripts:

1. `configure-boost-tests-linux.sh`: Sets up the test environment and creates test files
2. `temp_boost_test/build-and-run.sh`: Builds tests with CMake and runs them

#### Running Boost Tests on Linux

```bash
# Run the tests using the test runner script
./run-tests-linux.sh --boost-tests

# Filter specific tests
./run-tests-linux.sh --boost-tests --test-filter=SampleTest

# Run with verbose logging
./run-tests-linux.sh --boost-tests --verbose

# Run with code coverage analysis
./run-tests-linux.sh --boost-tests --coverage
```

## Current Boost Tests

- `SampleTest`: A simple test that verifies the Boost Test framework is working
- `AbsResultsPassTest`: Tests for the GsAutomatedTesting utility functions that verify tolerance calculations
- `HelperTests`: Tests for the TestHelpers utility functions that verify tolerance calculations 
- `RelativeErrorTest`: Tests for relative error calculations
- `VectorOperationsTest`: Tests for vector math operations
- `ImageThresholdingTest`: Tests for basic image processing operations
- `ConfigFileTest`: Tests for configuration file loading
- `GeometryCalculationTest`: Tests for geometry calculations
- `SimplifiedPipelineTest`: Tests for a simplified version of the main processing pipeline

## Adding New Boost Tests

To add new tests, create test cases in the `temp_boost_test/simple_tests.cpp` file using the Boost Test macros:

```cpp
BOOST_AUTO_TEST_CASE(MyNewTest)
{
    // Test code here
    BOOST_CHECK_EQUAL(expected, actual);
}
```

## Environment Requirements

For all test environments, ensure:

1. Environment variables are correctly set up (OPENCV_DIR, BOOST_ROOT)
2. Dependencies are installed (OpenCV, Boost)
3. Test data images are available in the correct locations

## Code Coverage Analysis

Code coverage analysis is enabled for both Windows and Linux environments:

### Windows Code Coverage

Windows code coverage uses OpenCppCoverage and is automatically set up when you run:

```powershell
./run-tests-windows.ps1 -BoostTests -Coverage
```

The coverage report will be generated in the `temp_boost_test\coverage_report` directory.

### Linux Code Coverage

Linux code coverage uses gcovr and can be run with:

```bash
./run-tests-linux.sh --boost-tests --coverage
```

The coverage report will be generated in the `temp_boost_test/build/coverage_report` directory.

## Continuous Integration

### Adding Tests to CI Pipeline

To add the Boost tests to your CI pipeline, add the following steps to your workflow:

```yaml
- name: Run Unit Tests (Windows)
  run: ./run-tests-windows.ps1 -BoostTests -Coverage

- name: Run Unit Tests (Linux) 
  run: ./run-tests-linux.sh --boost-tests --coverage

- name: Upload Coverage Reports
  uses: actions/upload-artifact@v2
  with:
    name: code-coverage-report
    path: |
      temp_boost_test/coverage_report
      temp_boost_test/build/coverage_report
```

## Maintaining the Test Environment

To keep your workspace clean and organized, we've provided cleanup scripts:

### Windows Cleanup

```powershell
# Clean temporary files and coverage reports
.\clean-test-environment.ps1

# Clean only coverage reports
.\clean-test-environment.ps1 -RemoveCoverageOnly

# Clean only temporary build files
.\clean-test-environment.ps1 -RemoveTempOnly

# Remove all test directories and start fresh
.\clean-test-environment.ps1 -RemoveAll
```

### Linux Cleanup

```bash
# Clean temporary files and coverage reports
./clean-test-environment.sh

# Clean only coverage reports
./clean-test-environment.sh --coverage-only

# Clean only temporary build files
./clean-test-environment.sh --temp-only

# Remove all test directories and start fresh
./clean-test-environment.sh --all
```

## Future Improvements

1. Add more comprehensive tests for specific PiTrac functionality
2. Add mock objects for external dependencies
3. Consider replacing header-only approach with dynamic linking when Windows build issues are resolved
4. Add performance benchmark tests