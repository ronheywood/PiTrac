# Camera Bounded Context - Unit Tests
# Using Boost Test Framework with xUnit Arrange-Act-Assert pattern

# Find Boost Test framework
find_package(Boost REQUIRED COMPONENTS unit_test_framework)

# Create the test executable target
add_executable(camera_tests
    test_main.cpp
    domain/test_camera_domain.cpp
    domain/test_advanced_domain.cpp
    infrastructure/test_platform_selection.cpp
)

# Link with Boost Test
target_link_libraries(camera_tests 
    PRIVATE 
    Boost::unit_test_framework
)

# Include paths for testing
target_include_directories(camera_tests 
    PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/..  # Camera bounded context root
    ${CMAKE_CURRENT_SOURCE_DIR}     # Test directory
)

# Compiler definitions for Boost Test
target_compile_definitions(camera_tests 
    PRIVATE 
    BOOST_TEST_DYN_LINK
)

# Enable testing for this directory
enable_testing()

# Register the test with CTest
add_test(
    NAME camera_unit_tests
    COMMAND camera_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Set test properties for better output
set_tests_properties(camera_unit_tests 
    PROPERTIES
    TIMEOUT 30
    LABELS "unit;camera"
)

# Create test groups for selective running
add_test(
    NAME camera_domain_tests
    COMMAND camera_tests --run_test=DomainTests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_test(
    NAME camera_infrastructure_tests  
    COMMAND camera_tests --run_test=InfrastructureTests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Platform-specific conditional tests
if(UNIX)
    add_test(
        NAME camera_unix_tests
        COMMAND camera_tests --run_test=UnixTests
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
elseif(WIN32)
    add_test(
        NAME camera_windows_tests
        COMMAND camera_tests --run_test=WindowsTests  
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()
