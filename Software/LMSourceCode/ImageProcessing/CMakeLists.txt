cmake_minimum_required(VERSION 3.10)
project(PiTrac CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED COMPONENTS 
    filesystem 
    system 
    timer 
    regex
    program_options
)

# Set global include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

# Add subdirectories
add_subdirectory(core)
add_subdirectory(encoder)
add_subdirectory(image)
add_subdirectory(output)
add_subdirectory(preview)
add_subdirectory(post_processing_stages)

# Add test executable
add_executable(gs_automated_testing 
    gs_automated_testing.cpp
    gs_automated_testing.h
)

# Link against required libraries and components
target_link_libraries(gs_automated_testing 
    PRIVATE
    ${OpenCV_LIBS}
    Boost::filesystem
    Boost::system
    Boost::timer
    Boost::regex
    Boost::program_options
    libcamera_app         # From core/
    post_processing_stages # From post_processing_stages/
    preview              # From preview/
    # Other libraries will be pulled in as dependencies
)

# Set include directories
target_include_directories(gs_automated_testing 
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)
