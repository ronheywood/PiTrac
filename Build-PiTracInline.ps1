# Build-PiTracInline.ps1
# PowerShell script to build PiTrac solution by initializing the VS environment in the current shell
# This script allows MSBuild to be used directly without launching a separate window

param (
    [string]$Configuration = "Release",
    [string]$Platform = "x64",
    [switch]$Rebuild = $false,
    [switch]$Clean = $false
)

# Display header
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host "      PiTrac Inline MSBuild Interface        " -ForegroundColor Cyan
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

# Define solution paths
$baseDir = Resolve-Path .
$solutionDir = "$baseDir\Software\LMSourceCode"
$solutionFile = "$solutionDir\GolfSim.sln"

# Verify solution exists
if (-not (Test-Path $solutionFile)) {
    Write-Host "Solution file not found at: $solutionFile" -ForegroundColor Red
    exit 1
}

# Method 1: Try using VsDevCmd.bat (most reliable)
$vcvarsPath = Join-Path -Path $vsInstallationPath -ChildPath "VC\Auxiliary\Build\vcvarsall.bat"
if (Test-Path $vcvarsPath) {
    Write-Host "Initializing Visual Studio environment using vcvarsall.bat..." -ForegroundColor Yellow
    
    # Create a temporary file to capture environment variables
    $tempFile = [IO.Path]::GetTempFileName()
    
    # Call vcvarsall.bat and capture environment variables
    $archParam = $Platform.ToLower()
    if ($archParam -eq "x64") { $archParam = "amd64" } # vcvarsall uses "amd64" instead of "x64"
    
    Write-Host "Running: cmd /c `"$vcvarsPath`" $archParam && set > `"$tempFile`"" -ForegroundColor DarkGray
    cmd /c "`"$vcvarsPath`" $archParam && set > `"$tempFile`""
    
    # Import environment variables into current PowerShell session
    Get-Content $tempFile | ForEach-Object {
        if ($_ -match "^(.*?)=(.*)$") {
            Set-Item -Path "env:$($matches[1])" -Value $matches[2]
        }
    }
    
    Remove-Item $tempFile
    
    # Check if CL.exe is now in the path
    $clPath = Get-Command cl.exe -ErrorAction SilentlyContinue
    if ($clPath) {
        Write-Host "Visual Studio C++ environment initialized successfully!" -ForegroundColor Green
        Write-Host "C++ compiler found at: $($clPath.Source)" -ForegroundColor Green
    } else {
        Write-Warning "Failed to find C++ compiler (cl.exe) after environment initialization."
        Write-Host "Please ensure the 'Desktop development with C++' workload is installed." -ForegroundColor Yellow
        exit 1
    }
} else {
    # Method 2: Try using the PowerShell module method
    $launchVsDevShell = Join-Path -Path $vsInstallationPath -ChildPath "Common7\Tools\Launch-VsDevShell.ps1"
    if (Test-Path $launchVsDevShell) {
        Write-Host "Initializing Visual Studio environment using PowerShell module..." -ForegroundColor Yellow
        try {
            # Import Visual Studio environment into current shell
            & $launchVsDevShell -Arch $Platform -SkipAutomaticLocation
            
            # Check if it worked
            $clPath = Get-Command cl.exe -ErrorAction SilentlyContinue
            if ($clPath) {
                Write-Host "Visual Studio C++ environment initialized successfully!" -ForegroundColor Green
                Write-Host "C++ compiler found at: $($clPath.Source)" -ForegroundColor Green
            } else {
                Write-Warning "Failed to find C++ compiler (cl.exe) after environment initialization."
                Write-Host "Please ensure the 'Desktop development with C++' workload is installed." -ForegroundColor Yellow
                exit 1
            }
        } catch {
            Write-Host "Failed to initialize Visual Studio environment: $_" -ForegroundColor Red
            exit 1
        }
    } else {
        Write-Host "Could not find Visual Studio environment initialization tools." -ForegroundColor Red
        Write-Host "Please ensure Visual Studio is properly installed." -ForegroundColor Yellow
        exit 1
    }
}

# Check dependencies
Write-Host "`nChecking dependencies..." -ForegroundColor Yellow

$devLibsDir = "$env:USERPROFILE\DevLibs"
$opencvDir = "$devLibsDir\opencv-4.10.0"
$boostDir = "$devLibsDir\boost_1_83_0"

$opencvInstalled = Test-Path "$opencvDir\build"
$boostInstalled = (Test-Path "$boostDir\stage\lib") -or (Test-Path "$boostDir\lib") -or (Test-Path "$env:ChocolateyInstall\lib\boost-msvc-14.3")

if (-not $opencvInstalled) {
    Write-Host "❌ OpenCV not found at $opencvDir\build" -ForegroundColor Red
    Write-Host "Please run the OpenCV installation task first:" -ForegroundColor Yellow
    Write-Host ".\Setup-PiTracDev.ps1 -Task InstallOpenCV" -ForegroundColor White
    $continue = Read-Host "Continue anyway? (Y/N)"
    if ($continue -ne "Y" -and $continue -ne "y") {
        exit 0
    }
} else {
    Write-Host "✅ OpenCV found at $opencvDir\build" -ForegroundColor Green
}

if (-not $boostInstalled) {
    Write-Host "❌ Boost not found" -ForegroundColor Red
    Write-Host "Please run the Boost installation task first:" -ForegroundColor Yellow
    Write-Host ".\Setup-PiTracDev.ps1 -Task InstallBoost" -ForegroundColor White
    $continue = Read-Host "Continue anyway? (Y/N)"
    if ($continue -ne "Y" -and $continue -ne "y") {
        exit 0
    }
} else {
    Write-Host "✅ Boost found" -ForegroundColor Green
}

# Determine the right boost path
$boostIncludePath = $boostDir
$boostLibPath = "$boostDir\stage\lib"

if (Test-Path "$env:ChocolateyInstall\lib\boost-msvc-14.3") {
    $chocoBoostDir = "$env:ChocolateyInstall\lib\boost-msvc-14.3"
    if (Test-Path "$chocoBoostDir\include") {
        $boostIncludePath = "$chocoBoostDir\include"
    }
    if (Test-Path "$chocoBoostDir\lib") {
        $boostLibPath = "$chocoBoostDir\lib"
    }
}

# Determine OpenCV version and path
$opencvVcVersion = "vc16"  # Default
$vcFolders = Get-ChildItem -Path "$opencvDir\build\x64" -Directory -ErrorAction SilentlyContinue
if ($vcFolders) {
    $opencvVcVersion = $vcFolders[0].Name
}

# Set up include and library paths
$includeDirectories = "$opencvDir\build\include;$boostIncludePath"
$libraryDirectories = "$opencvDir\build\x64\$opencvVcVersion\lib;$boostLibPath"

# Find OpenCV libraries
$opencvLibPattern = "$opencvDir\build\x64\$opencvVcVersion\lib\opencv_world*.lib"
$opencvLibs = Get-ChildItem -Path $opencvLibPattern -ErrorAction SilentlyContinue | Select-Object -First 1 -ExpandProperty Name

if ($opencvLibs) {
    Write-Host "Found OpenCV library: $opencvLibs" -ForegroundColor Green
    $additionalDependencies = $opencvLibs
} else {
    Write-Host "No OpenCV library found matching pattern: $opencvLibPattern" -ForegroundColor Red
    $additionalDependencies = "opencv_world410.lib" # Default value, may need adjustment
}

# Set build action
if ($Clean) {
    $action = "Clean"
    Write-Host "`nCleaning PiTrac solution ($Configuration|$Platform)..." -ForegroundColor Yellow
} elseif ($Rebuild) {
    $action = "Rebuild"
    Write-Host "`nRebuilding PiTrac solution ($Configuration|$Platform)..." -ForegroundColor Yellow
} else {
    $action = "Build"
    Write-Host "`nBuilding PiTrac solution ($Configuration|$Platform)..." -ForegroundColor Yellow
}

# Set up MSBuild arguments
$msbuildArgs = @(
    $solutionFile,
    "/t:$action",
    "/p:Configuration=$Configuration",
    "/p:Platform=$Platform",
    "/p:IncludePath=`"$includeDirectories`"",
    "/p:LibraryPath=`"$libraryDirectories`"",
    "/p:AdditionalDependencies=`"$additionalDependencies`"",
    "/p:PreprocessorDefinitions=`"BOOST_BIND_GLOBAL_PLACEHOLDERS;BOOST_ALL_DYN_LINK;BOOST_USE_WINAPI_VERSION=0x0A00`"",
    "/p:RuntimeLibrary=`"MultiThreadedDLL`"",
    "/m",      # Use multiple cores
    "/verbosity:normal"  # Show more detailed output
)

# Find MSBuild.exe
$msbuildPath = Get-Command MSBuild.exe -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Source
if (-not $msbuildPath) {
    Write-Warning "MSBuild.exe not found in PATH. Trying to find it in Visual Studio directory..."
    $msbuildPath = Get-ChildItem -Path $vsInstallationPath -Filter MSBuild.exe -Recurse -ErrorAction SilentlyContinue | 
        Where-Object { $_.Directory.FullName -like "*\MSBuild\*\Bin" } | 
        Select-Object -First 1 -ExpandProperty FullName
    
    if (-not $msbuildPath) {
        Write-Host "Could not find MSBuild.exe" -ForegroundColor Red
        exit 1
    }
}

# Execute MSBuild
try {
    # For easier readability in the console
    Write-Host "Running MSBuild..." -ForegroundColor Yellow
    Write-Host "Command: $msbuildPath $($msbuildArgs -join ' ')" -ForegroundColor DarkGray
    Write-Host ""
    
    # Run MSBuild
    & $msbuildPath $msbuildArgs
    
    # Check the result
    if ($LASTEXITCODE -eq 0) {
        Write-Host "`nBuild completed successfully" -ForegroundColor Green
        
        # Check for executable
        $executablePath = "$solutionDir\ImageProcessing\output\ImageProcessing.exe"
        if (Test-Path $executablePath) {
            Write-Host "PiTrac executable created at: $executablePath" -ForegroundColor Green
        } else {
            Write-Warning "Expected executable not found at: $executablePath"
            Write-Warning "The build may have succeeded but produced output in a different location."
        }
    } else {
        Write-Host "`nBuild failed with exit code: $LASTEXITCODE" -ForegroundColor Red
        exit $LASTEXITCODE
    }
} catch {
    Write-Host "Error during build: $_" -ForegroundColor Red
    exit 1
}

Write-Host "`nTo run PiTrac, use the Run-PiTrac.ps1 script:" -ForegroundColor Yellow
Write-Host "  .\Run-PiTrac.ps1" -ForegroundColor White
