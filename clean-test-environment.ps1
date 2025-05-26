# clean-test-environment.ps1
# This script cleans up temporary test files and directories

param (
    [switch]$RemoveAll,         # Remove all test files including the main test directory
    [switch]$RemoveTempOnly,    # Remove only temporary files but keep the main test directory
    [switch]$RemoveCoverageOnly # Remove only coverage reports
)

# Get the location of this script
$scriptDir = $PSScriptRoot
$testBuildDir = Join-Path $scriptDir "boost_test_build"
$tempBoostDir = Join-Path $scriptDir "temp_boost_test"
$testResultsDir = Join-Path $scriptDir "test-results"

Write-Host "Cleaning up test environment..." -ForegroundColor Cyan

# Remove temporary files (compilation artifacts, etc.) but keep the main structure
function RemoveTempFiles {
    # Clean temp_boost_test temporary files
    if (Test-Path $tempBoostDir) {
        Write-Host "Cleaning temporary files in $tempBoostDir..." -ForegroundColor Yellow
        Remove-Item -Path "$tempBoostDir\*.exe" -Force -ErrorAction SilentlyContinue
        Remove-Item -Path "$tempBoostDir\*.obj" -Force -ErrorAction SilentlyContinue
        Remove-Item -Path "$tempBoostDir\*.pdb" -Force -ErrorAction SilentlyContinue
        Remove-Item -Path "$tempBoostDir\*.ilk" -Force -ErrorAction SilentlyContinue
    }
    
    # Clean boost_test_build temporary files
    if (Test-Path $testBuildDir) {
        Write-Host "Cleaning temporary files in $testBuildDir..." -ForegroundColor Yellow
        Remove-Item -Path "$testBuildDir\*.exe" -Force -ErrorAction SilentlyContinue
        Remove-Item -Path "$testBuildDir\*.obj" -Force -ErrorAction SilentlyContinue
        Remove-Item -Path "$testBuildDir\*.pdb" -Force -ErrorAction SilentlyContinue
        Remove-Item -Path "$testBuildDir\*.ilk" -Force -ErrorAction SilentlyContinue
        Remove-Item -Path "$testBuildDir\*.sln" -Force -ErrorAction SilentlyContinue
        Remove-Item -Path "$testBuildDir\*.vcxproj*" -Force -ErrorAction SilentlyContinue
    }
}

# Remove coverage reports
function RemoveCoverageReports {
    # Remove coverage reports in temp_boost_test
    if (Test-Path "$tempBoostDir\coverage_report") {
        Write-Host "Removing coverage reports in $tempBoostDir\coverage_report..." -ForegroundColor Yellow
        Remove-Item -Path "$tempBoostDir\coverage_report" -Recurse -Force
    }
    
    # Remove coverage XML files
    Remove-Item -Path "$tempBoostDir\*.xml" -Filter "coverage*.xml" -Force -ErrorAction SilentlyContinue
    
    # Remove coverage reports in boost_test_build
    if (Test-Path "$testBuildDir\coverage_report") {
        Write-Host "Removing coverage reports in $testBuildDir\coverage_report..." -ForegroundColor Yellow
        Remove-Item -Path "$testBuildDir\coverage_report" -Recurse -Force
    }
    
    # Remove coverage reports in test-results
    if (Test-Path $testResultsDir) {
        $coverageDirs = Get-ChildItem -Path $testResultsDir -Filter "coverage" -Directory -Recurse
        foreach ($dir in $coverageDirs) {
            Write-Host "Removing coverage reports in $($dir.FullName)..." -ForegroundColor Yellow
            Remove-Item -Path $dir.FullName -Recurse -Force
        }
    }
}

# Process based on parameters
if ($RemoveCoverageOnly) {
    RemoveCoverageReports
} elseif ($RemoveTempOnly) {
    RemoveTempFiles
} elseif ($RemoveAll) {
    # Remove all test directories and files
    if (Test-Path $tempBoostDir) {
        Write-Host "Removing $tempBoostDir..." -ForegroundColor Yellow
        Remove-Item -Path $tempBoostDir -Recurse -Force
    }
    
    if (Test-Path $testBuildDir) {
        Write-Host "Removing $testBuildDir..." -ForegroundColor Yellow
        Remove-Item -Path $testBuildDir -Recurse -Force
    }
    
    if (Test-Path $testResultsDir) {
        Write-Host "Removing $testResultsDir..." -ForegroundColor Yellow
        Remove-Item -Path $testResultsDir -Recurse -Force
    }
} else {
    # Default behavior: clean temporary files and coverage reports
    RemoveTempFiles
    RemoveCoverageReports
}

Write-Host "Cleanup completed!" -ForegroundColor Green
