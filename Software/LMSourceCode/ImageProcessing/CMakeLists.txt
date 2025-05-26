cmake_minimum_required(VERSION 3.10)
project(PiTrac)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find OpenCV
find_package(OpenCV REQUIRED)

# Find Boost components
find_package(Boost REQUIRED COMPONENTS 
    filesystem 
    system
    timer
    log
    regex
    thread
    test
    program_options
)

# Include directories
include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    .
    ./core
    ./output
    ./image
    ./preview
    ./encoder
    ./post_processing_stages
)

# Define source files
set(SOURCES
    gs_main.cpp
    gs_automated_testing.cpp
    gs_config.cpp
    # Add other .cpp files here
)

# Main executable
add_executable(pitrac_lm ${SOURCES})
target_link_libraries(pitrac_lm 
    ${OpenCV_LIBS}
    ${Boost_LIBRARIES}
    pthread
)

# Test executable
add_executable(pitrac_tests boost_tests.cpp gs_automated_testing.cpp gs_config.cpp)
target_link_libraries(pitrac_tests 
    ${OpenCV_LIBS}
    ${Boost_LIBRARIES}
    pthread
)
