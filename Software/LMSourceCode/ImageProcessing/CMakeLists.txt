# SPDX-License-Identifier: GPL-2.0-only 
#
# Copyright (C) 2022-2025, Verdant Consultants, LLC.
#
# Root CMakeLists.txt for PiTrac Launch Monitor
# This provides CMake-based builds for Windows compatibility

cmake_minimum_required(VERSION 3.16)

project(pitrac_lm VERSION 0.0.1 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Platform-specific settings
if(WIN32)
    # Windows-specific settings
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DNOMINMAX)
    add_definitions(-D_WIN32_WINNT=0x0A00)
    add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)
    add_definitions(-DBOOST_ALL_DYN_LINK)
    add_definitions(-DBOOST_USE_WINAPI_VERSION=0x0A00)
    
    # Windows-only compiler flags
    if(MSVC)
        add_compile_options(/W3)
        add_compile_options(/bigobj)
        # Disable specific warnings that are common in this codebase
        add_compile_options(/wd4996) # deprecated functions
        add_compile_options(/wd4267) # size_t conversion
        add_compile_options(/wd4244) # type conversion
    endif()
else()
    # Unix-specific settings (matches meson.build)
    add_definitions(-D_FILE_OFFSET_BITS=64)
    add_definitions(-DLIBCXX_ENABLE_INCOMPLETE_FEATURES=ON)
    add_definitions(-DBOOST_LOG_DYN_LINK)
    add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)
    
    # Unix-only compiler flags
    add_compile_options(-Wno-deprecated-enum-enum-conversion)
    add_compile_options(-Wno-deprecated-declarations)
    add_compile_options(-Wno-comment)
    add_compile_options(-Wno-unused)
    add_compile_options(-pedantic)
    add_compile_options(-Wno-unused-parameter)
    add_compile_options(-faligned-new)
endif()

# Set include directory for installation
include(GNUInstallDirs)
set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR})

# Find required dependencies
if(WIN32)
    # Windows: Use environment variables for dependency paths
    if(DEFINED ENV{OPENCV_DIR})
        set(OpenCV_DIR "$ENV{OPENCV_DIR}")
        message(STATUS "Using OpenCV from environment: ${OpenCV_DIR}")
    endif()
    
    if(DEFINED ENV{BOOST_DIR})
        set(BOOST_ROOT "$ENV{BOOST_DIR}")
        message(STATUS "Using Boost from environment: ${BOOST_ROOT}")
    endif()
endif()

find_package(OpenCV 4.9 REQUIRED)
find_package(Boost 1.74 REQUIRED COMPONENTS program_options timer log thread filesystem regex)

# Platform-specific dependencies
if(NOT WIN32)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBCAMERA REQUIRED libcamera)
    pkg_check_modules(LGPIO REQUIRED lgpio) 
    pkg_check_modules(FMT REQUIRED fmt)
    pkg_check_modules(ACTIVEMQ REQUIRED activemq-cpp)
    pkg_check_modules(OPENSSL REQUIRED openssl)
    pkg_check_modules(MSGPACK REQUIRED msgpack-cxx)
    
    find_library(APR_LIBRARY apr-1 REQUIRED)
    find_library(BCM_HOST_LIBRARY bcm_host REQUIRED)
else()
    # Windows: These libraries don't exist, create empty targets or skip
    message(STATUS "Skipping Unix-specific dependencies on Windows")
endif()

find_package(Threads REQUIRED)

# Add subdirectories for component libraries
add_subdirectory(core)
add_subdirectory(encoder)
add_subdirectory(image)
add_subdirectory(output)
add_subdirectory(preview)
add_subdirectory(post_processing_stages)

# Define source files for main executable
set(PITRAC_LM_SOURCES
    # Main application files
    lm_main.cpp
    gs_globals.cpp
    gs_fsm.cpp
    gs_options.cpp
    gs_config.cpp
    gs_sim_interface.cpp
    gs_gspro_interface.cpp
    gs_gspro_response.cpp
    gs_gspro_test_server.cpp
    gs_sim_socket_interface.cpp
    gs_e6_interface.cpp
    gs_e6_results.cpp
    logging_tools.cpp
    gs_events.cpp
    worker_thread.cpp
    camera_hardware.cpp
    gs_ipc_message.cpp
    gs_ipc_control_msg.cpp
    gs_results.cpp
    gs_gspro_results.cpp
    gs_ui_system.cpp
    gs_ipc_mat.cpp
    gs_ipc_result.cpp
    gs_ipc_test.cpp
    gs_ipc_system.cpp
    gs_message_consumer.cpp
    gs_message_producer.cpp
    
    # Computer vision and image processing
    ball_watcher.cpp
    ball_watcher_image_buffer.cpp
    ball_image_proc.cpp
    pulse_strobe.cpp
    colorsys.cpp
    cv_utils.cpp
    golf_ball.cpp
    gs_automated_testing.cpp
    gs_camera.cpp
    gs_clubs.cpp
    gs_club_data.cpp
    
    # Ellipse detection
    ED.cpp
    EDColor.cpp
    EDPF.cpp
    EllipseDetectorCommon.cpp
    EllipseDetectorYaed.cpp
)

# Platform-specific sources
if(NOT WIN32)
    list(APPEND PITRAC_LM_SOURCES
        libcamera_interface.cpp
        libcamera_jpeg.cpp
    )
endif()

# Handle closed source components (simplified for Windows)
if(WIN32)
    # On Windows, we'll use a stub or skip closed source components
    list(APPEND PITRAC_LM_SOURCES gs_e6_response.cpp)
else()
    # Unix handling of closed source object files would go here
endif()

# Create main executable
add_executable(pitrac_lm ${PITRAC_LM_SOURCES})

# Set target properties
set_target_properties(pitrac_lm PROPERTIES
    VERSION ${PROJECT_VERSION}
    OUTPUT_NAME pitrac_lm
)

# Include directories
target_include_directories(pitrac_lm PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

# Link libraries
set(PITRAC_LINK_LIBRARIES
    ${OpenCV_LIBS}
    ${Boost_LIBRARIES}
    Threads::Threads
    libcamera_app
    encoders
    images
    outputs
    preview
    post_processing_stages
)

# Platform-specific linking
if(WIN32)
    # Windows-specific libraries
    list(APPEND PITRAC_LINK_LIBRARIES
        ws2_32
        iphlpapi
    )
else()
    # Unix-specific libraries
    list(APPEND PITRAC_LINK_LIBRARIES
        ${LIBCAMERA_LIBRARIES}
        ${LGPIO_LIBRARIES}
        ${FMT_LIBRARIES}
        ${ACTIVEMQ_LIBRARIES}
        ${OPENSSL_LIBRARIES}
        ${MSGPACK_LIBRARIES}
        ${APR_LIBRARY}
        ${BCM_HOST_LIBRARY}
    )
endif()

target_link_libraries(pitrac_lm ${PITRAC_LINK_LIBRARIES})

# Preprocessor definitions
target_compile_definitions(pitrac_lm PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# Installation
install(TARGETS pitrac_lm
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Optional: Create a config package for other projects to find PiTrac
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/pitrac_lm-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/pitrac_lm-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pitrac_lm
)
