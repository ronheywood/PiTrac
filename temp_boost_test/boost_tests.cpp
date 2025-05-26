// boost_tests.cpp
// This file contains the Boost Test Framework integration for PiTrac

#define BOOST_TEST_MODULE PiTracTests
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

// Include headers needed for tests
#include <opencv2/core.hpp>
#include <iostream>

#include "gs_automated_testing.h"
#include "gs_config.h"

using namespace golf_sim;

// Mock the logger function to avoid dependencies
namespace golf_sim {
    extern std::string kAutomatedBaseTestDir;
}

// Basic test case to verify Boost Test framework is working
BOOST_AUTO_TEST_CASE(SampleTest)
{
    std::cout << "Running sample test..." << std::endl;
    BOOST_CHECK_EQUAL(1 + 1, 2);
    BOOST_TEST_MESSAGE("Sample test passed");
}

// Test case for AbsResultsPass utility functions
BOOST_AUTO_TEST_CASE(AbsResultsPassTest)
{
    std::cout << "Running AbsResultsPass test..." << std::endl;
    
    // Test Vec2d version
    cv::Vec2d expected2d(10.0, 20.0);
    cv::Vec2d result2d(11.0, 19.5);
    cv::Vec2d tolerance2d(2.0, 1.0);
    BOOST_CHECK(GsAutomatedTesting::AbsResultsPass(expected2d, result2d, tolerance2d));
    
    // Test Vec2d version with failing case
    cv::Vec2d result2d_fail(13.0, 20.0);
    BOOST_CHECK(!GsAutomatedTesting::AbsResultsPass(expected2d, result2d_fail, tolerance2d));
    
    // Test Vec3d version
    cv::Vec3d expected3d(10.0, 20.0, 30.0);
    cv::Vec3d result3d(11.0, 19.5, 29.0);
    cv::Vec3d tolerance3d(2.0, 1.0, 1.5);
    BOOST_CHECK(GsAutomatedTesting::AbsResultsPass(expected3d, result3d, tolerance3d));
    
    // Test float version
    BOOST_CHECK(GsAutomatedTesting::AbsResultsPass(10.0f, 10.5f, 1.0f));
    BOOST_CHECK(!GsAutomatedTesting::AbsResultsPass(10.0f, 12.0f, 1.0f));
    
    // Test int version
    BOOST_CHECK(GsAutomatedTesting::AbsResultsPass(100, 101, 2));
    BOOST_CHECK(!GsAutomatedTesting::AbsResultsPass(100, 103, 2));
}
