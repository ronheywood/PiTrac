/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Advanced Domain Tests
 * 
 * Comprehensive parametric tests for camera domain types using test utilities.
 * Demonstrates xUnit Arrange-Act-Assert pattern with data-driven testing.
 */

#include <boost/test/unit_test.hpp>
#include "test_utilities.hpp"

BOOST_AUTO_TEST_SUITE(AdvancedDomainTests)

BOOST_FIXTURE_TEST_CASE(size_parametric_construction_test, camera_test_utils::DomainTypeFixture) {
    // Arrange
    auto test_sizes = camera_test_utils::CommonTestData::getValidSizes();
    
    // Act & Assert
    camera_test_utils::runParametricTest(test_sizes, [this](const auto& size_pair) {
        // Arrange
        unsigned int width = size_pair.first;
        unsigned int height = size_pair.second;
        
        // Act
        libcamera_domain::Size size(width, height);
        
        // Assert
        assertSizeEquals(size, width, height);
    });
}

BOOST_FIXTURE_TEST_CASE(transform_parametric_construction_test, camera_test_utils::DomainTypeFixture) {
    // Arrange
    auto test_transforms = camera_test_utils::CommonTestData::getValidTransforms();
    
    // Act & Assert
    camera_test_utils::runParametricTest(test_transforms, [this](int transform_value) {
        // Arrange
        // (transform_value provided by parametric test)
        
        // Act
        libcamera_domain::Transform transform(transform_value);
        
        // Assert
        assertTransformEquals(transform, transform_value);
    });
}

BOOST_FIXTURE_TEST_CASE(pixel_format_parametric_construction_test, camera_test_utils::DomainTypeFixture) {
    // Arrange
    auto test_formats = camera_test_utils::CommonTestData::getValidPixelFormats();
    
    // Act & Assert
    camera_test_utils::runParametricTest(test_formats, [this](unsigned int fourcc_value) {
        // Arrange
        // (fourcc_value provided by parametric test)
        
        // Act
        libcamera_domain::PixelFormat format(fourcc_value);
        
        // Assert
        assertPixelFormatEquals(format, fourcc_value);
    });
}

BOOST_FIXTURE_TEST_CASE(color_space_parametric_construction_test, camera_test_utils::DomainTypeFixture) {
    // Arrange
    auto test_colorspaces = camera_test_utils::CommonTestData::getValidColorSpaces();
    
    // Act & Assert
    camera_test_utils::runParametricTest(test_colorspaces, [this](int colorspace_value) {
        // Arrange
        // (colorspace_value provided by parametric test)
        
        // Act
        libcamera_domain::ColorSpace colorspace(colorspace_value);
        
        // Assert
        assertColorSpaceEquals(colorspace, colorspace_value);
    });
}

BOOST_FIXTURE_TEST_CASE(domain_types_performance_test, camera_test_utils::PerformanceFixture) {
    // Arrange
    const size_t iteration_count = 100000;
    std::vector<libcamera_domain::Size> sizes;
    std::vector<libcamera_domain::Transform> transforms;
    sizes.reserve(iteration_count);
    transforms.reserve(iteration_count);
    
    // Act
    for (size_t i = 0; i < iteration_count; ++i) {
        sizes.emplace_back(1920, 1080);
        transforms.emplace_back(90);
    }
    
    // Assert
    BOOST_CHECK_EQUAL(sizes.size(), iteration_count);
    BOOST_CHECK_EQUAL(transforms.size(), iteration_count);
    
    // Verify first and last elements
    BOOST_CHECK_EQUAL(sizes[0].width, 1920);
    BOOST_CHECK_EQUAL(sizes[iteration_count - 1].height, 1080);
    BOOST_CHECK_EQUAL(transforms[0].value, 90);
    BOOST_CHECK_EQUAL(transforms[iteration_count - 1].value, 90);
}

BOOST_AUTO_TEST_CASE(domain_types_memory_layout_test) {
    // Arrange
    // (Testing memory layout and size of domain types)
    
    // Act & Assert
    // Verify that domain types have expected memory characteristics
    BOOST_CHECK(sizeof(libcamera_domain::Size) >= sizeof(unsigned int) * 2);
    BOOST_CHECK(sizeof(libcamera_domain::Transform) >= sizeof(int));
    BOOST_CHECK(sizeof(libcamera_domain::PixelFormat) >= sizeof(unsigned int));
    BOOST_CHECK(sizeof(libcamera_domain::ColorSpace) >= sizeof(int));
    
    // Verify types are not excessively large (should be simple value types)
    BOOST_CHECK(sizeof(libcamera_domain::Size) <= 16);  // Allow some padding
    BOOST_CHECK(sizeof(libcamera_domain::Transform) <= 8);
    BOOST_CHECK(sizeof(libcamera_domain::PixelFormat) <= 8);
    BOOST_CHECK(sizeof(libcamera_domain::ColorSpace) <= 8);
}

BOOST_AUTO_TEST_CASE(domain_types_assignment_test) {
    // Arrange
    libcamera_domain::Size size1(640, 480);
    libcamera_domain::Size size2(1920, 1080);
    libcamera_domain::Transform transform1(0);
    libcamera_domain::Transform transform2(90);
    
    // Act
    size1 = size2;
    transform1 = transform2;
    
    // Assert
    BOOST_CHECK_EQUAL(size1.width, size2.width);
    BOOST_CHECK_EQUAL(size1.height, size2.height);
    BOOST_CHECK_EQUAL(transform1.value, transform2.value);
}

BOOST_AUTO_TEST_CASE(domain_types_in_arrays_test) {
    // Arrange
    const size_t array_size = 5;
    libcamera_domain::Size sizes[array_size];
    libcamera_domain::Transform transforms[array_size];
    
    // Act
    for (size_t i = 0; i < array_size; ++i) {
        sizes[i] = libcamera_domain::Size(static_cast<unsigned int>(i * 100), 
                                        static_cast<unsigned int>(i * 100));
        transforms[i] = libcamera_domain::Transform(static_cast<int>(i * 90));
    }
    
    // Assert
    for (size_t i = 0; i < array_size; ++i) {
        BOOST_CHECK_EQUAL(sizes[i].width, i * 100);
        BOOST_CHECK_EQUAL(sizes[i].height, i * 100);
        BOOST_CHECK_EQUAL(transforms[i].value, static_cast<int>(i * 90));
    }
}

BOOST_AUTO_TEST_SUITE_END()
