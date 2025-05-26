# simple_test_runner.ps1
# A simplified test runner that compiles and executes Boost tests for PiTrac

# Get script location
$scriptDir = $PSScriptRoot
$tempDir = Join-Path $scriptDir "temp_boost_test"
$testExe = Join-Path $tempDir "simple_tests.exe"
$vcvarsall = "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat"

# Set up environment variables
$env:OPENCV_DIR = [Environment]::GetEnvironmentVariable("OPENCV_DIR", "Machine")
$env:BOOST_ROOT = [Environment]::GetEnvironmentVariable("BOOST_ROOT", "Machine")

Write-Host "Environment setup:" -ForegroundColor Cyan
Write-Host "OPENCV_DIR: $env:OPENCV_DIR" 
Write-Host "BOOST_ROOT: $env:BOOST_ROOT"

# Add paths to PATH
$env:Path += ";$env:OPENCV_DIR\x64\vc16\bin"
$env:Path += ";$env:BOOST_ROOT\lib64-msvc-14.3"

# Compile the test using the Visual Studio compiler
Write-Host "Compiling simple test..." -ForegroundColor Cyan

# Find cl.exe
$clExe = $null
$paths = $env:Path -split ';'
foreach ($path in $paths) {
    $testPath = Join-Path $path "cl.exe"
    if (Test-Path $testPath) {
        $clExe = $testPath
        break
    }
}

if (Test-Path $vcvarsall) {
    Write-Host "Found vcvarsall.bat at: $vcvarsall" -ForegroundColor Cyan
            
    # Create a batch file to run the compilation
    $batchFile = Join-Path $tempDir "compile.cmd"
    @"
@echo off
call "$vcvarsall" x64
cd /d $tempDir
cl.exe /EHsc /std:c++17 /I"$env:OPENCV_DIR\include" /I"$env:BOOST_ROOT" simple_tests.cpp /link /LIBPATH:"$env:OPENCV_DIR\x64\vc16\lib" opencv_world470d.lib /out:simple_tests.exe
"@ | Out-File -FilePath $batchFile -Encoding ascii
    
    # Run the batch file
    $compileResult = Start-Process -FilePath "cmd.exe" -ArgumentList "/c `"$batchFile`"" -Wait -PassThru -NoNewWindow
    
    if ($compileResult.ExitCode -ne 0) {
        Write-Host "Compilation failed with exit code $($compileResult.ExitCode)" -ForegroundColor Red
        exit 1
    }
} else {
    Write-Host "Could not find vcvarsall.bat at: $vcvarsall" -ForegroundColor Red
    exit 1
}
} else {
    Write-Host "Found cl.exe at: $clExe" -ForegroundColor Cyan
    
    # Use cl.exe directly
    Push-Location $tempDir
    try {
        $compileArgs = "/EHsc", "/std:c++17", "/I`"$env:OPENCV_DIR\include`"", "/I`"$env:BOOST_ROOT`"", "simple_tests.cpp", "/link", "/LIBPATH:`"$env:OPENCV_DIR\x64\vc16\lib`"", "opencv_world470d.lib", "/LIBPATH:`"$env:BOOST_ROOT\lib64-msvc-14.3`"", "/out:simple_tests.exe"
        
        & $clExe $compileArgs
        
        if ($LASTEXITCODE -ne 0) {
            Write-Host "Compilation failed with exit code $LASTEXITCODE" -ForegroundColor Red
            exit 1
        }
    } finally {
        Pop-Location
    }
}

# Run the test if it was built successfully
if (Test-Path $testExe) {
    Write-Host "Running test..." -ForegroundColor Cyan
    Push-Location $tempDir
    try {
        & $testExe --log_level=test_suite
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "Tests completed successfully!" -ForegroundColor Green
        } else {
            Write-Host "Tests failed with exit code $LASTEXITCODE" -ForegroundColor Red
            exit $LASTEXITCODE
        }
    } finally {
        Pop-Location
    }
} else {
    Write-Host "Test executable not found at $testExe" -ForegroundColor Red
    exit 1
}

exit 0
