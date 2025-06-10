cmake_minimum_required(VERSION 3.6)

include(GNUInstallDirs)

# Find Boost for compatibility with core headers
find_package(Boost REQUIRED)

# Platform-specific source files
if(WIN32)
    # Windows: Only include file output and base output (no network output)
    add_library(outputs output.cpp file_output.cpp circular_output.cpp)
else()
    # Unix/Linux: Include all outputs including network
    add_library(outputs output.cpp file_output.cpp net_output.cpp circular_output.cpp)
endif()
set_target_properties(outputs PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

# Add include directories for finding core headers
target_include_directories(outputs PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${Boost_INCLUDE_DIRS}
)

install(TARGETS outputs LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

list(APPEND ${PROJECT_NAME}_HEADERS
    circular_output.hpp
    file_output.hpp
    net_output.hpp
    output.hpp
)

install(FILES
    ${${PROJECT_NAME}_HEADERS}
    DESTINATION
    ${INCLUDE_INSTALL_DIR}/${PROJECT_NAME}/output
    COMPONENT Devel
)
