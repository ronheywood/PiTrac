# Building PiTrac on Windows

This guide provides step-by-step instructions for building the PiTrac project on Windows using either Visual Studio GUI or command-line tools.

## Prerequisites

Before building PiTrac, ensure that:

1. You have Visual Studio 2019 or 2022 installed with the **Desktop development with C++** workload. 
   If you don't have this workload installed, run the following script as administrator:
   ```powershell
   # Run as administrator
   .\Install-VSCppWorkload.ps1
   ```

2. You have set up the development environment by running:
   ```powershell
   .\Setup-PiTracDev.ps1 -Task All
   ```

3. All dependencies (Chocolatey, OpenCV, Boost) are properly installed.

4. To check if your system meets all prerequisites, run:
   ```powershell
   .\Check-VisualStudioComponents.ps1
   ```

## Troubleshooting Dependencies

If you encounter issues with missing dependencies, you can manually install them:

1. **OpenCV Installation**:
   ```powershell
   .\Setup-PiTracDev.ps1 -Task InstallOpenCV
   ```

2. **Boost Installation**:
   ```powershell
   .\Setup-PiTracDev.ps1 -Task InstallBoost
   ```

3. **Configure Visual Studio Settings**:
   ```powershell
   .\Setup-PiTracDev.ps1 -Task ConfigureVS
   ```

## Building PiTrac

### Option 1: Using the Developer PowerShell Launcher (Most Reliable)

This method automatically launches a Visual Studio Developer PowerShell with the correct environment:

1. Open a regular PowerShell window in the PiTrac directory
2. Run:
   ```powershell
   .\Build-PiTracWithDevShell.ps1
   ```
   
This will open a new PowerShell window with the correct Visual Studio environment and run the build process automatically.

### Option 2: Using Developer PowerShell for Visual Studio (Manual)

If you prefer to manually open the Developer PowerShell:

1. Open the **Developer PowerShell for VS 2022** from the Start menu
2. Navigate to the PiTrac directory:
   ```powershell
   cd c:\kata\PiTrac
   ```
3. Run the command-line build script:
   ```powershell
   .\Build-PiTracCMD.ps1
   ```

This will automatically build the solution in Release mode for x64 platform.

**Additional Build Options:**

- Build in Debug mode:
  ```powershell
  .\Build-PiTracCMD.ps1 -Configuration Debug
  ```

- Clean and rebuild the solution:
  ```powershell
  .\Build-PiTracCMD.ps1 -Rebuild
  ```

- Just clean without building:
  ```powershell
  .\Build-PiTracCMD.ps1 -Clean
  ```

### Option 2: Using Standard PowerShell Scripts

You can use the standard PowerShell scripts, which will try to locate and use Visual Studio's build tools:

1. Open PowerShell in the PiTrac root directory
2. Run the build script:
   ```powershell
   .\Build-PiTrac.ps1
   ```
   
   Or try the Visual Studio batch file specific build script:
   ```powershell
   .\Build-PiTracVS.ps1
   ```

**Additional Build Options:**

- Build in Debug mode:
  ```powershell
  .\Build-PiTrac.ps1 -Configuration Debug
  ```

- Clean and rebuild the solution:
  ```powershell
  .\Build-PiTrac.ps1 -Rebuild
  ```

- Open the solution in Visual Studio:
  ```powershell
  .\Build-PiTrac.ps1 -OpenInVisualStudio
  ```

### Option 3: Using Visual Studio Directly

1. Open the solution file in Visual Studio:
   ```powershell
   # Open with default program (Visual Studio)
   start c:\kata\PiTrac\Software\LMSourceCode\GolfSim.sln
   ```

2. Configure the project settings for each project in the solution:
   - Right-click on the project in Solution Explorer and select **Properties**
   - Go to **C/C++ > General** and add to **Additional Include Directories**:
     - `C:\Users\[username]\DevLibs\opencv-4.10.0\build\include`
     - `C:\Users\[username]\DevLibs\boost_1_83_0`
     
   - Go to **Linker > General** and add to **Additional Library Directories**:
     - `C:\Users\[username]\DevLibs\opencv-4.10.0\build\x64\vc16\lib`
     - `C:\Users\[username]\DevLibs\boost_1_83_0\stage\lib`
     
   - Go to **Linker > Input** and add to **Additional Dependencies**:
     - `opencv_world410.lib` (verify the exact filename in the lib directory)
     
   - Go to **C/C++ > Preprocessor** and add to **Preprocessor Definitions**:
     - `BOOST_BIND_GLOBAL_PLACEHOLDERS`
     - `BOOST_ALL_DYN_LINK`
     - `BOOST_USE_WINAPI_VERSION=0x0A00`

3. Select the desired build configuration (Debug/Release) and platform (x64) from the dropdown menus in Visual Studio.

4. Build the solution by pressing **F7** or selecting **Build > Build Solution** from the menu.

## After Building

1. Ensure the build completed successfully.

2. The executable will be located at:
   ```
   c:\kata\PiTrac\Software\LMSourceCode\ImageProcessing\output\ImageProcessing.exe
   ```

3. Before running the application, ensure the runtime DLLs are in the PATH:
   ```powershell
   $env:PATH += ";C:\Users\[username]\DevLibs\opencv-4.10.0\build\x64\vc16\bin;C:\Users\[username]\DevLibs\boost_1_83_0\stage\lib"
   ```

4. Use the `Run-PiTrac.ps1` script to run the application:
   ```powershell
   .\Run-PiTrac.ps1
   ```

## Troubleshooting

### Common Build Issues

- **Missing C++ Compiler (cl.exe)**: This indicates that Visual Studio is missing the C++ workload:
  ```
  TRACKER : error TRK0005: Failed to locate: "CL.exe". The system cannot find the file specified.
  ```
  Solution: Run the Install-VSCppWorkload.ps1 script as administrator

- **Missing DLLs at Runtime**: If the application fails to start due to missing DLLs, verify that the PATH environment variable includes the directories with OpenCV and Boost DLLs.

- **Include Path Errors**: If you see errors about missing header files, check that the include paths are correctly set in the project properties.

- **Library Path Errors**: If you see linker errors about unresolved externals, check that the library paths and additional dependencies are correctly set.

### Build Environment Issues

- **PowerShell Execution Policy**: If scripts won't run, you may need to adjust the execution policy:
  ```powershell
  Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
  ```

- **Visual Studio Installation**: Ensure you have Visual Studio 2019 or 2022 with the "Desktop development with C++" workload installed.

- **Dependency Paths**: Make sure OpenCV and Boost are installed in the correct locations. If you've installed them in non-standard locations, update the paths in the scripts or project properties.

### Build Tools Path Issues

- **Developer Command Prompt**: For command-line building, it's best to use the "Developer PowerShell for VS 2022" or "Developer Command Prompt for VS 2022" which have all the necessary environment variables set.

- **Manual Environment Setup**: If you need to manually set up the build environment in a regular PowerShell window:
  ```powershell
  & "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\Launch-VsDevShell.ps1" -SkipAutomaticLocation
  ```

For additional help, see the Windows Development guide in the repository:
```
c:\kata\PiTrac\Windows-Development.md
```
