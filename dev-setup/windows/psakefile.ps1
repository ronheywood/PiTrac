Task Default -Depends Help

Task Help {
    Write-Host "PiTrac Windows Setup Script" -ForegroundColor Cyan
    Write-Host "Available tasks:" -ForegroundColor Yellow
    Write-Host "  Help                : Displays this help message"
    Write-Host ""
    Write-Host "Example usage:" -ForegroundColor Green
    Write-Host "  Invoke-psake .\run.psake.ps1 Help"
}

Task Install {
    Write-Host "Installing PiTrac dependencies..." -ForegroundColor Green
    # Install Chocolatey if not already installed
    if (-not (Get-Command choco -ErrorAction SilentlyContinue)) {
        Write-Host "Installing Chocolatey..." -ForegroundColor Yellow
        Set-ExecutionPolicy Bypass -Scope Process -Force; Invoke-Expression ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
    }

    choco install .\choco-packages.config
}