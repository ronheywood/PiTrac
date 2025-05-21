##############################################################################
# PiTrac Windows Run Script
# Automated run script using psake
#
# Usage:
# Import-Module psake
# Invoke-psake .\run.psake.ps1
##############################################################################

Properties {
    $baseDir = Resolve-Path .
    $devLibsDir = "$env:USERPROFILE\DevLibs"
    $opencvVersion = "4.10.0"
    $boostVersion = "1.83.0"
    
    # Directories
    $opencvDir = "$devLibsDir\opencv-$opencvVersion"
    $boostDir = "$devLibsDir\boost_$($boostVersion -replace '\.', '_')"
    
    # Solution and executable paths
    $solutionDir = "$baseDir\Software\LMSourceCode"
    $executableDir = "$solutionDir\ImageProcessing\output"
    $executableName = "ImageProcessing.exe"
    $executablePath = "$executableDir\$executableName"
    
    # Test image directory
    $testImagesDir = "$solutionDir\ImageProcessing\images"
    $defaultTestImage = Get-ChildItem -Path $testImagesDir -Filter "*.jpg" -File | Select-Object -First 1 -ExpandProperty FullName
    
    # Settings for PiTrac execution
    $defaultParameters = @{
        ShowImages = 1
        BaseImageLoggingDir = "$baseDir\logs"
        LoggingLevel = "debug"
        ArtifactSaveLevel = "all"
        WaitKeys = 1
        SystemMode = "pi1_camera1"
        TestImageFilename = $defaultTestImage
        SearchCenterX = 800
        SearchCenterY = 550
    }
}

Task Default -Depends Help

Task Help {
    Write-Host "PiTrac Windows Run Script" -ForegroundColor Cyan
    Write-Host "Available tasks:" -ForegroundColor Yellow
    Write-Host "  Run                 : Runs PiTrac with default parameters"
    Write-Host "  RunWithImage        : Runs PiTrac with specified test image"
    Write-Host "  RunWithDebugOptions : Runs PiTrac with custom debug options"
    Write-Host "  ListTestImages      : Lists available test images"
    Write-Host "  CleanLogs           : Cleans log files"
    Write-Host ""
    Write-Host "Example usage:" -ForegroundColor Green
    Write-Host "  Invoke-psake .\run.psake.ps1 Run"
    Write-Host "  Invoke-psake .\run.psake.ps1 RunWithImage -properties @{TestImage='ball_01.jpg'}"
}

Task ValidateEnvironment {
    # Check if executable exists
    if (-not (Test-Path $executablePath)) {
        Write-Warning "PiTrac executable not found at: $executablePath"
        Write-Host "Please build the project first using Visual Studio." -ForegroundColor Yellow
        throw "PiTrac executable not found."
    }
    
    # Check if OpenCV binaries are in PATH
    $currentPath = [Environment]::GetEnvironmentVariable("Path", "User") + ";" + [Environment]::GetEnvironmentVariable("Path", "Machine")
    $opencvBinPath = "$opencvDir\build\x64\vc*\bin"
    
    $resolvedBinPath = (Get-ChildItem -Path $opencvBinPath -ErrorAction SilentlyContinue).FullName
    if ($resolvedBinPath -and (-not ($currentPath -like "*$resolvedBinPath*"))) {
        Write-Warning "OpenCV bin directory not found in PATH. Adding it temporarily for this session."
        $env:Path = "$env:Path;$resolvedBinPath"
    }
    
    # Check if Boost binaries are in PATH
    $boostBinPath = "$boostDir\stage\lib"
    if (Test-Path $boostBinPath -and (-not ($currentPath -like "*$boostBinPath*"))) {
        Write-Warning "Boost bin directory not found in PATH. Adding it temporarily for this session."
        $env:Path = "$env:Path;$boostBinPath"
    }
    
    # Create logs directory if it doesn't exist
    if (-not (Test-Path $defaultParameters.BaseImageLoggingDir)) {
        Write-Host "Creating logs directory at $($defaultParameters.BaseImageLoggingDir)" -ForegroundColor Yellow
        New-Item -Path $defaultParameters.BaseImageLoggingDir -ItemType Directory -Force | Out-Null
    }
}

Task ListTestImages {
    Write-Host "Available test images:" -ForegroundColor Cyan
    
    if (-not (Test-Path $testImagesDir)) {
        Write-Warning "Test images directory not found at: $testImagesDir"
        return
    }
    
    $images = Get-ChildItem -Path $testImagesDir -Filter "*.jpg" -File
    if ($images.Count -eq 0) {
        Write-Host "No test images found in $testImagesDir" -ForegroundColor Yellow
        return
    }
    
    $images | ForEach-Object {
        Write-Host "  $($_.Name)"
    }
    
    Write-Host ""
    Write-Host "To run with a specific image:" -ForegroundColor Yellow
    Write-Host "  Invoke-psake .\run.psake.ps1 RunWithImage -properties @{TestImage='imagename.jpg'}" -ForegroundColor White
}

Task Run -Depends ValidateEnvironment {
    Write-Host "Running PiTrac with default parameters..." -ForegroundColor Cyan
    
    $paramString = ""
    $paramString += "--show_images $($defaultParameters.ShowImages) "
    $paramString += "--base_image_logging_dir `"$($defaultParameters.BaseImageLoggingDir)`" "
    $paramString += "--logging_level $($defaultParameters.LoggingLevel) "
    $paramString += "--artifact_save_level=$($defaultParameters.ArtifactSaveLevel) "
    $paramString += "--wait_keys $($defaultParameters.WaitKeys) "
    $paramString += "--system_mode $($defaultParameters.SystemMode) "
    
    if ($defaultParameters.TestImageFilename -and (Test-Path $defaultParameters.TestImageFilename)) {
        $paramString += "--test_image_filename `"$($defaultParameters.TestImageFilename)`" "
    } else {
        Write-Warning "Default test image not found. Running without test image."
    }
    
    $paramString += "--search_center_x $($defaultParameters.SearchCenterX) "
    $paramString += "--search_center_y $($defaultParameters.SearchCenterY)"
    
    Write-Host "Executing: $executablePath $paramString" -ForegroundColor Yellow
    
    try {
        Push-Location $executableDir
        Start-Process -FilePath $executablePath -ArgumentList $paramString -NoNewWindow -Wait
        Pop-Location
    } catch {
        Pop-Location
        Write-Error "Error running PiTrac: $_"
    }
}

Task RunWithImage -Depends ValidateEnvironment {
    # Check if TestImage parameter was provided
    if (-not $TestImage) {
        Write-Warning "No test image specified. Using default test image."
        Invoke-psake -buildFile $PSScriptRoot\run.psake.ps1 -taskList Run
        return
    }
    
    $testImagePath = Join-Path $testImagesDir $TestImage
    
    if (-not (Test-Path $testImagePath)) {
        Write-Warning "Test image not found at: $testImagePath"
        Write-Host "Available test images:" -ForegroundColor Yellow
        Invoke-psake -buildFile $PSScriptRoot\run.psake.ps1 -taskList ListTestImages
        return
    }
    
    Write-Host "Running PiTrac with test image: $TestImage" -ForegroundColor Cyan
    
    $paramString = ""
    $paramString += "--show_images $($defaultParameters.ShowImages) "
    $paramString += "--base_image_logging_dir `"$($defaultParameters.BaseImageLoggingDir)`" "
    $paramString += "--logging_level $($defaultParameters.LoggingLevel) "
    $paramString += "--artifact_save_level=$($defaultParameters.ArtifactSaveLevel) "
    $paramString += "--wait_keys $($defaultParameters.WaitKeys) "
    $paramString += "--system_mode $($defaultParameters.SystemMode) "
    $paramString += "--test_image_filename `"$testImagePath`" "
    $paramString += "--search_center_x $($defaultParameters.SearchCenterX) "
    $paramString += "--search_center_y $($defaultParameters.SearchCenterY)"
    
    Write-Host "Executing: $executablePath $paramString" -ForegroundColor Yellow
    
    try {
        Push-Location $executableDir
        Start-Process -FilePath $executablePath -ArgumentList $paramString -NoNewWindow -Wait
        Pop-Location
    } catch {
        Pop-Location
        Write-Error "Error running PiTrac: $_"
    }
}

Task RunWithDebugOptions -Depends ValidateEnvironment {
    param(
        [int]$ShowImages = $defaultParameters.ShowImages,
        [string]$LoggingDir = $defaultParameters.BaseImageLoggingDir,
        [string]$LoggingLevel = $defaultParameters.LoggingLevel,
        [string]$ArtifactSaveLevel = $defaultParameters.ArtifactSaveLevel,
        [int]$WaitKeys = $defaultParameters.WaitKeys,
        [string]$SystemMode = $defaultParameters.SystemMode,
        [string]$TestImageFilename = $defaultParameters.TestImageFilename,
        [int]$SearchCenterX = $defaultParameters.SearchCenterX,
        [int]$SearchCenterY = $defaultParameters.SearchCenterY
    )
    
    Write-Host "Running PiTrac with custom debug options..." -ForegroundColor Cyan
    
    $paramString = ""
    $paramString += "--show_images $ShowImages "
    $paramString += "--base_image_logging_dir `"$LoggingDir`" "
    $paramString += "--logging_level $LoggingLevel "
    $paramString += "--artifact_save_level=$ArtifactSaveLevel "
    $paramString += "--wait_keys $WaitKeys "
    $paramString += "--system_mode $SystemMode "
    
    if ($TestImageFilename -and (Test-Path $TestImageFilename)) {
        $paramString += "--test_image_filename `"$TestImageFilename`" "
    } else {
        Write-Warning "Specified test image not found. Running without test image."
    }
    
    $paramString += "--search_center_x $SearchCenterX "
    $paramString += "--search_center_y $SearchCenterY"
    
    Write-Host "Executing: $executablePath $paramString" -ForegroundColor Yellow
    
    try {
        Push-Location $executableDir
        Start-Process -FilePath $executablePath -ArgumentList $paramString -NoNewWindow -Wait
        Pop-Location
    } catch {
        Pop-Location
        Write-Error "Error running PiTrac: $_"
    }
}

Task CleanLogs {
    if (Test-Path $defaultParameters.BaseImageLoggingDir) {
        Write-Host "Cleaning log files in $($defaultParameters.BaseImageLoggingDir)..." -ForegroundColor Cyan
        Get-ChildItem -Path $defaultParameters.BaseImageLoggingDir -File | Remove-Item -Force
        Write-Host "Log files cleaned." -ForegroundColor Green
    } else {
        Write-Host "Log directory does not exist. Nothing to clean." -ForegroundColor Yellow
    }
}
