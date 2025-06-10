/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Platform Selection Tests
 * 
 * Unit tests for the camera platform selection mechanism using xUnit Arrange-Act-Assert pattern.
 * Tests that the correct platform implementation is selected and basic integration works.
 */

#include <boost/test/unit_test.hpp>
#include "camera_platform.hpp"
#include <vector>

BOOST_AUTO_TEST_SUITE(InfrastructureTests)

BOOST_AUTO_TEST_CASE(platform_header_includes_domain) {
    // Arrange
    // (Testing that including camera_platform.hpp provides domain types)
    
    // Act
    libcamera_domain::Size size(640, 480);
    libcamera_domain::Transform transform(90);
    libcamera_domain::PixelFormat format(0x32315659);
    libcamera_domain::ColorSpace colorSpace(1);
    
    // Assert
    BOOST_CHECK_EQUAL(size.width, 640);
    BOOST_CHECK_EQUAL(size.height, 480);
    BOOST_CHECK_EQUAL(transform.value, 90);
    BOOST_CHECK_EQUAL(format.fourcc, 0x32315659);
    BOOST_CHECK_EQUAL(colorSpace.value, 1);
}

BOOST_AUTO_TEST_CASE(single_include_provides_complete_interface) {
    // Arrange
    // (Testing that single include provides all necessary types)
    
    // Act - Create instances of all domain types
    libcamera_domain::Size test_size;
    libcamera_domain::Transform test_transform;
    libcamera_domain::PixelFormat test_format;
    libcamera_domain::ColorSpace test_colorspace;
    
    // Assert - Verify types are available and constructible
    BOOST_CHECK_NO_THROW(test_size = libcamera_domain::Size(1280, 720));
    BOOST_CHECK_NO_THROW(test_transform = libcamera_domain::Transform(180));
    BOOST_CHECK_NO_THROW(test_format = libcamera_domain::PixelFormat(0x56595559));
    BOOST_CHECK_NO_THROW(test_colorspace = libcamera_domain::ColorSpace(2));
    
    // Verify the values were set correctly
    BOOST_CHECK_EQUAL(test_size.width, 1280);
    BOOST_CHECK_EQUAL(test_size.height, 720);
    BOOST_CHECK_EQUAL(test_transform.value, 180);
    BOOST_CHECK_EQUAL(test_format.fourcc, 0x56595559);
    BOOST_CHECK_EQUAL(test_colorspace.value, 2);
}

BOOST_AUTO_TEST_SUITE_END()

// Platform-specific test suites
#ifdef __unix__
BOOST_AUTO_TEST_SUITE(UnixTests)

BOOST_AUTO_TEST_CASE(unix_platform_implementation_available) {
    // Arrange
    // (Testing Unix platform-specific functionality when available)
    
    // Act & Assert
    // On Unix, we should have access to both domain and infrastructure types
    libcamera_domain::Size domain_size(1920, 1080);
    BOOST_CHECK_EQUAL(domain_size.width, 1920);
    BOOST_CHECK_EQUAL(domain_size.height, 1080);
    
    // Note: Additional Unix-specific tests can be added here
    // when the Unix infrastructure implementation provides more functionality
}

BOOST_AUTO_TEST_SUITE_END()

#elif defined(_WIN32) || defined(WIN32)
BOOST_AUTO_TEST_SUITE(WindowsTests)

BOOST_AUTO_TEST_CASE(windows_platform_implementation_available) {
    // Arrange
    // (Testing Windows platform-specific functionality when available)
    
    // Act & Assert
    // On Windows, we should have access to domain types
    libcamera_domain::Size domain_size(1920, 1080);
    BOOST_CHECK_EQUAL(domain_size.width, 1920);
    BOOST_CHECK_EQUAL(domain_size.height, 1080);
    
    // Note: Additional Windows-specific tests will be added here
    // when the Windows infrastructure implementation is created
}

BOOST_AUTO_TEST_SUITE_END()

#endif

// Integration tests that should work on all platforms
BOOST_AUTO_TEST_SUITE(CrossPlatformTests)

BOOST_AUTO_TEST_CASE(platform_abstraction_provides_consistent_interface) {
    // Arrange
    const unsigned int test_width = 800;
    const unsigned int test_height = 600;
    const int test_transform = 270;
    const unsigned int test_fourcc = 0x32315559; // YUV format
    const int test_colorspace = 3;
    
    // Act
    libcamera_domain::Size size(test_width, test_height);
    libcamera_domain::Transform transform(test_transform);
    libcamera_domain::PixelFormat format(test_fourcc);
    libcamera_domain::ColorSpace colorSpace(test_colorspace);
    
    // Assert - Same interface works regardless of platform
    BOOST_CHECK_EQUAL(size.width, test_width);
    BOOST_CHECK_EQUAL(size.height, test_height);
    BOOST_CHECK_EQUAL(transform.value, test_transform);
    BOOST_CHECK_EQUAL(format.fourcc, test_fourcc);
    BOOST_CHECK_EQUAL(colorSpace.value, test_colorspace);
}

BOOST_AUTO_TEST_CASE(domain_types_work_in_collections) {
    // Arrange
    std::vector<libcamera_domain::Size> sizes;
    std::vector<libcamera_domain::Transform> transforms;
    
    // Act
    sizes.push_back(libcamera_domain::Size(640, 480));
    sizes.push_back(libcamera_domain::Size(1280, 720));
    sizes.push_back(libcamera_domain::Size(1920, 1080));
    
    transforms.push_back(libcamera_domain::Transform(0));
    transforms.push_back(libcamera_domain::Transform(90));
    transforms.push_back(libcamera_domain::Transform(180));
    transforms.push_back(libcamera_domain::Transform(270));
    
    // Assert
    BOOST_CHECK_EQUAL(sizes.size(), 3);
    BOOST_CHECK_EQUAL(transforms.size(), 4);
    
    BOOST_CHECK_EQUAL(sizes[0].width, 640);
    BOOST_CHECK_EQUAL(sizes[0].height, 480);
    BOOST_CHECK_EQUAL(sizes[2].width, 1920);
    BOOST_CHECK_EQUAL(sizes[2].height, 1080);
    
    BOOST_CHECK_EQUAL(transforms[1].value, 90);
    BOOST_CHECK_EQUAL(transforms[3].value, 270);
}

BOOST_AUTO_TEST_SUITE_END()
