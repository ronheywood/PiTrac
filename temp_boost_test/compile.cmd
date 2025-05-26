@echo off
setlocal

REM Setup VC environment for x64
call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat" x64

REM Set paths
set OPENCV_DIR=C:\tools\opencv\build
set BOOST_ROOT=C:\Dev_Libs\boost
set SRC_DIR=%~dp0

REM Parse command line arguments
set LOG_LEVEL=--log_level=test_suite
set TEST_FILTER=
:parse_args
if "%~1"=="" goto end_parse_args
if "%~1"=="--log_level=all" set LOG_LEVEL=--log_level=all
if "%~1:~0,11%"=="--run_test=" set TEST_FILTER=%~1
shift
goto parse_args
:end_parse_args

REM Compile the test
echo Compiling test...
echo OPENCV_DIR=%OPENCV_DIR%
cl.exe /EHsc /std:c++17 /I"%OPENCV_DIR%\include" /I"%BOOST_ROOT%" simple_tests.cpp /link /LIBPATH:"%OPENCV_DIR%\x64\vc16\lib" opencv_world4110.lib /out:simple_tests.exe

REM Run if successful
if %ERRORLEVEL% NEQ 0 (
    echo Compilation failed with error code %ERRORLEVEL%
    exit /b %ERRORLEVEL%
)

echo Running tests...
simple_tests.exe %LOG_LEVEL% %TEST_FILTER%

if %ERRORLEVEL% NEQ 0 (
    echo Tests failed with error code %ERRORLEVEL%
) else (
    echo Tests passed!
)

exit /b %ERRORLEVEL%
