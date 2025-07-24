/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2022-2025, Verdant Consultants, LLC.
 */

/**
 * @file test_approval_with_pitrac_images.cpp
 * @brief Approval tests using shared approval testing framework
 * 
 * Demonstrates using the shared approval testing framework for ImageAnalysis tests.
 * Uses the domain-specific services for golf ball analysis formatting and visualization.
 */

#define BOOST_TEST_MODULE ApprovalTestsWithSharedFramework
#include <boost/test/unit_test.hpp>
#include "../application/image_analysis_service.hpp"
#include "../infrastructure/opencv_image_analyzer.hpp"
#include "../../shared/testing/approval/approval_test_orchestrator.hpp"
#include "approval/result_formatter.hpp"
#include "approval/visualization_service.hpp"
#include <opencv2/opencv.hpp>
#include <chrono>

using namespace golf_sim::image_analysis;
using namespace golf_sim::shared::testing;

BOOST_AUTO_TEST_SUITE(ApprovalTestsWithSharedFramework)

/**
 * @brief Test fixture using shared approval framework
 * 
 * Uses the shared approval testing framework with domain-specific formatters.
 */
struct SharedApprovalTestFixture {
    SharedApprovalTestFixture() 
        : orchestrator("ImageAnalysis") {
        // Initialize dependencies
        analyzer = std::make_unique<infrastructure::OpenCVImageAnalyzer>();
        formatter = std::make_unique<golf_sim::image_analysis::testing::StandardApprovalFormatter>();
        visualizer = std::make_unique<golf_sim::image_analysis::testing::StandardVisualizationService>(
            golf_sim::image_analysis::testing::ApprovalTestConfig::Instance()
        );
        
        // Generate consistent timestamp for reproducible tests
        test_timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
        );
    }
    
    /**
     * @brief Run a golf ball analysis test using shared framework
     * @param image_filename Name of image in PiTrac images directory
     * @param test_name Unique test identifier
     */
    void RunGolfBallAnalysisTest(const std::string& image_filename, const std::string& test_name) {
        // Create data generator for golf ball analysis
        auto data_generator = [this, image_filename](const std::string& test_name) {
            // Load image from PiTrac images directory
            std::string image_path = config.GetPiTracImagesDir() + image_filename;
            cv::Mat image = cv::imread(image_path);
            if (image.empty()) {
                throw std::runtime_error("Cannot load image: " + image_path);
            }
            
            // Run analysis
            auto result = analyzer->AnalyzeTeedBallImage(
                domain::ImageBuffer{image.data, static_cast<size_t>(image.total() * image.elemSize())},
                domain::AnalysisConstraints{},
                test_timestamp
            );
            
            // Format result using domain formatter
            return formatter->FormatTeedBallResult(result);
        };
        
        // Create image generator for visualization
        auto image_generator = [this, image_filename](const std::string& test_name) {
            std::string image_path = config.GetPiTracImagesDir() + image_filename;
            cv::Mat original_image = cv::imread(image_path);
            
            // Run analysis for visualization
            auto result = analyzer->AnalyzeTeedBallImage(
                domain::ImageBuffer{original_image.data, static_cast<size_t>(original_image.total() * original_image.elemSize())},
                domain::AnalysisConstraints{},
                test_timestamp
            );
            
            // Create visualization using domain service
            cv::Mat visualization = original_image.clone();
            visualizer->CreateVisualization(original_image, result, "");
            return visualization;
        };
        
        auto result = orchestrator.RunCombinedApprovalTest(test_name, data_generator, image_generator);
        
        if (!result.passed) {
            BOOST_FAIL(result.failure_message);
        } else {
            BOOST_TEST_MESSAGE("Approval test passed for " + test_name);
        }
    }
    
    ApprovalTestOrchestrator orchestrator;
    std::unique_ptr<infrastructure::OpenCVImageAnalyzer> analyzer;
    std::unique_ptr<golf_sim::image_analysis::testing::IResultFormatter> formatter;
    std::unique_ptr<golf_sim::image_analysis::testing::IVisualizationService> visualizer;
    const golf_sim::image_analysis::testing::ApprovalTestConfig& config = 
        golf_sim::image_analysis::testing::ApprovalTestConfig::Instance();
    std::chrono::microseconds test_timestamp;
};

// Test cases using the shared approval framework
BOOST_FIXTURE_TEST_CASE(TestGolfSimLogBallFinalFoundBallImg, SharedApprovalTestFixture) {
    RunGolfBallAnalysisTest("gs_log_img_log_ball_final_found_ball_img.bmp", 
                           "gs_log_img_log_ball_final_found_ball_img");
}

BOOST_FIXTURE_TEST_CASE(TestGolfSimLogBallFinalNotFoundBallImg, SharedApprovalTestFixture) {
    RunGolfBallAnalysisTest("gs_log_img_log_ball_final_not_found_ball_img.bmp", 
                           "gs_log_img_log_ball_final_not_found_ball_img");
}

BOOST_FIXTURE_TEST_CASE(TestGolfSimLogBallInitialFoundBallImg, SharedApprovalTestFixture) {
    RunGolfBallAnalysisTest("gs_log_img_log_ball_initial_found_ball_img.bmp", 
                           "gs_log_img_log_ball_initial_found_ball_img");
}

BOOST_AUTO_TEST_SUITE_END()
