# Build-PiTracWithDevShell.ps1
# PowerShell script to build PiTrac solution by launching a VS Developer PowerShell
# This script ensures the build runs in the proper Visual Studio environment

param (
    [string]$Configuration = "Release",
    [string]$Platform = "x64",
    [switch]$Rebuild = $false,
    [switch]$Clean = $false
)

# Display header
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host "   PiTrac Visual Studio DevShell Builder     " -ForegroundColor Cyan
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host ""

# Find Visual Studio installation
$vsInstallationPath = $null
$vsInstallerPath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"

if (Test-Path $vsInstallerPath) {
    $vsInstallationPath = & $vsInstallerPath -latest -property installationPath 2>$null
}

if (-not $vsInstallationPath) {
    # Try common paths
    $possiblePaths = @(
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Professional",
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Enterprise",
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Community",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\Professional",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\Enterprise",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\Community"
    )
    
    foreach ($path in $possiblePaths) {
        if (Test-Path $path) {
            $vsInstallationPath = $path
            break
        }
    }
}

if (-not $vsInstallationPath) {
    Write-Host "Could not find Visual Studio installation." -ForegroundColor Red
    Write-Host "Please install Visual Studio 2019 or 2022 with C++ workload." -ForegroundColor Yellow
    exit 1
}

# Find the VS PowerShell script
$launchVsDevShell = Join-Path -Path $vsInstallationPath -ChildPath "Common7\Tools\Launch-VsDevShell.ps1"
if (-not (Test-Path $launchVsDevShell)) {
    Write-Host "Visual Studio Developer PowerShell launcher not found at:" -ForegroundColor Red
    Write-Host $launchVsDevShell -ForegroundColor Red
    Write-Host "Please ensure Visual Studio is properly installed with C++ workload." -ForegroundColor Yellow
    exit 1
}

# Prepare the command to run
$buildArgs = ""
if ($Rebuild) {
    $buildArgs = "-Rebuild"
} elseif ($Clean) {
    $buildArgs = "-Clean"
}

if ($Configuration -ne "Release") {
    $buildArgs += " -Configuration '$Configuration'"
}

if ($Platform -ne "x64") {
    $buildArgs += " -Platform '$Platform'"
}

# Create a temporary script to execute in the VS PowerShell
$tempScript = [System.IO.Path]::GetTempFileName() + ".ps1"
$currentDir = (Get-Location).Path

$scriptContent = @"
# This is a temporary script to build PiTrac in a VS Developer PowerShell
Set-Location "$currentDir"
Write-Host "Current directory: `$((Get-Location).Path)" -ForegroundColor Green
Write-Host "MSBuild location: `$(Get-Command MSBuild -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Source)" -ForegroundColor Green
Write-Host "CL.exe location: `$(Get-Command cl.exe -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Source)" -ForegroundColor Green

Write-Host "`nRunning Build-PiTracCMD.ps1 $buildArgs" -ForegroundColor Yellow
.\Build-PiTracCMD.ps1 $buildArgs

Write-Host "`nPress Enter to close this window..." -ForegroundColor Cyan
Read-Host
"@

Set-Content -Path $tempScript -Value $scriptContent

Write-Host "Launching Visual Studio Developer PowerShell..." -ForegroundColor Yellow
Write-Host "Build script will continue in the new window." -ForegroundColor Yellow
Write-Host ""

# Launch VS Developer PowerShell with our script
# Separate the arguments to avoid the parameter validation error
Start-Process powershell.exe -ArgumentList "-NoExit", "-ExecutionPolicy", "Bypass", "-File", "`"$launchVsDevShell`"", "-SkipAutomaticLocation", "-Arch", "$Platform", "-Command", "& {& `"$tempScript`"}" -Wait

# Clean up
if (Test-Path $tempScript) {
    Remove-Item $tempScript -Force
}

Write-Host "Build process completed in the Visual Studio Developer PowerShell." -ForegroundColor Green
