# SPDX-License-Identifier: GPL-2.0-only
# Copyright (C) 2022-2025, Verdant Consultants, LLC.

cmake_minimum_required(VERSION 3.16)
project(ImageAnalysis)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
#set(OpenCV_DIR "C:/opencv/build")
find_package(OpenCV REQUIRED)

# Configure Boost detection
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

message(STATUS "Looking for Boost Test framework...")
if(WIN32)
    # On Windows, try different naming conventions
    find_package(Boost QUIET COMPONENTS unit_test_framework)
    if(NOT Boost_unit_test_framework_FOUND)
        find_package(Boost QUIET COMPONENTS test)
        if(NOT Boost_test_FOUND)
            find_package(Boost QUIET COMPONENTS test_exec_monitor)
            if(NOT Boost_test_exec_monitor_FOUND)
                # Try header-only approach as fallback
                find_package(Boost REQUIRED)
                set(BOOST_TEST_HEADER_ONLY TRUE)
            endif()
        endif()
    endif()
else()
    # On Unix systems, standard naming should work
    find_package(Boost REQUIRED COMPONENTS unit_test_framework)
endif()

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

# Add tests if Boost is available
if(Boost_FOUND)
    enable_testing()
    
    # Domain tests (no external dependencies)
    add_executable(test_image_analysis_domain
        tests/test_image_analysis_domain.cpp
    )
    
    # Configure Boost linking
    if(BOOST_TEST_HEADER_ONLY)
        target_compile_definitions(test_image_analysis_domain 
            PRIVATE 
            BOOST_TEST_HEADER_ONLY=1
        )
        target_include_directories(test_image_analysis_domain 
            PRIVATE 
            ${Boost_INCLUDE_DIRS}
        )
    else()
        if(TARGET Boost::unit_test_framework)
            target_link_libraries(test_image_analysis_domain PRIVATE Boost::unit_test_framework)
            target_compile_definitions(test_image_analysis_domain PRIVATE BOOST_TEST_DYN_LINK)
        elseif(TARGET Boost::test)
            target_link_libraries(test_image_analysis_domain PRIVATE Boost::test)
            target_compile_definitions(test_image_analysis_domain PRIVATE BOOST_TEST_DYN_LINK)
        else()
            target_link_libraries(test_image_analysis_domain PRIVATE ${Boost_LIBRARIES})
            target_include_directories(test_image_analysis_domain PRIVATE ${Boost_INCLUDE_DIRS})
            target_compile_definitions(test_image_analysis_domain PRIVATE BOOST_TEST_DYN_LINK)
        endif()
    endif()
      target_link_libraries(test_image_analysis_domain
        PRIVATE
        image_analysis
        ${OpenCV_LIBS}
    )
    add_test(NAME DomainTests COMMAND test_image_analysis_domain)
    
    # OpenCV integration tests
    add_executable(test_opencv_analyzer
        tests/test_opencv_analyzer.cpp
    )
    
    # Configure Boost linking for OpenCV tests
    if(BOOST_TEST_HEADER_ONLY)
        target_compile_definitions(test_opencv_analyzer 
            PRIVATE 
            BOOST_TEST_HEADER_ONLY=1
        )
        target_include_directories(test_opencv_analyzer 
            PRIVATE 
            ${Boost_INCLUDE_DIRS}
        )
    else()
        if(TARGET Boost::unit_test_framework)
            target_link_libraries(test_opencv_analyzer PRIVATE Boost::unit_test_framework)
            target_compile_definitions(test_opencv_analyzer PRIVATE BOOST_TEST_DYN_LINK)
        elseif(TARGET Boost::test)
            target_link_libraries(test_opencv_analyzer PRIVATE Boost::test)
            target_compile_definitions(test_opencv_analyzer PRIVATE BOOST_TEST_DYN_LINK)
        else()
            target_link_libraries(test_opencv_analyzer PRIVATE ${Boost_LIBRARIES})
            target_include_directories(test_opencv_analyzer PRIVATE ${Boost_INCLUDE_DIRS})
            target_compile_definitions(test_opencv_analyzer PRIVATE BOOST_TEST_DYN_LINK)
        endif()
    endif()
    
    target_link_libraries(test_opencv_analyzer
        PRIVATE
        image_analysis
        ${OpenCV_LIBS}
    )
    add_test(NAME OpenCVTests COMMAND test_opencv_analyzer)
    
    # Approval tests with real PiTrac images
    add_executable(test_approval_with_pitrac_images
        tests/test_approval_with_pitrac_images.cpp
    )
    
    # Configure Boost linking for approval tests
    if(BOOST_TEST_HEADER_ONLY)
        target_compile_definitions(test_approval_with_pitrac_images 
            PRIVATE 
            BOOST_TEST_HEADER_ONLY=1
        )
        target_include_directories(test_approval_with_pitrac_images 
            PRIVATE 
            ${Boost_INCLUDE_DIRS}
        )
    else()
        if(TARGET Boost::unit_test_framework)
            target_link_libraries(test_approval_with_pitrac_images PRIVATE Boost::unit_test_framework)
            target_compile_definitions(test_approval_with_pitrac_images PRIVATE BOOST_TEST_DYN_LINK)
        elseif(TARGET Boost::test)
            target_link_libraries(test_approval_with_pitrac_images PRIVATE Boost::test)
            target_compile_definitions(test_approval_with_pitrac_images PRIVATE BOOST_TEST_DYN_LINK)
        else()
            target_link_libraries(test_approval_with_pitrac_images PRIVATE ${Boost_LIBRARIES})
            target_include_directories(test_approval_with_pitrac_images PRIVATE ${Boost_INCLUDE_DIRS})
            target_compile_definitions(test_approval_with_pitrac_images PRIVATE BOOST_TEST_DYN_LINK)
        endif()
    endif()
    
    target_link_libraries(test_approval_with_pitrac_images
        PRIVATE
        image_analysis
        ${OpenCV_LIBS}
    )
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

# Documentation target (requires Doxygen)
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    
    if(EXISTS ${DOXYGEN_IN})
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        add_custom_target(docs
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
    endif()
endif()

# Print configuration summary
message(STATUS "Image Analysis Bounded Context Configuration:")
message(STATUS "  OpenCV version: ${OpenCV_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Tests enabled: ${GTest_FOUND}")
message(STATUS "  Documentation: ${DOXYGEN_FOUND}")
