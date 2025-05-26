# ci-test-integration.ps1
# This script runs all tests and formats the results for CI/CD integration
# It generates JUnit-compatible test reports for Windows environments

param (
    [switch]$IncludeBoostTests = $true,  # Run Boost tests
    [switch]$IncludeMainTests = $true,   # Run Main app tests
    [switch]$Coverage = $true,           # Generate coverage reports
    [string]$OutputDir = "$PSScriptRoot\test-results"  # Output directory for test results
)

# Create output directory if it doesn't exist
if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir | Out-Null
}

$allTestsPassed = $true
$testSummary = @()

# Run Boost tests if specified
if ($IncludeBoostTests) {
    Write-Host "Running Boost tests..." -ForegroundColor Cyan
    
    # Create Boost test results directory
    $boostResultsDir = Join-Path $OutputDir "boost-tests"
    if (-not (Test-Path $boostResultsDir)) {
        New-Item -ItemType Directory -Path $boostResultsDir | Out-Null
    }
    
    # Run Boost tests with XML output
    $boostTestArgs = @(
        "-BoostTests",
        "-TestFilter", "" # Empty to run all tests
    )
    
    if ($Coverage) {
        $boostTestArgs += "-Coverage"
    }
    
    # Run the Boost tests using the main test runner
    & "$PSScriptRoot\run-tests-windows.ps1" @boostTestArgs
    
    # Check if tests passed
    if ($LASTEXITCODE -eq 0) {
        $testSummary += "Boost tests: Passed"
    } else {
        $testSummary += "Boost tests: Failed with exit code $LASTEXITCODE"
        $allTestsPassed = $false
    }
    
    # Copy coverage report to output directory if it exists
    $coverageDir = "$PSScriptRoot\temp_boost_test\coverage_report"
    if (Test-Path $coverageDir) {
        $coverageOutputDir = Join-Path $boostResultsDir "coverage"
        if (-not (Test-Path $coverageOutputDir)) {
            New-Item -ItemType Directory -Path $coverageOutputDir | Out-Null
        }
        
        # Copy the coverage report
        Copy-Item "$coverageDir\*" -Destination $coverageOutputDir -Recurse -Force
        $testSummary += "Boost test coverage report generated at: $coverageOutputDir"
    }
}

# Run main application tests if specified
if ($IncludeMainTests) {
    Write-Host "Running main application tests..." -ForegroundColor Cyan
    
    # Create main test results directory
    $mainResultsDir = Join-Path $OutputDir "main-tests"
    if (-not (Test-Path $mainResultsDir)) {
        New-Item -ItemType Directory -Path $mainResultsDir | Out-Null
    }
    
    # Run main application tests
    & "$PSScriptRoot\run-tests-windows.ps1"
    
    # Check if tests passed
    if ($LASTEXITCODE -eq 0) {
        $testSummary += "Main application tests: Passed"
    } else {
        $testSummary += "Main application tests: Failed with exit code $LASTEXITCODE"
        $allTestsPassed = $false
    }
}

# Write test summary
Write-Host "`nTest Summary:" -ForegroundColor Cyan
foreach ($result in $testSummary) {
    if ($result -like "*Passed*") {
        Write-Host "✅ $result" -ForegroundColor Green
    } else {
        Write-Host "❌ $result" -ForegroundColor Red
    }
}

# Create summary file for CI systems
$summaryFile = Join-Path $OutputDir "test-summary.md"
Set-Content -Path $summaryFile -Value "# PiTrac Test Results`n"
Add-Content -Path $summaryFile -Value "## Summary`n"

foreach ($result in $testSummary) {
    if ($result -like "*Passed*") {
        Add-Content -Path $summaryFile -Value "- ✅ $result"
    } else {
        Add-Content -Path $summaryFile -Value "- ❌ $result"
    }
}

# Set the CI exit code based on the test results
if ($allTestsPassed) {
    Write-Host "`nAll tests passed! 🎉" -ForegroundColor Green
    exit 0
} else {
    Write-Host "`nSome tests failed. 😢" -ForegroundColor Red
    exit 1
}
