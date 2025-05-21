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

$rootDirectory = git rev-parse --show-toplevel
#$workingDirectory = "$rootDirectory/Software/LMSourceCode"

Invoke-Psake "$rootDirectory/dev-setup/windows/psakefile.ps1" -task $Task