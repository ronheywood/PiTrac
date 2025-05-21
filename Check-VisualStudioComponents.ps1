# Check-VisualStudioComponents.ps1
# PowerShell script to check for required Visual Studio components for PiTrac development

# Display header
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host "  Visual Studio Components Check for PiTrac   " -ForegroundColor Cyan
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host ""

# Check if running as admin
$isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Warning "This script should be run as administrator to fix any issues."
    Write-Warning "Consider restarting this script with administrator privileges."
}

# Check for Visual Studio installation
Write-Host "Checking for Visual Studio installations..." -ForegroundColor Yellow
$vsInstallerPath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vsInstallations = @()

if (Test-Path $vsInstallerPath) {
    # Use vswhere to find installations
    $installations = & $vsInstallerPath -all -format json | ConvertFrom-Json
    foreach ($installation in $installations) {
        $vsInstallations += @{
            InstallationPath = $installation.installationPath
            Version = $installation.installationVersion
            DisplayName = $installation.displayName
            ChannelId = $installation.channelId
            ProductId = $installation.productId
        }
    }
} else {
    # Manual check for common paths
    $vsRoots = @(
        "${env:ProgramFiles}\Microsoft Visual Studio",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio"
    )
    
    foreach ($vsRoot in $vsRoots) {
        if (Test-Path $vsRoot) {
            $yearFolders = Get-ChildItem -Path $vsRoot -Directory
            foreach ($yearFolder in $yearFolders) {
                $editionFolders = Get-ChildItem -Path $yearFolder.FullName -Directory
                foreach ($editionFolder in $editionFolders) {
                    $vsInstallations += @{
                        InstallationPath = $editionFolder.FullName
                        Version = "Unknown"
                        DisplayName = "Visual Studio $($yearFolder.Name) $($editionFolder.Name)"
                        ChannelId = "Unknown"
                        ProductId = "Unknown"
                    }
                }
            }
        }
    }
}

if ($vsInstallations.Count -eq 0) {
    Write-Host "No Visual Studio installations found." -ForegroundColor Red
    Write-Host "Please install Visual Studio with C++ workload from:" -ForegroundColor Yellow
    Write-Host "https://visualstudio.microsoft.com/downloads/" -ForegroundColor Yellow
    exit 1
} else {
    Write-Host "Found the following Visual Studio installations:" -ForegroundColor Green
    foreach ($installation in $vsInstallations) {
        Write-Host "  - $($installation.DisplayName) ($($installation.Version)) at $($installation.InstallationPath)" -ForegroundColor White
    }
}

# Check for C++ components
Write-Host "`nChecking for C++ components..." -ForegroundColor Yellow

$cppComponentsFound = $false
foreach ($installation in $vsInstallations) {
    $vcvarsallPath = Join-Path -Path $installation.InstallationPath -ChildPath "VC\Auxiliary\Build\vcvarsall.bat"
    if (Test-Path $vcvarsallPath) {
        $cppComponentsFound = $true
        Write-Host "  Found C++ components in $($installation.DisplayName)" -ForegroundColor Green
        break
    }
}

if (-not $cppComponentsFound) {
    Write-Host "C++ components not found in any Visual Studio installation." -ForegroundColor Red
    Write-Host "Please install the 'Desktop development with C++' workload:" -ForegroundColor Yellow
    
    if (Test-Path $vsInstallerPath) {
        Write-Host "You can do this by running Visual Studio Installer and modifying your installation." -ForegroundColor Yellow
        
        $openInstaller = Read-Host "Do you want to open Visual Studio Installer now? (Y/N)"
        if ($openInstaller -eq "Y" -or $openInstaller -eq "y") {
            Start-Process -FilePath $vsInstallerPath
        }
    } else {
        Write-Host "Please run the Visual Studio Installer and add the 'Desktop development with C++' workload" -ForegroundColor Yellow
    }
    
    exit 1
}

# Check for CL.exe in Path
Write-Host "`nChecking for C++ compiler in PATH..." -ForegroundColor Yellow
$clExe = Get-Command -Name cl.exe -ErrorAction SilentlyContinue

if ($clExe) {
    Write-Host "  Found C++ compiler (cl.exe) at: $($clExe.Source)" -ForegroundColor Green
} else {
    Write-Host "  C++ compiler (cl.exe) not found in PATH." -ForegroundColor Yellow
    Write-Host "  This is expected if you're not running in a Developer Command Prompt." -ForegroundColor Yellow
    
    # Find cl.exe in Visual Studio installation
    $clFound = $false
    foreach ($installation in $vsInstallations) {
        $clPaths = Get-ChildItem -Path $installation.InstallationPath -Filter "cl.exe" -Recurse -ErrorAction SilentlyContinue | 
            Where-Object { $_.Directory.Name -eq "bin" -and $_.FullName -like "*\VC\Tools\MSVC\*" }
        
        if ($clPaths -and $clPaths.Count -gt 0) {
            $clFound = $true
            Write-Host "  Found cl.exe at: $($clPaths[0].FullName)" -ForegroundColor Green
            break
        }
    }
    
    if (-not $clFound) {
        Write-Host "  Could not find cl.exe in any Visual Studio installation." -ForegroundColor Red
        Write-Host "  This may indicate a problem with your Visual Studio C++ installation." -ForegroundColor Red
    }
}

# Check if Developer Command Prompt works
Write-Host "`nTesting Visual Studio Developer Command Prompt..." -ForegroundColor Yellow

$devPromptFound = $false
foreach ($installation in $vsInstallations) {
    $vsDevCmdPath = Join-Path -Path $installation.InstallationPath -ChildPath "Common7\Tools\VsDevCmd.bat"
    if (Test-Path $vsDevCmdPath) {
        $devPromptFound = $true
        Write-Host "  Found VsDevCmd.bat at: $vsDevCmdPath" -ForegroundColor Green
        
        # Test the command prompt by running a command and capturing output
        $tempFile = [System.IO.Path]::GetTempFileName()
        cmd.exe /c "`"$vsDevCmdPath`" && where cl.exe > `"$tempFile`" 2>&1"
        $clPath = Get-Content -Path $tempFile -ErrorAction SilentlyContinue
        Remove-Item -Path $tempFile -ErrorAction SilentlyContinue
        
        if ($clPath -and $clPath -like "*cl.exe") {
            Write-Host "  Developer Command Prompt works correctly and can find cl.exe" -ForegroundColor Green
        } else {
            Write-Host "  Developer Command Prompt exists but could not find cl.exe" -ForegroundColor Red
            Write-Host "  This may indicate a problem with your Visual Studio C++ installation." -ForegroundColor Red
        }
        break
    }
}

if (-not $devPromptFound) {
    Write-Host "  Could not find Visual Studio Developer Command Prompt." -ForegroundColor Red
}

# Check project's solution file
Write-Host "`nChecking PiTrac solution file..." -ForegroundColor Yellow
$solutionFile = "c:\kata\PiTrac\Software\LMSourceCode\GolfSim.sln"

if (Test-Path $solutionFile) {
    Write-Host "  Solution file exists at: $solutionFile" -ForegroundColor Green
    
    # Check if solution can be parsed
    $solutionContent = Get-Content $solutionFile -ErrorAction SilentlyContinue
    if ($solutionContent -and ($solutionContent -like "*Microsoft Visual Studio Solution File*")) {
        Write-Host "  Solution file appears to be valid" -ForegroundColor Green
    } else {
        Write-Host "  Solution file may be corrupted or in an unexpected format" -ForegroundColor Red
    }
} else {
    Write-Host "  Solution file not found at expected location: $solutionFile" -ForegroundColor Red
}

# Examine project settings
Write-Host "`nExamining project settings..." -ForegroundColor Yellow
$projectFile = "c:\kata\PiTrac\Software\LMSourceCode\ImageProcessing\ImageProcessing.vcxproj"

if (Test-Path $projectFile) {
    Write-Host "  Project file exists at: $projectFile" -ForegroundColor Green
    
    # Parse include paths from the project file
    $projectXml = [xml](Get-Content $projectFile)
    $includePaths = $projectXml.Project.ItemDefinitionGroup.ClCompile.AdditionalIncludeDirectories
    
    if ($includePaths) {
        Write-Host "  Additional Include Directories:" -ForegroundColor White
        foreach ($path in $includePaths.Split(';')) {
            if ($path -and $path.Trim() -ne "") {
                $expandedPath = $path.Replace('$(UserProfile)', $env:USERPROFILE)
                Write-Host "    - $expandedPath" -ForegroundColor White
                
                # Check if the path exists
                if (-not (Test-Path $expandedPath)) {
                    Write-Host "      WARNING: Path does not exist" -ForegroundColor Red
                }
            }
        }
    } else {
        Write-Host "  No additional include directories found in the project file" -ForegroundColor Yellow
    }
    
    # Parse lib paths from the project file
    $libPaths = $projectXml.Project.ItemDefinitionGroup.Link.AdditionalLibraryDirectories
    
    if ($libPaths) {
        Write-Host "  Additional Library Directories:" -ForegroundColor White
        foreach ($path in $libPaths.Split(';')) {
            if ($path -and $path.Trim() -ne "") {
                $expandedPath = $path.Replace('$(UserProfile)', $env:USERPROFILE)
                Write-Host "    - $expandedPath" -ForegroundColor White
                
                # Check if the path exists
                if (-not (Test-Path $expandedPath)) {
                    Write-Host "      WARNING: Path does not exist" -ForegroundColor Red
                }
            }
        }
    } else {
        Write-Host "  No additional library directories found in the project file" -ForegroundColor Yellow
    }
} else {
    Write-Host "  Project file not found at expected location: $projectFile" -ForegroundColor Red
}

# Check if dependency libraries are installed where expected
Write-Host "`nChecking for dependency libraries..." -ForegroundColor Yellow

$devLibsDir = "$env:USERPROFILE\DevLibs"
$opencvDir = "$devLibsDir\opencv-4.10.0"
$boostDir = "$devLibsDir\boost_1_83_0"

# Check OpenCV
if (Test-Path $opencvDir) {
    Write-Host "  OpenCV found at: $opencvDir" -ForegroundColor Green
    
    # Check for build directory
    if (Test-Path "$opencvDir\build") {
        Write-Host "    OpenCV build directory exists" -ForegroundColor Green
        
        # Check for include directory
        if (Test-Path "$opencvDir\build\include") {
            Write-Host "    OpenCV include directory exists" -ForegroundColor Green
        } else {
            Write-Host "    WARNING: OpenCV include directory not found" -ForegroundColor Red
        }
        
        # Check for libraries
        $opencvLibs = Get-ChildItem -Path "$opencvDir\build" -Filter "*.lib" -Recurse -ErrorAction SilentlyContinue
        if ($opencvLibs -and $opencvLibs.Count -gt 0) {
            Write-Host "    OpenCV libraries found" -ForegroundColor Green
        } else {
            Write-Host "    WARNING: No OpenCV libraries found" -ForegroundColor Red
        }
    } else {
        Write-Host "    WARNING: OpenCV build directory not found" -ForegroundColor Red
    }
} else {
    Write-Host "  OpenCV not found at expected location: $opencvDir" -ForegroundColor Red
    Write-Host "  Please run the InstallOpenCV task:" -ForegroundColor Yellow
    Write-Host "  .\Setup-PiTracDev.ps1 -Task InstallOpenCV" -ForegroundColor White
}

# Check Boost
$boostInstalled = (Test-Path "$boostDir\stage\lib") -or (Test-Path "$boostDir\lib") -or (Test-Path "$env:ChocolateyInstall\lib\boost-msvc-14.3")

if ($boostInstalled) {
    Write-Host "  Boost libraries found" -ForegroundColor Green
    
    # Check if installed via Chocolatey
    if (Test-Path "$env:ChocolateyInstall\lib\boost-msvc-14.3") {
        Write-Host "    Boost installed via Chocolatey at: $env:ChocolateyInstall\lib\boost-msvc-14.3" -ForegroundColor Green
    }
    
    # Check standard installation
    if (Test-Path $boostDir) {
        Write-Host "    Boost found at: $boostDir" -ForegroundColor Green
        
        # Check for include directory
        if (Test-Path "$boostDir\boost") {
            Write-Host "    Boost include directory exists" -ForegroundColor Green
        } else {
            Write-Host "    WARNING: Boost include directory not found" -ForegroundColor Yellow
        }
        
        # Check for libraries
        if (Test-Path "$boostDir\stage\lib") {
            $boostLibs = Get-ChildItem -Path "$boostDir\stage\lib" -Filter "*.lib" -ErrorAction SilentlyContinue
            if ($boostLibs -and $boostLibs.Count -gt 0) {
                Write-Host "    Boost libraries found in stage\lib" -ForegroundColor Green
            } else {
                Write-Host "    WARNING: No Boost libraries found in stage\lib" -ForegroundColor Yellow
            }
        } elseif (Test-Path "$boostDir\lib") {
            $boostLibs = Get-ChildItem -Path "$boostDir\lib" -Filter "*.lib" -ErrorAction SilentlyContinue
            if ($boostLibs -and $boostLibs.Count -gt 0) {
                Write-Host "    Boost libraries found in lib" -ForegroundColor Green
            } else {
                Write-Host "    WARNING: No Boost libraries found in lib" -ForegroundColor Yellow
            }
        } else {
            Write-Host "    WARNING: Boost library directories not found" -ForegroundColor Red
        }
    }
} else {
    Write-Host "  Boost not found at expected locations" -ForegroundColor Red
    Write-Host "  Please run the InstallBoost task:" -ForegroundColor Yellow
    Write-Host "  .\Setup-PiTracDev.ps1 -Task InstallBoost" -ForegroundColor White
}

# Summary
Write-Host "`nDiagnosis Summary:" -ForegroundColor Yellow
if ($cppComponentsFound -and $devPromptFound -and (Test-Path $solutionFile) -and (Test-Path $opencvDir) -and $boostInstalled) {
    Write-Host "✅ All required components appear to be installed." -ForegroundColor Green
    Write-Host "Try building the solution using Visual Studio directly:" -ForegroundColor Green
    Write-Host "1. Open Visual Studio" -ForegroundColor White
    Write-Host "2. Open the solution file: $solutionFile" -ForegroundColor White
    Write-Host "3. Select 'Release' configuration and 'x64' platform" -ForegroundColor White
    Write-Host "4. Build the solution (F7 or Build > Build Solution)" -ForegroundColor White
} else {
    Write-Host "❌ Some components are missing or incorrectly configured." -ForegroundColor Red
    Write-Host "Please address the warnings above before attempting to build." -ForegroundColor Yellow
    
    # Suggestions
    Write-Host "`nSuggestions:" -ForegroundColor Cyan
    
    if (-not $cppComponentsFound) {
        Write-Host "- Install the 'Desktop development with C++' workload in Visual Studio" -ForegroundColor White
    }
    
    if (-not (Test-Path $opencvDir)) {
        Write-Host "- Install OpenCV: .\Setup-PiTracDev.ps1 -Task InstallOpenCV" -ForegroundColor White
    }
    
    if (-not $boostInstalled) {
        Write-Host "- Install Boost: .\Setup-PiTracDev.ps1 -Task InstallBoost" -ForegroundColor White
    }
    
    Write-Host "- Update your project's include and library paths to match your installed dependencies" -ForegroundColor White
    Write-Host "- Consider running Visual Studio as Administrator" -ForegroundColor White
}

Write-Host "`nFor additional help, see the Windows Development guide:" -ForegroundColor Cyan
Write-Host "c:\kata\PiTrac\Windows-Development.md" -ForegroundColor White
