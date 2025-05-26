# setup-code-coverage.ps1
# This script sets up code coverage tools for the PiTrac Boost tests
# It installs OpenCppCoverage if not already installed and configures the project

param (
    [switch]$Install  # Install OpenCppCoverage if not present
)

# Get the location of this script
$scriptDir = $PSScriptRoot
Write-Host "Setting up code coverage for PiTrac tests..." -ForegroundColor Cyan

# Check if OpenCppCoverage is installed
$opencppcoverage = Get-Command "OpenCppCoverage.exe" -ErrorAction SilentlyContinue

if (-not $opencppcoverage) {
    Write-Host "OpenCppCoverage not found in path" -ForegroundColor Yellow
    
    if ($Install) {
        Write-Host "Installing OpenCppCoverage..." -ForegroundColor Cyan
        
        # Use Chocolatey to install OpenCppCoverage
        if (-not (Get-Command "choco.exe" -ErrorAction SilentlyContinue)) {
            Write-Host "Chocolatey not installed. Please install Chocolatey first or install OpenCppCoverage manually." -ForegroundColor Red
            Write-Host "Visit https://github.com/OpenCppCoverage/OpenCppCoverage/releases to download manually." -ForegroundColor Yellow
            exit 1
        }
        
        # Install OpenCppCoverage
        choco install opencppcoverage -y
        
        # Refresh path
        $env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")
        
        # Check if installation was successful
        $opencppcoverage = Get-Command "OpenCppCoverage.exe" -ErrorAction SilentlyContinue
        if (-not $opencppcoverage) {
            Write-Host "Failed to install OpenCppCoverage. Please install manually." -ForegroundColor Red
            exit 1
        }
    } else {
        Write-Host "Please install OpenCppCoverage or run this script with -Install switch." -ForegroundColor Yellow
        Write-Host "Visit https://github.com/OpenCppCoverage/OpenCppCoverage/releases to download manually." -ForegroundColor Yellow
        exit 1
    }
}

# Create a coverage script for the Boost tests
$coverageScriptPath = Join-Path $scriptDir "temp_boost_test\run-with-coverage.cmd"
$coverageScriptContent = @"
@echo off
setlocal

REM Get OpenCppCoverage path
for /f "tokens=*" %%i in ('where OpenCppCoverage.exe') do set OPENCPPCOVERAGE=%%i

REM Parse command line arguments
set LOG_LEVEL=--log_level=test_suite
set TEST_FILTER=
:parse_args
if "%~1"=="" goto end_parse_args
if "%~1"=="--log_level=all" set LOG_LEVEL=--log_level=all
if "%~1:~0,11%"=="--run_test=" set TEST_FILTER=%~1
shift
goto parse_args
:end_parse_args

REM Run the compiled tests with coverage
echo Running tests with coverage...
"%OPENCPPCOVERAGE%" --sources="%~dp0..\..\Software\LMSourceCode\ImageProcessing" --excluded_sources="%~dp0..\..\Software\LMSourceCode\ImageProcessing\boost_tests*.cpp" --export_type=html:%~dp0coverage_report --export_type=cobertura:%~dp0coverage.xml --working_dir="%~dp0" -- "%~dp0simple_tests.exe" %LOG_LEVEL% %TEST_FILTER%

if %ERRORLEVEL% NEQ 0 (
    echo Tests failed with error code %ERRORLEVEL%
) else (
    echo Tests passed!
    echo Coverage report generated at %~dp0coverage_report\index.html
)

exit /b %ERRORLEVEL%
"@

# Write coverage script
Write-Host "Creating coverage script at $coverageScriptPath" -ForegroundColor Cyan
Set-Content -Path $coverageScriptPath -Value $coverageScriptContent

# Update the Windows test runner to support code coverage
$runTestsPath = Join-Path $scriptDir "run-tests-windows.ps1"
if (Test-Path $runTestsPath) {
    Write-Host "Updating test runner to support code coverage..." -ForegroundColor Cyan
    $testRunnerContent = Get-Content $runTestsPath -Raw
    
    if (-not $testRunnerContent.Contains("-Coverage")) {
        $updatedContent = $testRunnerContent -replace "param \(", "param (`r`n    [switch]`$Coverage,    # Run tests with code coverage enabled`r`n"
        $updatedContent = $updatedContent -replace "& cmd /c `"compile.cmd `$argString`"", @"
if (`$Coverage) {
            & cmd /c "run-with-coverage.cmd `$argString"
        } else {
            & cmd /c "compile.cmd `$argString"
        }
"@
        
        Set-Content -Path $runTestsPath -Value $updatedContent
    }
}

Write-Host "Code coverage setup completed. You can now run tests with coverage using:" -ForegroundColor Green
Write-Host ".\run-tests-windows.ps1 -BoostTests -Coverage" -ForegroundColor Green
