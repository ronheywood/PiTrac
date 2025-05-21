Task Default -Depends Help

Task Help {
    Write-Host "PiTrac Windows Setup Script" -ForegroundColor Cyan
    Write-Host "Available tasks:" -ForegroundColor Yellow
    Write-Host "  Help                : Displays this help message"
    Write-Host ""
    Write-Host "Example usage:" -ForegroundColor Green
    Write-Host "  Invoke-psake .\run.psake.ps1 Help"
}