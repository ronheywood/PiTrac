#pragma once
#include <opencv2/core.hpp>

// A simplified header just for testing the Boost Test framework
// This class contains only the static test helper functions that we want to test

class TestHelpers {
public:
    // Test if the absolute differences between expected and result are within tolerances
    static bool WithinTolerance(const cv::Vec2d& expected, const cv::Vec2d& result, const cv::Vec2d& abs_tolerances) {
        return 
            std::abs(expected[0] - result[0]) <= abs_tolerances[0] &&
            std::abs(expected[1] - result[1]) <= abs_tolerances[1];
    }

    static bool WithinTolerance(const cv::Vec3d& expected, const cv::Vec3d& result, const cv::Vec3d& abs_tolerances) {
        return 
            std::abs(expected[0] - result[0]) <= abs_tolerances[0] &&
            std::abs(expected[1] - result[1]) <= abs_tolerances[1] &&
            std::abs(expected[2] - result[2]) <= abs_tolerances[2];
    }

    static bool WithinTolerance(float expected, float result, float abs_tolerance) {
        return std::abs(expected - result) <= abs_tolerance;
    }

    static bool WithinTolerance(int expected, int result, int abs_tolerance) {
        return std::abs(expected - result) <= abs_tolerance;
    }
};
