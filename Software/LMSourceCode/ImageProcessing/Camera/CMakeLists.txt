cmake_minimum_required(VERSION 3.6)

# Camera Bounded Context
# Handles all camera-related domain and infrastructure concerns

project(camera_context)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Camera domain and infrastructure headers
set(CAMERA_HEADERS
    domain/camera_domain.hpp
)

# Platform-specific infrastructure headers
if(UNIX)
    list(APPEND CAMERA_HEADERS
        infrastructure/unix/libcamera_unix_impl.hpp
    )
elseif(WIN32)
    # Windows infrastructure will be added here when created
    # list(APPEND CAMERA_HEADERS
    #     infrastructure/windows/libcamera_windows_impl.hpp
    # )
endif()

# Header-only library for camera abstractions
add_library(camera_abstractions INTERFACE)

# Include directories for the bounded context
target_include_directories(camera_abstractions INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/domain
    ${CMAKE_CURRENT_SOURCE_DIR}/infrastructure
)

# Platform-specific compile definitions
if(UNIX)
    target_compile_definitions(camera_abstractions INTERFACE CAMERA_UNIX_PLATFORM)
elseif(WIN32)
    target_compile_definitions(camera_abstractions INTERFACE CAMERA_WINDOWS_PLATFORM)
endif()

# Install headers
install(FILES ${CAMERA_HEADERS}
    DESTINATION include/Camera
    COMPONENT Devel
)
