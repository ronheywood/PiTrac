# Install-VSCppWorkload.ps1
# PowerShell script to install the C++ workload in Visual Studio

param (
    [switch]$SkipAdminCheck,
    [switch]$InstallCommandLineOnly
)

function OpenVSDownloadPage {
    Write-Host "Please download and install Visual Studio with the C++ workload:" -ForegroundColor Yellow
    Write-Host "https://visualstudio.microsoft.com/downloads/" -ForegroundColor White
    
    $openBrowser = Read-Host "Do you want to open the download page now? (Y/N)"
    if ($openBrowser -eq "Y" -or $openBrowser -eq "y") {
        Start-Process "https://visualstudio.microsoft.com/downloads/"
    }
}

# Check if running as admin, unless explicitly skipped
if (-not $SkipAdminCheck) {
    $isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
    if (-not $isAdmin) {
        Write-Host "This script requires Administrator privileges." -ForegroundColor Red
        Write-Host "Please restart PowerShell as Administrator and run this script again." -ForegroundColor Yellow
        exit 1
    }
}

# Display header
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host "  Install Visual Studio C++ Workload for PiTrac " -ForegroundColor Cyan
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host ""

# Find Visual Studio installation
$vsInstallerPath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vsInstallerPath) {
    Write-Host "Using vswhere.exe to locate Visual Studio Installer..." -ForegroundColor Yellow
    $vsInstallation = & $vsInstallerPath -latest -format json | ConvertFrom-Json
    
    if ($vsInstallation) {
        Write-Host "Found Visual Studio installation: $($vsInstallation.displayName)" -ForegroundColor Green
        $vsInstallerExe = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vs_installer.exe"
        
        if (Test-Path $vsInstallerExe) {
            Write-Host "Launching Visual Studio Installer to add the C++ workload..." -ForegroundColor Yellow
              # This will modify the existing installation to add the Desktop C++ workload
            Write-Host "Starting installation. This may take a while..." -ForegroundColor Yellow
            Start-Process -FilePath $vsInstallerExe -ArgumentList "modify --installPath `"$($vsInstallation.installationPath)`" --add Microsoft.VisualStudio.Workload.NativeDesktop --includeRecommended --passive" -NoNewWindow -Wait
            
            Write-Host "`nVisual Studio C++ workload installation has completed." -ForegroundColor Green
        Write-Host "Please verify that the installation was successful." -ForegroundColor Yellow
            
            # Check if we should also install command line build tools
            $installBuildTools = $true
            if ($InstallCommandLineOnly -eq $false) {
                $installBuildTools = (Read-Host "Do you also want to install command-line build tools for PowerShell building? (Y/N)") -eq "Y"
            }
            
            if ($installBuildTools) {
                Write-Host "`nInstalling Build Tools for command-line building..." -ForegroundColor Yellow
                # This installs the command-line build tools
                Start-Process -FilePath $vsInstallerExe -ArgumentList "modify --installPath `"$($vsInstallation.installationPath)`" --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 --add Microsoft.VisualStudio.Component.Windows10SDK --passive" -NoNewWindow -Wait
                Write-Host "Command-line build tools installation completed." -ForegroundColor Green
            }
            
            Write-Host "`nAfter installation, try building the PiTrac solution again:" -ForegroundColor Yellow
            Write-Host "1. Open Visual Studio" -ForegroundColor White
            Write-Host "2. Open the solution file: C:\kata\PiTrac\Software\LMSourceCode\GolfSim.sln" -ForegroundColor White
            Write-Host "3. Select 'Release' configuration and 'x64' platform" -ForegroundColor White
            Write-Host "4. Build the solution (F7 or Build > Build Solution)" -ForegroundColor White
            
            if ($installBuildTools) {
                Write-Host "`nOr build from command line:" -ForegroundColor Yellow
                Write-Host "1. Open a Developer PowerShell for VS 2022" -ForegroundColor White
                Write-Host "2. Navigate to the PiTrac directory: cd C:\kata\PiTrac" -ForegroundColor White  
                Write-Host "3. Run: .\Build-PiTrac.ps1" -ForegroundColor White
            }
        } else {
            Write-Host "Visual Studio Installer executable not found." -ForegroundColor Red
            Write-Host "Please install the 'Desktop development with C++' workload manually:" -ForegroundColor Yellow
            Write-Host "1. Open Visual Studio Installer" -ForegroundColor White
            Write-Host "2. Select 'Modify' for your Visual Studio installation" -ForegroundColor White
            Write-Host "3. Check 'Desktop development with C++' workload" -ForegroundColor White
            Write-Host "4. Click 'Modify' to install the component" -ForegroundColor White
        }
    } else {
        Write-Host "No Visual Studio installation found." -ForegroundColor Red
        OpenVSDownloadPage
    }
} else {
    Write-Host "Visual Studio Installer not found." -ForegroundColor Red
    OpenVSDownloadPage
}

function OpenVSDownloadPage {
    Write-Host "Please download and install Visual Studio with the C++ workload:" -ForegroundColor Yellow
    Write-Host "https://visualstudio.microsoft.com/downloads/" -ForegroundColor White
    
    $openBrowser = Read-Host "Do you want to open the download page now? (Y/N)"
    if ($openBrowser -eq "Y" -or $openBrowser -eq "y") {
        Start-Process "https://visualstudio.microsoft.com/downloads/"
    }
}
