# run-boost-tests-windows.ps1
# Builds and runs the Boost Test framework tests for PiTrac in Windows using CMake

param (
    [switch]$Verbose,        # Enable verbose logging
    [switch]$Coverage,        # Run tests with code coverage
    [string]$TestFilter="",  # Filter specific tests
    [switch]$Enhanced         # Use the enhanced boost tests file
)

# Get the location of this script
$scriptDir = $PSScriptRoot
$projectDir = Join-Path $scriptDir "Software\LMSourceCode\ImageProcessing"
$testBuildDir = Join-Path $scriptDir "boost_test_build"
$boostTestExe = Join-Path $testBuildDir "Debug\boost_tests.exe"

# Set up environment variables 
$env:OPENCV_DIR = [Environment]::GetEnvironmentVariable("OPENCV_DIR", "Machine")
$env:BOOST_ROOT = [Environment]::GetEnvironmentVariable("BOOST_ROOT", "Machine")

Write-Host "OPENCV_DIR: $env:OPENCV_DIR" -ForegroundColor Cyan
Write-Host "BOOST_ROOT: $env:BOOST_ROOT" -ForegroundColor Cyan

# Add required paths to PATH for runtime DLL resolution
$pathAdditions = @(
    "$scriptDir\Software\LMSourceCode\ImageProcessing\bin",
    "$env:OPENCV_DIR\x64\vc16\bin", 
    "$env:BOOST_ROOT\lib64-msvc-14.3"
)

# Update the current session path
foreach ($pathItem in $pathAdditions) {
    if ($env:Path -notlike "*$pathItem*" -and (Test-Path $pathItem)) {
        $env:Path += ";$pathItem"
        Write-Host "Added to PATH: $pathItem" -ForegroundColor DarkGray
    }
}

# Create test build directory if needed
if (-not (Test-Path $testBuildDir)) {
    New-Item -ItemType Directory -Path $testBuildDir | Out-Null
}

# Copy the test files and CMake configuration
Copy-Item "$projectDir\CMakeLists-tests.txt" "$testBuildDir\CMakeLists.txt" -Force
Copy-Item "$projectDir\gs_automated_testing.cpp" "$testBuildDir" -Force
Copy-Item "$projectDir\gs_automated_testing.h" "$testBuildDir" -Force
Copy-Item "$projectDir\gs_config.cpp" "$testBuildDir" -Force
Copy-Item "$projectDir\gs_config.h" "$testBuildDir" -Force

# Use the enhanced boost tests file if specified
if ($Enhanced -and (Test-Path "$projectDir\boost_tests_enhanced.cpp")) {
    Copy-Item "$projectDir\boost_tests_enhanced.cpp" "$testBuildDir\boost_tests.cpp" -Force
    Write-Host "Using enhanced Boost tests file" -ForegroundColor Cyan
} else {
    Copy-Item "$projectDir\boost_tests.cpp" "$testBuildDir" -Force
}

# Copy any other necessary dependencies
$additionalFiles = @(
    "gs_camera.h",
    "gs_results.h"
)

foreach ($file in $additionalFiles) {
    if (Test-Path "$projectDir\$file") {
        Copy-Item "$projectDir\$file" "$testBuildDir" -Force
    }
}

# Configure with CMake
Push-Location $testBuildDir
try {
    Write-Host "Configuring tests with CMake..." -ForegroundColor Cyan
    
    # Run CMake configure step
    & cmake -G "Visual Studio 17 2022" -A x64 .
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "CMake configure failed with exit code $LASTEXITCODE" -ForegroundColor Red
        exit $LASTEXITCODE
    }
    
    # Build the tests
    Write-Host "Building tests..." -ForegroundColor Cyan
    & cmake --build . --config Debug
    
    if ($LASTEXITCODE -ne 0 -or -not (Test-Path $boostTestExe)) {
        Write-Host "Test build failed with exit code $LASTEXITCODE" -ForegroundColor Red
        exit $LASTEXITCODE
    }
      # Set up test parameters
    $testParams = @()
    
    if ($Verbose) {
        $testParams += "--log_level=all"
    } else {
        $testParams += "--log_level=test_suite" 
    }
    
    if ($TestFilter) {
        $testParams += "--run_test=$TestFilter"
    }
    
    # Run the tests
    Write-Host "Running Boost tests..." -ForegroundColor Cyan
    
    if ($Coverage) {
        # Check if OpenCppCoverage is installed
        $opencppcoverage = Get-Command "OpenCppCoverage.exe" -ErrorAction SilentlyContinue
        
        if (-not $opencppcoverage) {
            Write-Host "OpenCppCoverage not found. Running setup-code-coverage.ps1..." -ForegroundColor Yellow
            & "$scriptDir\setup-code-coverage.ps1" -Install
            $opencppcoverage = Get-Command "OpenCppCoverage.exe" -ErrorAction SilentlyContinue
        }
        
        if ($opencppcoverage) {
            # Create coverage directory if it doesn't exist
            $coverageDir = Join-Path $testBuildDir "coverage_report"
            if (-not (Test-Path $coverageDir)) {
                New-Item -ItemType Directory -Path $coverageDir | Out-Null
            }
            
            # Run with code coverage
            Write-Host "Running tests with code coverage..." -ForegroundColor Cyan
            & OpenCppCoverage.exe --sources="$projectDir" --excluded_sources="$projectDir\boost_tests*.cpp" --export_type=html:"$coverageDir" -- $boostTestExe $testParams
            
            if ($LASTEXITCODE -eq 0) {
                Write-Host "Tests completed successfully!" -ForegroundColor Green
                Write-Host "Coverage report generated at: $coverageDir\index.html" -ForegroundColor Cyan
            } else {
                Write-Host "Tests failed with exit code $LASTEXITCODE" -ForegroundColor Red
            }
        } else {
            # Fall back to running without coverage
            Write-Host "OpenCppCoverage not available. Running tests without coverage." -ForegroundColor Yellow
            & $boostTestExe $testParams
        }
    } else {
        # Run without code coverage
        & $boostTestExe $testParams
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "Tests completed successfully!" -ForegroundColor Green
        } else {
            Write-Host "Tests failed with exit code $LASTEXITCODE" -ForegroundColor Red
        }
    }
} finally {
    Pop-Location
}

exit $LASTEXITCODE
