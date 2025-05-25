# Configure Windows for PiTrac Development
# This script installs Chocolatey, OpenCV, Boost, Visual Studio 2022 Community Edition, and psake.
# It also sets environment variables for OPENCV_DIR and BOOST_ROOT.


function Install-Choco {
    if (!(Get-Command choco -ErrorAction SilentlyContinue)) {
        Write-Host "Installing Chocolatey..."
        Set-ExecutionPolicy Bypass -Scope Process -Force
        [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
        Invoke-Expression ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
    } else {
        Write-Host "Chocolatey already installed."
    }
}

function Install-Package-IfNeeded {
    param(
        [string]$PackageName,
        [string]$ChocoName = $PackageName
    )
    if (!(choco list | Select-String "^$ChocoName ")) {
        Write-Host "Installing $PackageName..."
        choco install $ChocoName -y
    } else {
        Write-Host "$PackageName already installed."
    }
}

function Set-EnvVar-IfNeeded {
    param(
        [string]$Name,
        [string]$Value
    )
    $current = [Environment]::GetEnvironmentVariable($Name, 'Machine')
    if ($current -ne $Value) {
        Write-Host "Setting environment variable $Name to $Value (Machine scope)"
        [Environment]::SetEnvironmentVariable($Name, $Value, 'Machine')
    } else {
        Write-Host "$Name already set at Machine scope."
    }
}

# Set PITRAC_ROOT as a machine environment variable for Windows builds
$pitRacRoot = Join-Path $PSScriptRoot 'Software\LMSourceCode'
Set-EnvVar-IfNeeded -Name 'PITRAC_ROOT' -Value $pitRacRoot -Scope 'Machine'
Write-Host "Set PITRAC_ROOT to $pitRacRoot (Machine scope)"

# Helper function to download and install Boost if not present
function Ensure-BoostInstalled {
    param(
        [string]$boostTargetDir,
        [string]$boostExeUrl
    )
    $boostLibDir = Join-Path $boostTargetDir 'lib64-msvc-14.3'
    if (!(Test-Path $boostLibDir)) {
        $boostExe = "$env:TEMP\boost_1_87_0-msvc-14.3-64.exe"
        Write-Host "Downloading and installing Boost..."
        Invoke-WebRequest -UserAgent "WGet" -Uri $boostExeUrl -OutFile $boostExe -UseBasicParsing
        if (!(Test-Path $boostTargetDir)) { New-Item -ItemType Directory -Path $boostTargetDir | Out-Null }
        # Run the self-extracting EXE with silent switches
        Write-Host "Running Boost self-extracting installer..."
        Start-Process -FilePath $boostExe -ArgumentList "/DIR=$boostTargetDir", "/SILENT" -Wait
        Remove-Item $boostExe
        Write-Host "Boost installed to $boostTargetDir"
    } else {
        Write-Host "Boost already present at $boostTargetDir"
    }
}

function Set-BoostEnvironment {
    $boostTargetDir = "C:\Dev_Libs\boost"
    $boostExeUrl = "https://sourceforge.net/projects/boost/files/boost-binaries/1.87.0/boost_1_87_0-msvc-14.3-64.exe/download"
    Ensure-BoostInstalled -boostTargetDir $boostTargetDir -boostExeUrl $boostExeUrl
    Set-EnvVar-IfNeeded -Name "BOOST_ROOT" -Value $boostTargetDir -Scope 'Machine'
}

function Test-OpenCVProjectPath {
    param(
        [string]$vcxprojPath
    )

    [xml]$vcxproj = Get-Content $vcxprojPath
    $includeDir = $vcxproj.Project.ItemDefinitionGroup | Where-Object { $_.Condition -like "*Debug|x64*" } | ForEach-Object { $_.ClCompile.AdditionalIncludeDirectories } | Select-Object -First 1
    $opencvInclude = $null
    if ($includeDir) {
        $parts = $includeDir -split ";"
        $opencvInclude = $parts | Where-Object { $_ -match "opencv" } | Select-Object -First 1
    }
    $opencvRoot = $null
    if ($opencvInclude -and ($opencvInclude -match "(.+?)\\build\\include")) {
        $opencvRoot = $Matches[1] + "\build"
    }

    if ($opencvRoot -and (Test-Path $opencvRoot)) {
        Write-Host "OpenCV project path is valid: $opencvRoot"
        return $true
    } 
    elseif ($opencvRoot) {
        Write-Host "OpenCV project path is set but not found: $opencvRoot"
        return $false
    } 

    Write-Host "Could not determine OpenCV root from project file."
    return $false
}

function Set-OpenCvEnvironment {
    # Analyze project file for expected OpenCV location
    $scriptPath = $MyInvocation.PSCommandPath
    $scriptDir = Split-Path -Parent $scriptPath
    $vcxprojPath = Join-Path $scriptDir 'Software\LMSourceCode\ImageProcessing\ImageProcessing.vcxproj'
    $opencvDir = [Environment]::GetEnvironmentVariable("OPENCV_DIR", 'Machine')
    if (-not $opencvDir -or -not (Test-Path $opencvDir)) {
        $opencvConfigured = Test-OpenCVProjectPath -vcxprojPath $vcxprojPath
        if (-not $opencvConfigured) {
            Write-Host "OpenCV is not properly configured. Please ensure OpenCV is installed and the project file is updated."
        }
        return;
    }
    Set-EnvVar-IfNeeded -Name "OPENCV_DIR" -Value $opencvDir -Scope 'Machine'
}

# Main script
Write-Host "--- PiTrac Windows Configuration Script ---"

Install-Choco
#Install-Package-IfNeeded -PackageName "Visual Studio 2022 Community" -ChocoName "visualstudio2022community"
Install-Package-IfNeeded -PackageName "psake" -ChocoName "psake"
Install-Package-IfNeeded -PackageName "7zip" -ChocoName "7zip"

Set-OpenCvEnvironment
Set-BoostEnvironment

#Ensure machine environment variables are loaded
Import-Module "$env:ChocolateyInstall\helpers\chocolateyProfile.psm1"
refreshenv

Write-Host "Setup complete!"
Write-Host "You can now open the Visual Studio solution and build the solution."
Write-Host "Alternatively run ./build_and_run_windows.ps1"

# End of configure-windows.ps1
