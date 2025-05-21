# Setup-PiTracDev.ps1
# PowerShell script to set up PiTrac development environment on Windows
# This is a wrapper around the psake build script to make it easier to run

param (
    [string]$Task = "Help"
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
Write-Host "   PiTrac Windows Development Environment    " -ForegroundColor Cyan
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host ""

# Execute the specified task
try {
    Invoke-psake -buildFile "$PSScriptRoot\build.psake.ps1" -taskList $Task
    if (-not $psake.build_success) {
        throw "Build failed. See error details above."
    }
} 
catch {
    Write-Host "An error occurred: $_" -ForegroundColor Red
    exit 1
}

# Display instructions for running specific tasks
Write-Host ""
Write-Host "To run a specific task, use:" -ForegroundColor Yellow
Write-Host "  .\Setup-PiTracDev.ps1 -Task <TaskName>" -ForegroundColor White
Write-Host ""
Write-Host "For example, to install only OpenCV:" -ForegroundColor Yellow
Write-Host "  .\Setup-PiTracDev.ps1 -Task InstallOpenCV" -ForegroundColor White
Write-Host ""
Write-Host "To run all setup tasks:" -ForegroundColor Yellow
Write-Host "  .\Setup-PiTracDev.ps1 -Task All" -ForegroundColor White
