# PiTrac Windows Development Guide

This guide explains how to set up PiTrac for development and debugging on Windows machines. Unlike the primary Raspberry Pi deployment, this setup allows developers to work on the codebase using Visual Studio and debug issues without requiring physical hardware.

## Prerequisites

- Windows 10/11
- PowerShell 5.1 or later
- Administrator privileges (for installing some dependencies)
- Visual Studio 2022 with C++ workload installed

## Quick Start

1. Open PowerShell as Administrator
2. Navigate to the PiTrac repository folder
3. Run the setup script:

```powershell
.\Setup-PiTracDev.ps1 -Task All
```

This will install all required dependencies including:
- Chocolatey (Windows package manager)
- Git and 7-Zip
- OpenCV libraries
- Boost C++ libraries

## Manual Setup

If you prefer to install components manually or if the automated setup encounters issues, you can run specific tasks:

```powershell
# Get help and list all available tasks
.\Setup-PiTracDev.ps1 -Task Help

# Install only OpenCV
.\Setup-PiTracDev.ps1 -Task InstallOpenCV

# Install only Boost libraries
.\Setup-PiTracDev.ps1 -Task InstallBoost
```

## Visual Studio Project Configuration

After installing the dependencies, you'll need to configure the Visual Studio project. The script provides instructions, but you'll need to manually:

1. Open `Software\LMSourceCode\ImageProcessing\ImageProcessing.sln` in Visual Studio
2. For each project, set the appropriate:
   - Include directories
   - Library directories
   - Additional dependencies
   - Preprocessor definitions

## Running PiTrac in Debug Mode

For debugging purposes, you can set up command-line parameters. Here's an example configuration:

```
--show_images 1 --base_image_logging_dir C:\Path\To\Images --logging_level debug --artifact_save_level=all --wait_keys 1 --system_mode pi1_camera1 --test_image_filename <path-to-test-image> --search_center_x 800 --search_center_y 550
```

## Troubleshooting

### Visual Studio Issues
- Ensure that all required C++ components are installed with Visual Studio
- Verify library paths match the versions installed by the setup script

### Linking Issues
- Make sure you're using consistent library versions (debug vs. release)
- Check that the PATH environment variable includes paths to required DLLs

### Boost Build Failures
- If Boost build fails, you may need to manually build it following these steps:
  1. Navigate to the boost directory
  2. Run `bootstrap.bat`
  3. Run `b2 toolset=msvc-14.3 --with-filesystem,system,timer,regex,program_options,thread,date_time,chrono link=shared threading=multi runtime-link=shared address-model=64 -j4`

## Windows vs. Raspberry Pi Differences

Note that the Windows development environment:
- Uses pre-captured images instead of live camera input
- Doesn't require hardware components (LED strobes, etc.)
- May behave slightly differently due to platform-specific code

All core algorithms should work identically between platforms, but hardware-specific features will only function on the Raspberry Pi.

## Additional Resources

- [PiTrac - Debugging and Code Walk-Throughs](https://github.com/jamespilgrim/PiTrac/blob/main/Documentation/PiTrac%20-%20Debugging%20and%20Code%20Walk-Throughs.md)
- [OpenCV Documentation](https://docs.opencv.org)
- [Boost Documentation](https://www.boost.org/doc/libs)
