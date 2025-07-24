/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2022-2025, Verdant Consultants, LLC.
 */

/**
 * @file image_analysis_approval_adapter.hpp
 * @brief Adapter for ImageAnalysis-specific approval testing
 * 
 * Bridges the shared approval testing framework with ImageAnalysis domain-specific
 * services like result formatting and visualization.
 */

#pragma once

#include "../../../shared/testing/approval/approval_test_orchestrator.hpp"
#include "../../../shared/testing/approval/comparison_service.hpp"
#include "../../../shared/testing/approval/diff_launcher.hpp"
#include "result_formatter.hpp"
#include "visualization_service.hpp"
#include "../../domain/analysis_results.hpp"
#include "../../infrastructure/opencv_image_analyzer.hpp"
#include <memory>

namespace golf_sim::image_analysis::testing {

/**
 * @brief ImageAnalysis-specific approval test adapter
 * 
 * Combines the shared approval testing framework with ImageAnalysis domain services
 * for specialized golf ball analysis testing.
 */
class ImageAnalysisApprovalAdapter {
public:
    /**
     * @brief Constructor with dependency injection
     */
    ImageAnalysisApprovalAdapter(
        std::unique_ptr<IResultFormatter> formatter,
        std::unique_ptr<IVisualizationService> visualizer,
        std::unique_ptr<golf_sim::shared::testing::IComparisonService> comparator,
        std::unique_ptr<golf_sim::shared::testing::IDiffLauncher> diff_launcher
    );
    
    /**
     * @brief Run approval test for golf ball analysis
     * @param image_filename Name of image file in PiTrac images directory
     * @param test_name Unique test identifier
     * @param analyzer Image analyzer to use for analysis
     * @param timestamp Test timestamp for consistent results
     * @return Approval test result
     */
    golf_sim::shared::testing::ApprovalTestResult RunGolfBallAnalysisTest(
        const std::string& image_filename,
        const std::string& test_name,
        infrastructure::OpenCVImageAnalyzer& analyzer,
        const std::chrono::microseconds& timestamp
    );
    
    /**
     * @brief Run approval test for movement analysis
     * @param image_sequence Sequence of images for movement analysis
     * @param reference_position Reference ball position
     * @param test_name Unique test identifier
     * @param analyzer Image analyzer to use for analysis
     * @return Approval test result
     */
    golf_sim::shared::testing::ApprovalTestResult RunMovementAnalysisTest(
        const std::vector<domain::ImageBuffer>& image_sequence,
        const domain::BallPosition& reference_position,
        const std::string& test_name,
        infrastructure::OpenCVImageAnalyzer& analyzer
    );

private:
    golf_sim::shared::testing::ApprovalTestOrchestrator orchestrator_;
    std::unique_ptr<IResultFormatter> formatter_;
    std::unique_ptr<IVisualizationService> visualizer_;
    std::unique_ptr<golf_sim::shared::testing::IComparisonService> comparator_;
    std::unique_ptr<golf_sim::shared::testing::IDiffLauncher> diff_launcher_;
    
    // Helper methods
    cv::Mat LoadPiTracImage(const std::string& filename) const;
    std::string FormatAnalysisResult(const domain::TeedBallResult& result) const;
    cv::Mat CreateVisualization(const cv::Mat& original_image, const domain::TeedBallResult& result) const;
};

/**
 * @brief Factory for creating ImageAnalysis approval adapters
 */
class ImageAnalysisApprovalFactory {
public:
    /**
     * @brief Create standard ImageAnalysis approval adapter
     */
    static std::unique_ptr<ImageAnalysisApprovalAdapter> CreateStandard();
    
    /**
     * @brief Create fuzzy comparison adapter
     */
    static std::unique_ptr<ImageAnalysisApprovalAdapter> CreateWithFuzzyComparison(double tolerance = 0.01);
    
    /**
     * @brief Create compact formatting adapter
     */
    static std::unique_ptr<ImageAnalysisApprovalAdapter> CreateCompact();
};

} // namespace golf_sim::image_analysis::testing
