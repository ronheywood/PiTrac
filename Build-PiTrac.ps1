# Build-PiTrac.ps1
# PowerShell script to build PiTrac solution on Windows
# This script builds the PiTrac solution by launching a Visual Studio Developer Command Prompt

param (
    [string]$Configuration = "Release",
    [string]$Platform = "x64",
    [switch]$Rebuild = $false,
    [switch]$OpenInVisualStudio = $false,
    [switch]$UseVSDevCmd = $true     # Use Visual Studio Developer Command Prompt
)

# Ensure PSake is installed
if (-not (Get-Module -ListAvailable -Name psake)) {
    Write-Host "Installing PSake PowerShell module..." -ForegroundColor Yellow
    Install-Module -Name psake -Scope CurrentUser -Force -ErrorAction Stop
}

# Import the PSake module
Import-Module psake -ErrorAction Stop

# Display header
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host "          PiTrac Build Interface             " -ForegroundColor Cyan
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host ""

# Check prerequisites are installed by calling the Init task
Invoke-psake -buildFile "$PSScriptRoot\build.psake.ps1" -taskList "Init"
if (-not $psake.build_success) {
    Write-Host "Failed to initialize build environment." -ForegroundColor Red
    exit 1
}

# Define solution file path
$baseDir = Resolve-Path .
$solutionDir = "$baseDir\Software\LMSourceCode"
$solutionFile = "$solutionDir\GolfSim.sln"

# Open in Visual Studio if requested
if ($OpenInVisualStudio) {
    if (Test-Path $solutionFile) {
        Write-Host "Opening solution in Visual Studio..." -ForegroundColor Green
        Start-Process $solutionFile
        exit 0
    } else {
        Write-Host "Solution file not found at: $solutionFile" -ForegroundColor Red
        exit 1
    }
}

# Visual Studio detection
$vsInstallationPath = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath 2>$null
if (-not $vsInstallationPath) {
    $vsInstallationPath = "C:\Program Files\Microsoft Visual Studio\2022\Professional"
}

# Find Visual Studio Developer Command Prompt batch file
$vcvarsallPath = "$vsInstallationPath\VC\Auxiliary\Build\vcvarsall.bat"
if (-not (Test-Path $vcvarsallPath)) {
    # Try alternative paths
    $possiblePaths = @(
        "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat",
        "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat",
        "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat",
        "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat",
        "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat",
        "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat"
    )
    
    foreach ($path in $possiblePaths) {
        if (Test-Path $path) {
            $vcvarsallPath = $path
            break
        }
    }
    
    if (-not (Test-Path $vcvarsallPath)) {
        Write-Host "Visual Studio Developer Command Prompt environment setup script not found." -ForegroundColor Red
        Write-Host "Please ensure Visual Studio with C++ workload is properly installed." -ForegroundColor Red
        exit 1
    }
}

# Initialize the Visual Studio environment by calling vcvarsall.bat
Write-Host "Initializing Visual Studio Developer environment..." -ForegroundColor Yellow
$vcvarsArch = "x64"  # Use x86, x64, arm, arm64 depending on the platform
$tempFile = [System.IO.Path]::GetTempFileName()

# Call vcvarsall.bat and capture the environment
cmd.exe /c "`"$vcvarsallPath`" $vcvarsArch && set > `"$tempFile`""

# Import the environment variables into the current PowerShell session
Get-Content $tempFile | ForEach-Object {
    if ($_ -match "^([^=]+)=(.*)$") {
        $name = $matches[1]
        $value = $matches[2]
        Set-Item -Path "env:$name" -Value $value
    }
}

# Cleanup the temporary file
Remove-Item $tempFile -Force

# Now look for MSBuild in the updated PATH
$msbuildPath = "MSBuild.exe"  # Use the one in the PATH now that environment is set up
try {
    $msbuildPath = (Get-Command MSBuild.exe -ErrorAction Stop).Source
} catch {
    Write-Host "MSBuild not found in the PATH even after setting up the Visual Studio environment." -ForegroundColor Red
    Write-Host "Please ensure Visual Studio with C++ workload is properly installed." -ForegroundColor Red
    exit 1
}

# Verify solution exists
if (-not (Test-Path $solutionFile)) {
    Write-Host "Solution file not found at: $solutionFile" -ForegroundColor Red
    exit 1
}

# Prepare MSBuild arguments
$msbuildArgs = @(
    $solutionFile,
    "/p:Configuration=$Configuration",
    "/p:Platform=$Platform",
    "/m"  # Use multiple cores for faster build
)

if ($Rebuild) {
    $msbuildArgs += "/t:Rebuild"
    Write-Host "Rebuilding PiTrac solution ($Configuration|$Platform)..." -ForegroundColor Yellow
} else {
    $msbuildArgs += "/t:Build"
    Write-Host "Building PiTrac solution ($Configuration|$Platform)..." -ForegroundColor Yellow
}

# Build the solution
try {
    # Verify Visual Studio environment looks healthy
    if (-not $env:INCLUDE -or -not $env:LIB -or -not $env:LIBPATH) {
        Write-Warning "Visual Studio environment variables don't appear to be properly set."
        Write-Host "Attempting to continue with build anyway..." -ForegroundColor Yellow
    } else {
        Write-Host "Visual Studio environment initialized successfully" -ForegroundColor Green
        Write-Host "Using compiler: $(Get-Command cl.exe -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Source)" -ForegroundColor Green
    }

    # Execute MSBuild with the configured arguments
    & $msbuildPath $msbuildArgs
    
    if ($LASTEXITCODE -eq 0) {
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
        Write-Host "Build failed with exit code: $LASTEXITCODE" -ForegroundColor Red
        exit $LASTEXITCODE
    }
} catch {
    Write-Host "Error during build: $_" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "To run PiTrac, use the Run-PiTrac.ps1 script:" -ForegroundColor Yellow
Write-Host "  .\Run-PiTrac.ps1" -ForegroundColor White
