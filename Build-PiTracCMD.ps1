# Build-PiTracCMD.ps1
# PowerShell script to build PiTrac solution using direct command-line tools
# This approach uses MSBuild directly from a VS Developer PowerShell

param (
    [string]$Configuration = "Release",
    [string]$Platform = "x64",
    [switch]$Rebuild = $false,
    [switch]$Clean = $false
)

# Display header
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host "    PiTrac Command-Line Build Interface      " -ForegroundColor Cyan
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host ""

# Check if running in Developer PowerShell for Visual Studio
$isDeveloperPowerShell = $false
if ($env:VSINSTALLDIR -and $env:VCToolsVersion) {
    $isDeveloperPowerShell = $true
    Write-Host "✅ Running in Developer PowerShell for Visual Studio" -ForegroundColor Green
} else {
    Write-Host "❌ Not running in Developer PowerShell for Visual Studio" -ForegroundColor Red
    Write-Host "For best results, please run this script from 'Developer PowerShell for VS 2022'" -ForegroundColor Yellow
    
    $continue = Read-Host "Continue anyway? (Y/N)"
    if ($continue -ne "Y" -and $continue -ne "y") {
        exit 0
    }
}

# Define paths
$baseDir = Resolve-Path .
$solutionDir = "$baseDir\Software\LMSourceCode"
$solutionFile = "$solutionDir\GolfSim.sln"
$projectFile = "$solutionDir\ImageProcessing\ImageProcessing.vcxproj"

# Verify solution and project exist
if (-not (Test-Path $solutionFile)) {
    Write-Host "Solution file not found at: $solutionFile" -ForegroundColor Red
    exit 1
}

if (-not (Test-Path $projectFile)) {
    Write-Host "Project file not found at: $projectFile" -ForegroundColor Red
    exit 1
}

# Check dependencies
Write-Host "Checking dependencies..." -ForegroundColor Yellow
$devLibsDir = "$env:USERPROFILE\DevLibs"
$opencvDir = "$devLibsDir\opencv-4.10.0"
$boostDir = "$devLibsDir\boost_1_83_0"

$opencvInstalled = Test-Path "$opencvDir\build"
$boostInstalled = (Test-Path "$boostDir\stage\lib") -or (Test-Path "$boostDir\lib") -or (Test-Path "$env:ChocolateyInstall\lib\boost-msvc-14.3")

if (-not $opencvInstalled) {
    Write-Host "❌ OpenCV not found at $opencvDir\build" -ForegroundColor Red
    Write-Host "Please run the OpenCV installation task first:" -ForegroundColor Yellow
    Write-Host ".\Setup-PiTracDev.ps1 -Task InstallOpenCV" -ForegroundColor White
    $continue = Read-Host "Continue anyway? (Y/N)"
    if ($continue -ne "Y" -and $continue -ne "y") {
        exit 0
    }
} else {
    Write-Host "✅ OpenCV found at $opencvDir\build" -ForegroundColor Green
}

if (-not $boostInstalled) {
    Write-Host "❌ Boost not found" -ForegroundColor Red
    Write-Host "Please run the Boost installation task first:" -ForegroundColor Yellow
    Write-Host ".\Setup-PiTracDev.ps1 -Task InstallBoost" -ForegroundColor White
    $continue = Read-Host "Continue anyway? (Y/N)"
    if ($continue -ne "Y" -and $continue -ne "y") {
        exit 0
    }
} else {
    Write-Host "✅ Boost found" -ForegroundColor Green
}

# Determine the right boost path
$boostIncludePath = $boostDir
$boostLibPath = "$boostDir\stage\lib"

if (Test-Path "$env:ChocolateyInstall\lib\boost-msvc-14.3") {
    $chocoBoostDir = "$env:ChocolateyInstall\lib\boost-msvc-14.3"
    if (Test-Path "$chocoBoostDir\include") {
        $boostIncludePath = "$chocoBoostDir\include"
    }
    if (Test-Path "$chocoBoostDir\lib") {
        $boostLibPath = "$chocoBoostDir\lib"
    }
}

# Determine OpenCV version and path
$opencvVcVersion = "vc16"  # Default
$vcFolders = Get-ChildItem -Path "$opencvDir\build\x64" -Directory -ErrorAction SilentlyContinue
if ($vcFolders) {
    $opencvVcVersion = $vcFolders[0].Name
}

# Set up include and library paths
$includeDirectories = "$opencvDir\build\include;$boostIncludePath"
$libraryDirectories = "$opencvDir\build\x64\$opencvVcVersion\lib;$boostLibPath"

# Find OpenCV libraries
$opencvLibPattern = "$opencvDir\build\x64\$opencvVcVersion\lib\opencv_world*.lib"
$opencvLibs = Get-ChildItem -Path $opencvLibPattern -ErrorAction SilentlyContinue | Select-Object -First 1 -ExpandProperty Name

if ($opencvLibs) {
    Write-Host "Found OpenCV library: $opencvLibs" -ForegroundColor Green
    $additionalDependencies = $opencvLibs
} else {
    Write-Host "No OpenCV library found matching pattern: $opencvLibPattern" -ForegroundColor Red
    $additionalDependencies = "opencv_world410.lib" # Default value, may need adjustment
}

# Set up MSBuild arguments
if ($Clean) {
    $action = "Clean"
    Write-Host "Cleaning PiTrac solution ($Configuration|$Platform)..." -ForegroundColor Yellow
} elseif ($Rebuild) {
    $action = "Rebuild"
    Write-Host "Rebuilding PiTrac solution ($Configuration|$Platform)..." -ForegroundColor Yellow
} else {
    $action = "Build"
    Write-Host "Building PiTrac solution ($Configuration|$Platform)..." -ForegroundColor Yellow
}

$msbuildArgs = @(
    $solutionFile,
    "/t:$action",
    "/p:Configuration=$Configuration",
    "/p:Platform=$Platform",
    "/p:IncludePath=`"$includeDirectories`"",
    "/p:LibraryPath=`"$libraryDirectories`"",
    "/p:AdditionalDependencies=`"$additionalDependencies`"",
    "/p:PreprocessorDefinitions=`"BOOST_BIND_GLOBAL_PLACEHOLDERS;BOOST_ALL_DYN_LINK;BOOST_USE_WINAPI_VERSION=0x0A00`"",
    "/p:RuntimeLibrary=`"MultiThreadedDLL`""
)

# Add additional useful build flags
$msbuildArgs += "/m"      # Use multiple CPU cores
$msbuildArgs += "/verbosity:normal"  # Show more detailed output

# Execute MSBuild
try {
    # For easier readability in the console, show what we're about to run
    Write-Host "Running:" -ForegroundColor Yellow
    Write-Host "MSBuild $($msbuildArgs -join ' ')" -ForegroundColor DarkGray
    Write-Host ""
    
    # Run MSBuild
    & MSBuild $msbuildArgs
    
    # Check the result
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
