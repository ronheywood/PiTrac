cmake_minimum_required(VERSION 3.10)

# Camera Bounded Context
project(camera_context)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
if(WIN32)
    # Windows Media Foundation is part of Windows SDK
    find_package(PkgConfig QUIET)
endif()

# Find OpenCV for image processing in tests
find_package(OpenCV QUIET)

# Platform-specific source files
set(PLATFORM_SOURCES "")
set(PLATFORM_LIBS "")

if(WIN32)
    set(PLATFORM_SOURCES
        infrastructure/windows/windows_camera.cpp
    )
    set(PLATFORM_LIBS
        mf
        mfplat  
        mfreadwrite
        mfuuid
    )
elseif(UNIX)
    # Unix platform sources would go here
    set(PLATFORM_SOURCES "")
endif()

# Create camera library with platform-specific implementations
if(PLATFORM_SOURCES)
    add_library(camera_implementations ${PLATFORM_SOURCES})
    
    target_include_directories(camera_implementations PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    
    target_link_libraries(camera_implementations PRIVATE
        ${PLATFORM_LIBS}
    )
    
    # Platform-specific compile definitions
    if(UNIX)
        target_compile_definitions(camera_implementations PUBLIC CAMERA_UNIX_PLATFORM)
    elseif(WIN32)
        target_compile_definitions(camera_implementations PUBLIC CAMERA_WINDOWS_PLATFORM)
    endif()
else()
    # Header-only library for camera abstractions when no platform implementation
    add_library(camera_implementations INTERFACE)
    target_include_directories(camera_implementations INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

# Header-only library for camera abstractions (always available)
add_library(camera_abstractions INTERFACE)

# Include directories for the bounded context
target_include_directories(camera_abstractions INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Platform-specific compile definitions
if(UNIX)
    target_compile_definitions(camera_abstractions INTERFACE CAMERA_UNIX_PLATFORM)
elseif(WIN32)
    target_compile_definitions(camera_abstractions INTERFACE CAMERA_WINDOWS_PLATFORM)
endif()

# Enable testing
option(BUILD_CAMERA_TESTS "Build Camera bounded context unit tests" ON)
if(BUILD_CAMERA_TESTS)
    add_subdirectory(tests)
endif()
