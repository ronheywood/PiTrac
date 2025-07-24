/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2022-2025, Verdant Consultants, LLC.
 */

/**
 * @file test_approval_with_pitrac_images_simple.cpp
 * @brief Simple test to verify shared approval testing framework integration
 * 
 * This test validates that the shared approval testing framework is properly
 * integrated with the ImageAnalysis bounded context.
 */

#define BOOST_TEST_MODULE SimpleApprovalTestsWithSharedFramework
#include <boost/test/unit_test.hpp>
#include "../../shared/testing/approval/approval_test_orchestrator.hpp"
#include "../../shared/testing/approval/approval_test_config.hpp"
#include "approval/result_formatter.hpp"
#include "approval/visualization_service.hpp"
#include <opencv2/opencv.hpp>

using namespace golf_sim::shared::testing;
using namespace golf_sim::image_analysis::testing;

BOOST_AUTO_TEST_SUITE(SimpleApprovalTestsWithSharedFramework)

/**
 * @brief Test that shared approval framework is accessible
 */
BOOST_AUTO_TEST_CASE(SharedFrameworkAccessible) {
    // Verify we can access the shared approval test config
    const auto& config = ApprovalTestConfig::Instance();
    
    // Verify directory access works
    std::string artifacts_dir = config.GetImageAnalysisArtifactsDir();
    BOOST_CHECK(!artifacts_dir.empty());
    
    // Verify we can create an orchestrator
    ApprovalTestOrchestrator orchestrator(config);
    BOOST_CHECK(true); // If we get here, construction succeeded
}

/**
 * @brief Test that domain-specific formatters work with shared framework
 */
BOOST_AUTO_TEST_CASE(DomainSpecificFormattersAccessible) {
    // Verify we can create domain-specific services
    auto formatter = std::make_unique<StandardResultFormatter>();
    BOOST_CHECK(formatter != nullptr);
    
    // Verify we can create visualization service with shared config
    const auto& config = ApprovalTestConfig::Instance();
    auto visualizer = std::make_unique<OpenCVVisualizationService>(config);
    BOOST_CHECK(visualizer != nullptr);
}

/**
 * @brief Test basic OpenCV integration works
 */
BOOST_AUTO_TEST_CASE(OpenCVIntegrationWorks) {
    // Create a simple test image
    cv::Mat test_image = cv::Mat::zeros(100, 100, CV_8UC3);
    cv::rectangle(test_image, cv::Point(25, 25), cv::Point(75, 75), cv::Scalar(255, 255, 255), -1);
    
    // Verify the image was created
    BOOST_CHECK(!test_image.empty());
    BOOST_CHECK_EQUAL(test_image.rows, 100);
    BOOST_CHECK_EQUAL(test_image.cols, 100);
}

BOOST_AUTO_TEST_SUITE_END()
