# run-tests-windows.ps1
# Runs the PiTrac automated tests in a separate process
# This ensures test isolation and proper environment setup

param (
    [switch]$BoostTests,    # Run Boost Test framework instead of main app tests
    [switch]$Verbose,       # Enable verbose logging
    [switch]$Coverage,      # Run tests with code coverage enabled
    [string]$TestFilter=""  # Filter specific tests (for Boost test framework only)
)

# Get the location of this script
$scriptDir = $PSScriptRoot
$exePath = Join-Path $scriptDir "Software\LMSourceCode\x64\Debug\ImageProcessing.exe"

# If boost tests specified, run those instead
if ($BoostTests) {
    Write-Host "Running Boost unit tests..." -ForegroundColor Cyan
    
    $boostTestDir = Join-Path $scriptDir "temp_boost_test"
    $compileCmd = Join-Path $boostTestDir "compile.cmd"
    
    # Check if we need to set up the Boost test environment
    if (-not (Test-Path $compileCmd)) {
        Write-Host "Setting up Boost test environment..." -ForegroundColor Yellow
        & "$scriptDir\configure-boost-tests-windows.ps1"
    }
      # Run the tests
    Push-Location $boostTestDir
    try {
        $testParams = @()
        if ($Verbose) { $testParams += "--log_level=all" }
        if ($TestFilter) { $testParams += "--run_test=$TestFilter" }        $argString = $testParams -join " "
        
        if ($Coverage) {
            if (-not (Test-Path "$boostTestDir\run-with-coverage.cmd")) {
                Write-Host "Setting up code coverage..." -ForegroundColor Yellow
                & "$scriptDir\setup-code-coverage.ps1" -Install
            }
            & cmd /c "run-with-coverage.cmd $argString"
        } else {
            & cmd /c "compile.cmd $argString"
        }
        exit $LASTEXITCODE
    } finally {
        Pop-Location
    }
}

# Otherwise, run the main application tests
# Verify the executable exists
if (-not (Test-Path $exePath)) {
    Write-Host "Error: Executable not found at $exePath" -ForegroundColor Red
    Write-Host "Please build the solution first using build-and-run-windows.ps1" -ForegroundColor Yellow
    exit 1
}

# Set up environment variables for test run
$env:OPENCV_DIR = [Environment]::GetEnvironmentVariable("OPENCV_DIR", "Machine")
$env:BOOST_ROOT = [Environment]::GetEnvironmentVariable("BOOST_ROOT", "Machine")

# Add required paths to PATH for runtime DLL resolution
$pathAdditions = @(
    "$scriptDir\Software\LMSourceCode\ImageProcessing\bin",
    "$env:OPENCV_DIR\x64\vc16\bin", 
    "$env:BOOST_ROOT\lib64-msvc-14.3"
)

# Update the current session path to ensure DLLs are found
foreach ($pathItem in $pathAdditions) {
    if ($env:Path -notlike "*$pathItem*") {
        $env:Path += ";$pathItem"
    }
}

# Change to the directory where the executable is
$exeDir = Split-Path $exePath
Push-Location $exeDir
try {
    Write-Host "Running PiTrac automated tests..." -ForegroundColor Cyan
      # Run the test executable with the automated_testing system_mode
    $testParams = @(
        "--logging_level", "info",
        "--artifact_save_level", "all",
        "--system_mode", "automated_testing",
        "--test_overrides", "1"  # Force test mode even if not debug build
    )
      # Execute the tests
    & $exePath @testParams
    
    # Check the exit code
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Tests completed successfully!" -ForegroundColor Green
    } else {
        Write-Host "Tests failed with exit code $LASTEXITCODE" -ForegroundColor Red
    }
} finally {
    Pop-Location
}

exit $LASTEXITCODE
