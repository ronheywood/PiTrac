# Run-PiTrac.ps1
# PowerShell script to run PiTrac on Windows
# This is a wrapper around the psake run script to make it easier to run

param (
    [string]$Task = "Help",
    [string]$TestImage = "",
    [hashtable]$CustomOptions = @{}
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
Write-Host "            PiTrac Run Interface             " -ForegroundColor Cyan
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host ""

# Prepare properties based on parameters
$properties = @{}

if ($TestImage) {
    $properties["TestImage"] = $TestImage
}

# Add any custom options
foreach ($key in $CustomOptions.Keys) {
    $properties[$key] = $CustomOptions[$key]
}

# Execute the specified task
try {
    Invoke-psake -buildFile "$PSScriptRoot\run.psake.ps1" -taskList $Task -parameters $properties
    if (-not $psake.build_success) {
        throw "Task failed. See error details above."
    }
} 
catch {
    Write-Host "An error occurred: $_" -ForegroundColor Red
    exit 1
}

# Display instructions for running specific tasks
Write-Host ""
Write-Host "Common commands:" -ForegroundColor Yellow
Write-Host "  .\Run-PiTrac.ps1" -ForegroundColor White
Write-Host "  .\Run-PiTrac.ps1 -Task Run" -ForegroundColor White
Write-Host "  .\Run-PiTrac.ps1 -Task RunWithImage -TestImage ball_01.jpg" -ForegroundColor White
Write-Host "  .\Run-PiTrac.ps1 -Task ListTestImages" -ForegroundColor White
Write-Host "  .\Run-PiTrac.ps1 -Task CleanLogs" -ForegroundColor White
Write-Host ""
Write-Host "For advanced options:" -ForegroundColor Yellow
Write-Host "  .\Run-PiTrac.ps1 -Task RunWithDebugOptions -CustomOptions @{" -ForegroundColor White
Write-Host "      ShowImages = 1" -ForegroundColor White
Write-Host "      LoggingLevel = 'verbose'" -ForegroundColor White
Write-Host "      TestImageFilename = 'C:\Path\To\Custom\Image.jpg'" -ForegroundColor White
Write-Host "  }" -ForegroundColor White
