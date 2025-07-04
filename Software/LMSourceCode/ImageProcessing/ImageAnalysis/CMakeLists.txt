# SPDX-License-Identifier: GPL-2.0-only
# Copyright (C) 2022-2025, Verdant Consultants, LLC.

cmake_minimum_required(VERSION 3.16)
project(ImageAnalysis)

# Handle CMake policy for FindBoost module removal in newer CMake versions
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)  # Keep old behavior for compatibility
endif()

# Handle CMake policy for BOOST_ROOT variable
if(POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW)
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
# Configure OpenCV detection - require explicit OPENCV_DIR
if(NOT DEFINED ENV{OPENCV_DIR} AND NOT DEFINED OPENCV_DIR)
    message(FATAL_ERROR 
        "OPENCV_DIR must be set. Please set environment variable or pass -DOPENCV_DIR=<path> to CMake.\n"
        "Example: $env:OPENCV_DIR = \"C:/opencv\" (PowerShell)\n"
        "Or: cmake -DOPENCV_DIR=\"C:/opencv\" .."
    )
endif()

# Use environment variable if set, otherwise use CMake variable
if(DEFINED ENV{OPENCV_DIR} AND NOT DEFINED OPENCV_DIR)
    set(OPENCV_DIR $ENV{OPENCV_DIR})
    message(STATUS "Using OPENCV_DIR from environment: ${OPENCV_DIR}")
else()
    message(STATUS "Using OPENCV_DIR from CMake: ${OPENCV_DIR}")
endif()

# Add OpenCV directory to the prefix path so find_package can find it
# The path should point directly to the directory containing OpenCVConfig.cmake
list(APPEND CMAKE_PREFIX_PATH "${OPENCV_DIR}")
message(STATUS "Looking for OpenCVConfig.cmake in: ${OPENCV_DIR}")

# Find OpenCV using the specified path - no fallbacks or guessing
find_package(OpenCV REQUIRED)

# Configure Boost detection - require explicit BOOST_ROOT
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

# Require BOOST_ROOT to be set (either environment variable or CMake variable)
if(NOT DEFINED ENV{BOOST_ROOT} AND NOT DEFINED BOOST_ROOT)
    message(FATAL_ERROR 
        "BOOST_ROOT must be set. Please set environment variable or pass -DBOOST_ROOT=<path> to CMake.\n"
        "Example: $env:BOOST_ROOT = \"C:/Dev_Libs/boost\" (PowerShell)\n"
        "Or: cmake -DBOOST_ROOT=\"C:/Dev_Libs/boost\" .."
    )
endif()

# Use environment variable if set, otherwise use CMake variable
if(DEFINED ENV{BOOST_ROOT} AND NOT DEFINED BOOST_ROOT)
    set(BOOST_ROOT $ENV{BOOST_ROOT})
    message(STATUS "Using BOOST_ROOT from environment: ${BOOST_ROOT}")
else()
    message(STATUS "Using BOOST_ROOT from CMake: ${BOOST_ROOT}")
endif()

# Configure Boost detection
set(Boost_NO_SYSTEM_PATHS ON)
set(Boost_NO_BOOST_CMAKE ON)  # Use traditional FindBoost for consistency

# Find Boost with unit test framework
find_package(Boost REQUIRED COMPONENTS unit_test_framework)

# Include directories
include_directories(${OpenCV_INCLUDE_DIRS})

# Source files for the bounded context
set(IMAGE_ANALYSIS_SOURCES
    infrastructure/opencv_image_analyzer.cpp
)

set(IMAGE_ANALYSIS_HEADERS
    domain/value_objects.hpp
    domain/analysis_results.hpp
    domain/interfaces.hpp
    application/image_analysis_service.hpp
    infrastructure/opencv_image_analyzer.hpp
    infrastructure/ml_image_analyzer.hpp
)

# Create static library for the bounded context
add_library(image_analysis STATIC ${IMAGE_ANALYSIS_SOURCES})

# Link libraries
target_link_libraries(image_analysis 
    ${OpenCV_LIBS}
)

# Set include directories for the library
target_include_directories(image_analysis PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..  # For access to existing headers
)

# Compiler flags
if(MSVC)
    target_compile_options(image_analysis PRIVATE
        /W4  # High warning level
        $<$<CONFIG:Debug>:/Od /Zi>
        $<$<CONFIG:Release>:/O2 /DNDEBUG>
    )
else()
    target_compile_options(image_analysis PRIVATE
        -Wall -Wextra -Wpedantic
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O3 -DNDEBUG>
    )
endif()

# Function to configure Boost Test linking for a target
function(configure_boost_test_target target_name)
    target_link_libraries(${target_name} PRIVATE 
        Boost::unit_test_framework 
        image_analysis 
        ${OpenCV_LIBS}
    )
    target_compile_definitions(${target_name} PRIVATE BOOST_TEST_DYN_LINK)
endfunction()

# Add tests if Boost is available
if(Boost_FOUND)
    enable_testing()
    
    # Domain tests (no external dependencies)
    add_executable(test_image_analysis_domain tests/test_image_analysis_domain.cpp)
    configure_boost_test_target(test_image_analysis_domain)
    add_test(NAME DomainTests COMMAND test_image_analysis_domain)
    
    # OpenCV integration tests
    add_executable(test_opencv_analyzer tests/test_opencv_analyzer.cpp)
    configure_boost_test_target(test_opencv_analyzer)
    add_test(NAME OpenCVTests COMMAND test_opencv_analyzer)
    
    # Approval tests with real PiTrac images
    add_executable(test_approval_with_pitrac_images tests/test_approval_with_pitrac_images.cpp)
    configure_boost_test_target(test_approval_with_pitrac_images)
    add_test(NAME ApprovalTests COMMAND test_approval_with_pitrac_images)
    
    # Set test properties
    set_tests_properties(DomainTests PROPERTIES
        TIMEOUT 30
        LABELS "unit;domain"
    )
    
    set_tests_properties(OpenCVTests PROPERTIES
        TIMEOUT 60
        LABELS "integration;opencv"
    )
    
    set_tests_properties(ApprovalTests PROPERTIES
        TIMEOUT 120
        LABELS "approval;pitrac;images"
    )
    
else()
    message(WARNING "Boost not found. Tests will not be built.")
endif()

# Install rules
install(TARGETS image_analysis
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${IMAGE_ANALYSIS_HEADERS}
    DESTINATION include/ImageAnalysis
)

# Development convenience targets
add_custom_target(run_tests
    COMMAND ctest --output-on-failure
    DEPENDS test_image_analysis_domain test_opencv_analyzer test_approval_with_pitrac_images
    COMMENT "Running all image analysis tests"
)

add_custom_target(run_domain_tests
    COMMAND test_image_analysis_domain
    DEPENDS test_image_analysis_domain
    COMMENT "Running domain tests only"
)

add_custom_target(run_integration_tests
    COMMAND test_opencv_analyzer
    DEPENDS test_opencv_analyzer
    COMMENT "Running integration tests only"
)

add_custom_target(run_approval_tests
    COMMAND test_approval_with_pitrac_images
    DEPENDS test_approval_with_pitrac_images
    COMMENT "Running approval tests with real PiTrac images"
)

# Print configuration summary
message(STATUS "Image Analysis Bounded Context Configuration:")
message(STATUS "  OpenCV version: ${OpenCV_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Tests enabled: ${Boost_FOUND}")
