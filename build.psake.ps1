##############################################################################
# PiTrac Windows Development Environment Setup
# Automated build script using psake
#
# Usage:
# Import-Module psake
# Invoke-psake .\build.psake.ps1
##############################################################################

Properties {
    $baseDir = Resolve-Path .
    $devLibsDir = "$env:USERPROFILE\DevLibs"
    $opencvVersion = "4.10.0"
    $boostVersion = "1.83.0" # Using a more stable version
    $boostVersionUnderscore = $boostVersion -replace "\.", "_"
    $vsVersion = "2022"
    $vcVersion = "14.3"  # Visual C++ version, modify as needed
    
    # Directories
    $opencvDir = "$devLibsDir\opencv-$opencvVersion"
    $boostDir = "$devLibsDir\boost_$boostVersionUnderscore"

    # Visual Studio detection
    $vsPath = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath
    if (-not $vsPath) {
        $vsPath = & "$env:ProgramFiles\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath
    }
    
    # URLs
    $opencvUrl = "https://github.com/opencv/opencv/releases/download/$opencvVersion/opencv-$opencvVersion-windows.exe"
    $boostUrl = "https://boostorg.jfrog.io/artifactory/main/release/$boostVersion/source/boost_$boostVersionUnderscore.zip"
      # Solution file path
    $solutionFile = "$baseDir\Software\LMSourceCode\GolfSim.sln"
}

Task Default -Depends Help

Task Help {
    Write-Host "PiTrac Windows Development Environment Setup" -ForegroundColor Cyan
    Write-Host "Available tasks:" -ForegroundColor Yellow
    Write-Host "  Init          : Prepares directories and checks prerequisites"
    Write-Host "  InstallChoco  : Installs Chocolatey (if not already installed)"
    Write-Host "  InstallTools  : Installs basic development tools via Chocolatey"
    Write-Host "  InstallOpenCV : Downloads and installs OpenCV"
    Write-Host "  InstallBoost  : Downloads and installs Boost libraries"
    Write-Host "  ConfigureVS   : Configures Visual Studio project for PiTrac"
    Write-Host "  All           : Runs all tasks in sequence"
    Write-Host ""
    Write-Host "Example usage:" -ForegroundColor Green
    Write-Host "  Invoke-psake .\build.psake.ps1 All"
}

Task Init {
    Write-Host "Initializing development environment setup..." -ForegroundColor Cyan
    
    # Check if running as admin
    $isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")
    if (-not $isAdmin) {
        Write-Warning "This script requires administrator privileges for some steps. Consider rerunning as administrator."
    }
    
    # Create development libraries directory if it doesn't exist
    if (-not (Test-Path $devLibsDir)) {
        Write-Host "Creating development libraries directory at $devLibsDir" -ForegroundColor Yellow
        New-Item -Path $devLibsDir -ItemType Directory -Force | Out-Null
    }
    
    # Check for Visual Studio
    if (-not $vsPath) {
        Write-Warning "Visual Studio not found. Please install Visual Studio $vsVersion with C++ workload."
    } else {
        Write-Host "Visual Studio found at: $vsPath" -ForegroundColor Green
    }
}

Task InstallChoco -Depends Init {
    # Check if Chocolatey is installed
    if (-not (Get-Command choco -ErrorAction SilentlyContinue)) {
        Write-Host "Installing Chocolatey..." -ForegroundColor Yellow
        Set-ExecutionPolicy Bypass -Scope Process -Force
        [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
        Invoke-Expression ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
        
        # Refresh environment variables
        $env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
    } else {
        Write-Host "Chocolatey is already installed." -ForegroundColor Green
    }
}

Task InstallTools -Depends InstallChoco {
    Write-Host "Installing development tools..." -ForegroundColor Cyan
    
    # Install basic development tools
    choco install -y git 7zip.install
    
    # Install psake if not already installed
    if (-not (Get-Module -ListAvailable -Name psake)) {
        Write-Host "Installing psake PowerShell module..." -ForegroundColor Yellow
        Install-Module -Name psake -Scope CurrentUser -Force
    }
    
    # Install Python (useful for some scripts and tools)
    choco install -y python

    # Refresh environment variables
    refreshenv
}

Task InstallOpenCV -Depends Init {
    Write-Host "Setting up OpenCV..." -ForegroundColor Cyan
    
    # For this version, we'll use the prebuilt OpenCV from Chocolatey
    # which is more reliable than trying to extract the self-extracting exe
    
    if (Test-Path "$opencvDir\build") {
        Write-Host "OpenCV is already installed at $opencvDir" -ForegroundColor Green
        return
    }
    
    # Create OpenCV directory if it doesn't exist
    if (-not (Test-Path $opencvDir)) {
        New-Item -Path $opencvDir -ItemType Directory -Force | Out-Null
    }
    
    # Install OpenCV using Chocolatey
    Write-Host "Installing OpenCV using Chocolatey..." -ForegroundColor Yellow
    choco install -y opencv
    
    # Copy OpenCV files from Chocolatey installation to our directory
    $chocoOpenCVDir = "$env:ChocolateyInstall\lib\opencv"
    
    if (Test-Path $chocoOpenCVDir) {
        Write-Host "Copying OpenCV files to $opencvDir..." -ForegroundColor Yellow
        
        # Create build directory structure
        New-Item -Path "$opencvDir\build\include" -ItemType Directory -Force | Out-Null
        New-Item -Path "$opencvDir\build\x64\vc16\bin" -ItemType Directory -Force | Out-Null
        New-Item -Path "$opencvDir\build\x64\vc16\lib" -ItemType Directory -Force | Out-Null
        
        # Copy include files
        if (Test-Path "$chocoOpenCVDir\build\include") {
            Copy-Item -Path "$chocoOpenCVDir\build\include\*" -Destination "$opencvDir\build\include" -Recurse -Force
        }
        
        # Copy bin files
        if (Test-Path "$chocoOpenCVDir\build\x64\vc16\bin") {
            Copy-Item -Path "$chocoOpenCVDir\build\x64\vc16\bin\*" -Destination "$opencvDir\build\x64\vc16\bin" -Recurse -Force
        }
        
        # Copy lib files
        if (Test-Path "$chocoOpenCVDir\build\x64\vc16\lib") {
            Copy-Item -Path "$chocoOpenCVDir\build\x64\vc16\lib\*" -Destination "$opencvDir\build\x64\vc16\lib" -Recurse -Force
        }
    } else {
        Write-Warning "Could not find OpenCV installation from Chocolatey. You may need to install it manually."
        
        # Fallback to direct download method
        Write-Host "Trying alternative installation method..." -ForegroundColor Yellow
        
        # We'll use a simpler approach - downloading a pre-built zip file
        $opencvZipUrl = "https://github.com/opencv/opencv/releases/download/4.8.0/opencv-4.8.0-windows.exe"
        $opencvZip = "$devLibsDir\opencv-4.8.0-windows.exe"
        
        Write-Host "Downloading OpenCV 4.8.0..." -ForegroundColor Yellow
        Invoke-WebRequest -Uri $opencvZipUrl -OutFile $opencvZip -UseBasicParsing
        
        Write-Host "Please extract the OpenCV package manually using the self-extracting exe at: $opencvZip" -ForegroundColor Yellow
        Write-Host "After extraction, copy the files to: $opencvDir\build" -ForegroundColor Yellow
    }
      # Add OpenCV bin directory to PATH if it exists
    $opencvBinPath = "$opencvDir\build\x64\vc*\bin"
    $currentPath = [Environment]::GetEnvironmentVariable("Path", "User")
    
    if (Test-Path -Path "$opencvDir\build\x64") {
        if (-not $currentPath.Contains($opencvBinPath)) {
            $resolvedBinPath = (Get-ChildItem -Path $opencvBinPath -ErrorAction SilentlyContinue).FullName
            if ($resolvedBinPath) {
                [Environment]::SetEnvironmentVariable("Path", "$currentPath;$resolvedBinPath", "User")
                Write-Host "Added OpenCV bin directory to PATH" -ForegroundColor Green
            }
        }
    } else {
        Write-Host "OpenCV bin directory not found. You may need to set the PATH manually." -ForegroundColor Yellow
    }
    
    Write-Host "OpenCV $opencvVersion has been installed to $opencvDir" -ForegroundColor Green
}

Task InstallBoost -Depends Init {
    Write-Host "Setting up Boost libraries..." -ForegroundColor Cyan
    
    # Let's try using Chocolatey to install Boost since it's more reliable
    if (Test-Path "$boostDir\stage\lib") {
        Write-Host "Boost is already installed at $boostDir" -ForegroundColor Green
        return
    }

    # Try to use Chocolatey first (simpler and more reliable)
    Write-Host "Attempting to install Boost using Chocolatey..." -ForegroundColor Yellow
    try {
        # Install boost via chocolatey
        choco install -y boost-msvc-14.3
        
        # Check if Chocolatey installed boost successfully
        $chocoBoostDir = "$env:ChocolateyInstall\lib\boost-msvc-14.3"
        if (Test-Path $chocoBoostDir) {
            # Create our own boost directory structure
            if (-not (Test-Path $boostDir)) {
                New-Item -Path $boostDir -ItemType Directory -Force | Out-Null
            }
            
            # Copy files from Chocolatey location to our boost directory
            Write-Host "Copying Boost files to $boostDir..." -ForegroundColor Yellow
            Copy-Item -Path "$chocoBoostDir\*" -Destination "$boostDir" -Recurse -Force
            Write-Host "Boost installed successfully via Chocolatey" -ForegroundColor Green
            return
        }
    }
    catch {
        Write-Warning "Chocolatey installation failed, falling back to manual download."
    }
    
    # Fall back to manual download method
    $boostZip = "$devLibsDir\boost_$boostVersionUnderscore.zip"
    
    # Download Boost
    Write-Host "Downloading Boost $boostVersion..." -ForegroundColor Yellow
    try {
        # Direct download link to a known version of Boost
        $boostUrl = "https://boostorg.jfrog.io/artifactory/main/release/$boostVersion/source/boost_$boostVersionUnderscore.zip"
        Invoke-WebRequest -Uri $boostUrl -OutFile $boostZip -UseBasicParsing
        
        # Verify the download
        if (-not (Test-Path $boostZip) -or (Get-Item $boostZip).Length -lt 1MB) {
            Write-Error "Downloaded Boost file appears to be invalid or corrupt"
            return
        }
        
        Write-Host "Boost downloaded successfully" -ForegroundColor Green
    } catch {
        Write-Error "Failed to download Boost. Error: $_"
        return
    }
    
    # Extract Boost
    Write-Host "Extracting Boost..." -ForegroundColor Yellow
    try {
        # Use 7-Zip to extract since it's more reliable than Expand-Archive for large files
        $7zipPath = "$env:ProgramFiles\7-Zip\7z.exe"
        
        if (Test-Path $7zipPath) {
            Write-Host "Using 7-Zip to extract boost..." -ForegroundColor Yellow
            # Create the parent directory for extraction
            if (-not (Test-Path $devLibsDir)) {
                New-Item -Path $devLibsDir -ItemType Directory -Force | Out-Null
            }
            
            & $7zipPath x $boostZip -o"$devLibsDir" -y
            
            if (-not $?) {
                throw "7-Zip extraction failed"
            }
        } else {
            Write-Host "7-Zip not found, using PowerShell's Expand-Archive..." -ForegroundColor Yellow
            Expand-Archive -Path $boostZip -DestinationPath $devLibsDir -Force -ErrorAction Stop
        }
        
        # Verify extraction
        if (-not (Test-Path $boostDir)) {
            throw "Boost extraction failed, directory not found: $boostDir"
        }
        
        Write-Host "Boost extracted successfully" -ForegroundColor Green
    } catch {
        Write-Error "Failed to extract Boost. Error: $_"
        return
    }
      # Check if we need to build Boost
    if (-not (Test-Path "$boostDir\stage\lib")) {
        # Build Boost (for a minimal required set of libraries by PiTrac)
        Write-Host "Building Boost libraries..." -ForegroundColor Yellow
        
        try {
            # Check if bootstrap.bat exists
            if (-not (Test-Path "$boostDir\bootstrap.bat")) {
                Write-Error "Bootstrap script not found at $boostDir\bootstrap.bat"
                return
            }
            
            # Navigate to the boost directory
            Push-Location $boostDir
            
            # Run bootstrap
            Write-Host "Running bootstrap..." -ForegroundColor Yellow
            $bootstrapProcess = Start-Process -FilePath "cmd.exe" -ArgumentList "/c bootstrap.bat" -WorkingDirectory $boostDir -Wait -NoNewWindow -PassThru
            
            if ($bootstrapProcess.ExitCode -ne 0) {
                throw "Bootstrap failed with exit code: $($bootstrapProcess.ExitCode)"
            }
            
            # Check if b2.exe was created
            if (-not (Test-Path "$boostDir\b2.exe")) {
                throw "b2.exe not found after bootstrap"
            }
            
            # Build specific libraries needed by PiTrac
            Write-Host "Building required Boost libraries..." -ForegroundColor Yellow
            $boostLibs = "filesystem,system,timer,regex,program_options,thread,date_time,chrono"
            
            # Find Visual Studio toolset version
            $toolset = "msvc-14.3" # Default
            if ($vcVersion) {
                $toolset = "msvc-$vcVersion"
            }
            
            $b2Process = Start-Process -FilePath "$boostDir\b2.exe" -ArgumentList "toolset=$toolset --with-$boostLibs link=shared threading=multi runtime-link=shared address-model=64 -j4 --build-type=complete stage" -WorkingDirectory $boostDir -Wait -NoNewWindow -PassThru
            
            if ($b2Process.ExitCode -ne 0) {
                throw "b2 build failed with exit code: $($b2Process.ExitCode)"
            }
            
            Pop-Location
            
            Write-Host "Boost $boostVersion has been built successfully" -ForegroundColor Green
        }
        catch {
            Pop-Location -ErrorAction SilentlyContinue
            Write-Error "Failed to build Boost. Error: $_"
            return
        }
    }
    else {
        Write-Host "Boost libraries already built at $boostDir\stage\lib" -ForegroundColor Green
    }
    
    Write-Host "Boost $boostVersion is ready at $boostDir" -ForegroundColor Green
}

Task ConfigureVS -Depends Init, InstallOpenCV, InstallBoost {
    Write-Host "Configuring Visual Studio project for PiTrac..." -ForegroundColor Cyan
    
    # Ensure OpenCV is installed
    if (-not (Test-Path "$opencvDir\build")) {
        Write-Warning "OpenCV is not properly installed. Please run the InstallOpenCV task first."
        return
    }
    
    # For Boost, check either the standard directory structure or the Chocolatey installation
    $boostInstalled = (Test-Path "$boostDir\stage\lib") -or (Test-Path "$boostDir\lib") -or (Test-Path "$env:ChocolateyInstall\lib\boost-msvc-14.3")
    
    if (-not $boostInstalled) {
        Write-Warning "Boost is not properly installed. Please run the InstallBoost task first."
        return
    }
    
    # Find the solution file
    if (-not (Test-Path $solutionFile)) {
        Write-Warning "Solution file not found at: $solutionFile"
        return
    }
      # Determine the correct paths for Boost
    $boostIncludePath = $boostDir
    $boostLibPath = "$boostDir\stage\lib"
    
    # Check if we're using the Chocolatey installation
    if (Test-Path "$env:ChocolateyInstall\lib\boost-msvc-14.3") {
        $chocoBoostDir = "$env:ChocolateyInstall\lib\boost-msvc-14.3"
        if (Test-Path "$chocoBoostDir\include") {
            $boostIncludePath = "$chocoBoostDir\include"
        }
        if (Test-Path "$chocoBoostDir\lib") {
            $boostLibPath = "$chocoBoostDir\lib"
        }
    }
    
    # Determine the correct vc version for OpenCV
    $opencvVcVersion = "vc16"  # Default
    $vcFolders = Get-ChildItem -Path "$opencvDir\build\x64" -Directory -ErrorAction SilentlyContinue
    if ($vcFolders) {
        $opencvVcVersion = $vcFolders[0].Name
    }
    
    Write-Host "To configure Visual Studio projects:" -ForegroundColor Yellow
    Write-Host "1. Open the solution in Visual Studio: $solutionFile" -ForegroundColor White
    Write-Host "2. For each project, right-click and select Properties" -ForegroundColor White
    Write-Host "3. Set the following values:" -ForegroundColor White
    Write-Host "   Include Directories:" -ForegroundColor Cyan
    Write-Host "   $opencvDir\build\include;$boostIncludePath" -ForegroundColor White
    Write-Host "   Library Directories:" -ForegroundColor Cyan
    Write-Host "   $opencvDir\build\x64\$opencvVcVersion\lib;$boostLibPath" -ForegroundColor White
    Write-Host "   Additional Dependencies (example):" -ForegroundColor Cyan
    
    # Find the actual OpenCV lib files
    $opencvLibs = Get-ChildItem -Path "$opencvDir\build\x64\$opencvVcVersion\lib\*.lib" -ErrorAction SilentlyContinue | Select-Object -First 3 | ForEach-Object { $_.Name } 
    if ($opencvLibs) {
        foreach ($lib in $opencvLibs) {
            Write-Host "   $opencvDir\build\x64\$opencvVcVersion\lib\$lib" -ForegroundColor White
        }
    } else {
        Write-Host "   $opencvDir\build\x64\$opencvVcVersion\lib\opencv_world410.lib (this is an example, check actual files)" -ForegroundColor White
    }
    
    Write-Host "   Preprocessor Definitions:" -ForegroundColor Cyan
    Write-Host "   BOOST_BIND_GLOBAL_PLACEHOLDERS;BOOST_ALL_DYN_LINK;BOOST_USE_WINAPI_VERSION=0x0A00" -ForegroundColor White
    
    Write-Host "For runtime, add the following to your PATH environment variable:" -ForegroundColor Yellow
    Write-Host "$opencvDir\build\x64\$opencvVcVersion\bin;$boostLibPath" -ForegroundColor White
    
    # Future enhancement: automatically update project files using MSBuild or other methods
}

# Composite tasks
Task All -Depends Init, InstallChoco, InstallTools, InstallOpenCV, InstallBoost, ConfigureVS {
    Write-Host "PiTrac development environment setup complete!" -ForegroundColor Green
    Write-Host "You should now be able to build PiTrac using Visual Studio." -ForegroundColor Cyan
}
