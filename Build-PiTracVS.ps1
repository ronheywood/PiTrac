# Build-PiTracVS.ps1
# PowerShell script to build PiTrac solution using Visual Studio Developer Command Prompt
# This script uses an alternative approach to build the solution by launching a dedicated command prompt

param (
    [string]$Configuration = "Release",
    [string]$Platform = "x64",
    [switch]$Rebuild = $false
)

# Display header
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host "    PiTrac Visual Studio Build Interface     " -ForegroundColor Cyan
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host ""

# Define paths
$baseDir = Resolve-Path .
$solutionDir = "$baseDir\Software\LMSourceCode"
$solutionFile = "$solutionDir\GolfSim.sln"

# Verify solution exists
if (-not (Test-Path $solutionFile)) {
    Write-Host "Solution file not found at: $solutionFile" -ForegroundColor Red
    exit 1
}

# Visual Studio detection
$vsInstallerPath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vsInstallerPath) {
    Write-Host "Using vswhere.exe to locate Visual Studio..." -ForegroundColor Yellow
    $vsInstallationPath = & $vsInstallerPath -latest -property installationPath 2>$null
    if ($vsInstallationPath) {
        Write-Host "Visual Studio found at: $vsInstallationPath" -ForegroundColor Green
    } else {
        Write-Host "Visual Studio not found by vswhere." -ForegroundColor Yellow
    }
} else {
    Write-Host "vswhere.exe not found, trying common installation paths..." -ForegroundColor Yellow
    $vsInstallationPath = $null
}

# Find Developer Command Prompt
$devPromptPaths = @(
    # VS 2022 paths
    "${env:ProgramFiles}\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat",
    "${env:ProgramFiles}\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat",
    "${env:ProgramFiles}\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat",
    
    # VS 2019 paths
    "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\Enterprise\Common7\Tools\VsDevCmd.bat",
    "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\Professional\Common7\Tools\VsDevCmd.bat",
    "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"
)

# If vsInstallationPath was found, add it to the search paths
if ($vsInstallationPath) {
    $devPromptPaths = @("$vsInstallationPath\Common7\Tools\VsDevCmd.bat") + $devPromptPaths
}

$devPromptPath = $null
foreach ($path in $devPromptPaths) {
    if (Test-Path $path) {
        $devPromptPath = $path
        Write-Host "Developer Command Prompt found at: $devPromptPath" -ForegroundColor Green
        break
    }
}

if (-not $devPromptPath) {
    Write-Host "Visual Studio Developer Command Prompt not found." -ForegroundColor Red
    Write-Host "Please ensure Visual Studio with C++ workload is properly installed." -ForegroundColor Red
    exit 1
}

# Prepare build command
$buildAction = if ($Rebuild) { "Rebuild" } else { "Build" }
$buildCmd = "`"$devPromptPath`" && cd /d `"$baseDir`" && msbuild `"$solutionFile`" /t:$buildAction /p:Configuration=$Configuration /p:Platform=$Platform /m"

# Create a temporary batch file to run the build commands
$tempBatchFile = [System.IO.Path]::GetTempFileName() + ".bat"
Set-Content -Path $tempBatchFile -Value $buildCmd -Encoding ASCII

try {
    Write-Host "Building PiTrac solution ($Configuration|$Platform)..." -ForegroundColor Yellow
    Write-Host "Launching Visual Studio Developer Command Prompt..." -ForegroundColor Yellow
    
    # Run the build using cmd.exe
    $buildProcess = Start-Process -FilePath "cmd.exe" -ArgumentList "/c `"$tempBatchFile`"" -NoNewWindow -PassThru -Wait
    
    if ($buildProcess.ExitCode -eq 0) {
        Write-Host "Build completed successfully" -ForegroundColor Green
        
        # Check for executable
        $executablePath = "$solutionDir\ImageProcessing\output\ImageProcessing.exe"
        if (Test-Path $executablePath) {
            Write-Host "PiTrac executable created at: $executablePath" -ForegroundColor Green
        } else {
            Write-Warning "Expected executable not found at: $executablePath"
            Write-Warning "The build may have succeeded but produced output in a different location."
        }
    } else {
        Write-Host "Build failed with exit code: $($buildProcess.ExitCode)" -ForegroundColor Red
        exit $buildProcess.ExitCode
    }
} catch {
    Write-Host "Error during build: $_" -ForegroundColor Red
    exit 1
} finally {
    # Clean up
    if (Test-Path $tempBatchFile) {
        Remove-Item $tempBatchFile -Force
    }
}

Write-Host ""
Write-Host "To run PiTrac, use the Run-PiTrac.ps1 script:" -ForegroundColor Yellow
Write-Host "  .\Run-PiTrac.ps1" -ForegroundColor White
