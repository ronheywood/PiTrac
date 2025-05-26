# configure-boost-tests-windows.ps1
# Sets up the boost test environment for Windows

# Get script location
$scriptDir = $PSScriptRoot
$sourceDir = Join-Path $scriptDir "Software\LMSourceCode\ImageProcessing"
$testDir = Join-Path $scriptDir "temp_boost_test"
$testExe = Join-Path $testDir "simple_tests.exe"

# Make sure test directory exists
if (-not (Test-Path $testDir)) {
    New-Item -ItemType Directory -Path $testDir | Out-Null
}

# Copy test files
Write-Host "Copying test files..." -ForegroundColor Cyan
Copy-Item "$sourceDir\boost_tests.cpp" "$testDir\" -Force
Copy-Item "$sourceDir\gs_automated_testing.cpp" "$testDir\" -Force
Copy-Item "$sourceDir\gs_automated_testing.h" "$testDir\" -Force
Copy-Item "$sourceDir\gs_config.cpp" "$testDir\" -Force
Copy-Item "$sourceDir\gs_config.h" "$testDir\" -Force

# Create test_helpers.h if it doesn't exist
if (-not (Test-Path "$testDir\test_helpers.h")) {
    Write-Host "Creating test_helpers.h..." -ForegroundColor Cyan
    @"
#pragma once
#include <opencv2/core.hpp>

// A simplified header just for testing the Boost Test framework
class TestHelpers {
public:
    // Test if the absolute differences between expected and result are within tolerances
    static bool WithinTolerance(const cv::Vec2d& expected, const cv::Vec2d& result, const cv::Vec2d& abs_tolerances) {
        return 
            std::abs(expected[0] - result[0]) <= abs_tolerances[0] &&
            std::abs(expected[1] - result[1]) <= abs_tolerances[1];
    }

    static bool WithinTolerance(const cv::Vec3d& expected, const cv::Vec3d& result, const cv::Vec3d& abs_tolerances) {
        return 
            std::abs(expected[0] - result[0]) <= abs_tolerances[0] &&
            std::abs(expected[1] - result[1]) <= abs_tolerances[1] &&
            std::abs(expected[2] - result[2]) <= abs_tolerances[2];
    }

    static bool WithinTolerance(float expected, float result, float abs_tolerance) {
        return std::abs(expected - result) <= abs_tolerance;
    }

    static bool WithinTolerance(int expected, int result, int abs_tolerance) {
        return std::abs(expected - result) <= abs_tolerance;
    }
};
"@ | Out-File -FilePath "$testDir\test_helpers.h" -Encoding utf8
}

# Create simple_tests.cpp if it doesn't exist
if (-not (Test-Path "$testDir\simple_tests.cpp")) {
    Write-Host "Creating simple_tests.cpp..." -ForegroundColor Cyan
    @"
#define BOOST_TEST_MODULE PiTracTests
#include <boost/test/included/unit_test.hpp>
#include "test_helpers.h"
#include <iostream>

// Basic test case to verify Boost Test framework is working
BOOST_AUTO_TEST_CASE(SampleTest)
{
    std::cout << "Running sample test..." << std::endl;
    BOOST_CHECK_EQUAL(1 + 1, 2);
    BOOST_TEST_MESSAGE("Sample test passed");
}

// Tests for the TestHelpers class
BOOST_AUTO_TEST_CASE(HelperTests)
{
    std::cout << "Running TestHelpers tests..." << std::endl;
    
    // Test Vec2d version
    cv::Vec2d expected2d(10.0, 20.0);
    cv::Vec2d result2d(11.0, 19.5);
    cv::Vec2d tolerance2d(2.0, 1.0);
    BOOST_CHECK(TestHelpers::WithinTolerance(expected2d, result2d, tolerance2d));
    
    // Test Vec2d version with failing case
    cv::Vec2d result2d_fail(13.0, 20.0);
    BOOST_CHECK(!TestHelpers::WithinTolerance(expected2d, result2d_fail, tolerance2d));
    
    // Test Vec3d version
    cv::Vec3d expected3d(10.0, 20.0, 30.0);
    cv::Vec3d result3d(11.0, 19.5, 29.0);
    cv::Vec3d tolerance3d(2.0, 1.0, 1.5);
    BOOST_CHECK(TestHelpers::WithinTolerance(expected3d, result3d, tolerance3d));
    
    // Test float version
    BOOST_CHECK(TestHelpers::WithinTolerance(10.0f, 10.5f, 1.0f));
    BOOST_CHECK(!TestHelpers::WithinTolerance(10.0f, 12.0f, 1.0f));
    
    // Test int version
    BOOST_CHECK(TestHelpers::WithinTolerance(100, 101, 2));
    BOOST_CHECK(!TestHelpers::WithinTolerance(100, 103, 2));
}
"@ | Out-File -FilePath "$testDir\simple_tests.cpp" -Encoding utf8
}

# Create compile.cmd file
Write-Host "Creating compile.cmd..." -ForegroundColor Cyan
$vcvarsall = "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat"

# If not found, try other common locations
if (-not (Test-Path $vcvarsall)) {
    $vcvarsall = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
}

if (-not (Test-Path $vcvarsall)) {
    $vcvarsall = "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat"
}

if (-not (Test-Path $vcvarsall)) {
    $vcvarsall = "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat"
}

if (-not (Test-Path $vcvarsall)) {
    Write-Host "Could not find vcvarsall.bat. Please ensure Visual Studio with C++ tools is installed." -ForegroundColor Red
    exit 1
}

$opencvDir = [Environment]::GetEnvironmentVariable("OPENCV_DIR", "Machine")
$boostRoot = [Environment]::GetEnvironmentVariable("BOOST_ROOT", "Machine")

@"
@echo off
setlocal

REM Setup VC environment for x64
call "$vcvarsall" x64

REM Set paths
set OPENCV_DIR=$opencvDir
set BOOST_ROOT=$boostRoot

REM Compile the test
echo Compiling test...
dir "%OPENCV_DIR%\x64\vc16\lib\*.lib"
cl.exe /EHsc /std:c++17 /I"%OPENCV_DIR%\include" /I"%BOOST_ROOT%" simple_tests.cpp /link /LIBPATH:"%OPENCV_DIR%\x64\vc16\lib" opencv_world4110.lib /out:simple_tests.exe

REM Run if successful
if %ERRORLEVEL% NEQ 0 (
    echo Compilation failed with error code %ERRORLEVEL%
    exit /b %ERRORLEVEL%
)

echo Running tests...
simple_tests.exe --log_level=test_suite

if %ERRORLEVEL% NEQ 0 (
    echo Tests failed with error code %ERRORLEVEL%
) else (
    echo Tests passed!
)

exit /b %ERRORLEVEL%
"@ | Out-File -FilePath "$testDir\compile.cmd" -Encoding ascii

Write-Host "Boost test environment configured for Windows." -ForegroundColor Green
Write-Host "To build and run tests, execute:" -ForegroundColor White
Write-Host "cd $testDir && .\compile.cmd" -ForegroundColor Cyan
