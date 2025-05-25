# build_and_run_windows.ps1
# Builds the PiTrac solution and runs the ImageProcessing executable with the correct environment variables for OpenCV and Boost.

function Get-FirstValidPath {
    param(
        [string[]]$possiblePaths
    )
    foreach ($path in $possiblePaths) {
        if (Test-Path $path) { return $path }
    }
    return $null
}

function Get-MSBuildPath {
    param([switch]$ListAll)
    $possiblePaths = @(
        'C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe',
        'C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe',
        'C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe',
        'C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe',
        'C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe',
        'C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\MSBuild\Current\Bin\MSBuild.exe'
    )
    if ($ListAll) { return $possiblePaths }
    $result = Get-FirstValidPath $possiblePaths
    if ($result) { return $result }
    Write-Host "MSBuild.exe not found. Please install Visual Studio with Desktop development tools." -ForegroundColor Red
    exit 1
}

function Get-VsDevPath {
    param([switch]$ListAll)
    $possiblePaths = @(
        'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat',
        'C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat',
        'C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat',
        'C:\Program Files (x86)\Microsoft Visual Studio\2019\Common7\Tools\VsDevCmd.bat',
        'C:\Program Files (x86)\Microsoft Visual Studio\2019\Common7\Tools\VsDevCmd.bat',
        'C:\Program Files (x86)\Microsoft Visual Studio\2019\Common7\Tools\VsDevCmd.bat'
    )
    if ($ListAll) { return $possiblePaths }
    $result = Get-FirstValidPath $possiblePaths
    if ($result) { return $result }
    Write-Host "VsDevCmd.bat not found. Please install Visual Studio with Desktop development tools." -ForegroundColor Red
    exit 1
}

function Test-VisualStudioCppWorkload {
    param([string]$msbuildPath)
    # Go up to the Visual Studio install root
    $vsRoot = Split-Path (Split-Path (Split-Path (Split-Path $msbuildPath)))
    # Search for cl.exe anywhere under the VS root
    $cl = Get-ChildItem -Path $vsRoot -Recurse -Filter cl.exe -ErrorAction SilentlyContinue | Select-Object -First 1
    return ($null -ne $cl)
}

function Get-ValidMSBuildWithCpp {
    $candidates = Get-MSBuildPath -ListAll
    foreach ($msbuild in $candidates) {
        if (Test-Path $msbuild) {
            if (Test-VisualStudioCppWorkload $msbuild) {
                return $msbuild
            }
        }
    }
    return $null
}

$msbuild = Get-ValidMSBuildWithCpp
if (-not $msbuild) {
    Write-Host "No Visual Studio installation with both MSBuild and C++ build tools (cl.exe) was found. Please install the 'Desktop development with C++' workload in at least one Visual Studio edition." -ForegroundColor Red
    exit 1
}
$vsDev = Get-VsDevPath

$scriptDir = $PSScriptRoot
$solution = Join-Path $scriptDir "Software\LMSourceCode\GolfSim.sln"
if (-not (Test-Path $solution)) {
    Write-Host "Solution file not found: $solution"
    exit 1
}
# Run msbuild inside a new cmd.exe process that first calls VsDevCmd.bat
$msbuildCmd = '"' + $msbuild + '" "' + $solution + '" /p:Configuration=Debug /p:Platform=x64'
$cmd = "/c call `"$vsDev`" && $msbuildCmd"
Write-Host "Running: cmd $cmd"
$buildResult = Start-Process cmd -ArgumentList $cmd -Wait -PassThru

if ($buildResult.ExitCode -ne 0) {
    Write-Host "Build failed with exit code $($buildResult.ExitCode)" -ForegroundColor Red
    exit $buildResult.ExitCode
} else {
    Write-Host "Build succeeded!" -ForegroundColor Green
}

# Set up environment variables for OpenCV and Boost
$opencvDir = [Environment]::GetEnvironmentVariable("OPENCV_DIR", "User")
$boostRoot = [Environment]::GetEnvironmentVariable("BOOST_ROOT", "User")

if (-not $opencvDir) {
    Write-Host "OPENCV_DIR environment variable is not set. Please run configure-windows.ps1 first."
    exit 1
}
if (-not $boostRoot) {
    Write-Host "BOOST_ROOT environment variable is not set. Please run configure-windows.ps1 first."
    exit 1
}

$env:PATH = "$env:PATH;$opencvDir\x64\vc16\bin;$boostRoot\lib64-msvc-14.3"

# Path to the built executable (relative to script location)
$exe = Join-Path $scriptDir "Software\LMSourceCode\x64\Debug\ImageProcessing.exe"

if (-not (Test-Path $exe)) {
    Write-Host "Build failed or executable not found: $exe"
    exit 1
}

# Run the executable (inherits PATH and other env vars)
$exeDir = Split-Path $exe
Push-Location $exeDir
try {
    $exeArgs = @(
        "--show_images", "1",
        "--lm_comparison_mode=0",
        "--logging_level", "trace",
        "--artifact_save_level=all",
        "--wait_keys", "1",
        "--system_mode", "camera1_test_standalone",
        "--search_center_x", "800",
        "--search_center_y", "550"
    )
    & $exe @exeArgs
} finally {
    Pop-Location
}
