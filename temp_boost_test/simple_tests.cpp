#define BOOST_TEST_MODULE PiTracTests
#include <boost/test/included/unit_test.hpp>
#include "test_helpers.h"
#include <iostream>

// Tests for the TestHelpers class
BOOST_AUTO_TEST_CASE(HelperTests)
{
    std::cout << "Running TestHelpers tests..." << std::endl;
    
    // Test Vec2d version
    cv::Vec2d expected2d(10.0, 20.0);
    cv::Vec2d result2d(11.0, 19.5);
    cv::Vec2d tolerance2d(2.0, 1.0);
    BOOST_CHECK(TestHelpers::WithinTolerance(expected2d, result2d, tolerance2d));
    
    // Test Vec2d version with failing case
    cv::Vec2d result2d_fail(13.0, 20.0);
    BOOST_CHECK(!TestHelpers::WithinTolerance(expected2d, result2d_fail, tolerance2d));
    
    // Test Vec3d version
    cv::Vec3d expected3d(10.0, 20.0, 30.0);
    cv::Vec3d result3d(11.0, 19.5, 29.0);
    cv::Vec3d tolerance3d(2.0, 1.0, 1.5);
    BOOST_CHECK(TestHelpers::WithinTolerance(expected3d, result3d, tolerance3d));
    
    // Test float version
    BOOST_CHECK(TestHelpers::WithinTolerance(10.0f, 10.5f, 1.0f));
    BOOST_CHECK(!TestHelpers::WithinTolerance(10.0f, 12.0f, 1.0f));
    
    // Test int version
    BOOST_CHECK(TestHelpers::WithinTolerance(100, 101, 2));
    BOOST_CHECK(!TestHelpers::WithinTolerance(100, 103, 2));
}
