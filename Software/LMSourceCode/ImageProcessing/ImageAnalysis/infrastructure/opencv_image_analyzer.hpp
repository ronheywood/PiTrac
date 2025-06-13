/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2022-2025, Verdant Consultants, LLC.
 */

/**
 * @file opencv_image_analyzer.hpp
 * @brief OpenCV-based implementation of image analysis interface
 * 
 * Wraps the existing sophisticated OpenCV ball detection logic
 * (BallImageProc and GolfSimCamera) behind the domain interface.
 * This preserves all the complex Hough circle detection, strobed
 * ball analysis, and movement detection while providing a clean
 * abstraction for future AI/ML integration.
 */

#pragma once

#include "../domain/interfaces.hpp"
#include "../domain/analysis_results.hpp"
#include <opencv2/opencv.hpp>
// #include "../../ball_image_proc.h"    // Existing OpenCV implementation - temporarily disabled
// #include "../../golf_ball.h"          // Existing GolfBall class - temporarily disabled  
// #include "../../gs_camera.h"          // Existing camera implementation - temporarily disabled

namespace golf_sim::image_analysis::infrastructure {

    /**
     * @brief Simplified OpenCV-based implementation of IImageAnalyzer
     * 
     * This implementation provides basic OpenCV-based ball detection
     * using Hough circle detection. It serves as a working prototype
     * for the bounded context while the full integration with existing
     * BallImageProc can be completed later.
     */
    class OpenCVImageAnalyzer : public domain::IImageAnalyzer {
    public:
        OpenCVImageAnalyzer();
        ~OpenCVImageAnalyzer() override = default;

        // Domain interface implementation
        domain::TeedBallResult AnalyzeTeedBall(
            const domain::ImageBuffer& image,
            const std::optional<domain::BallPosition>& expected_position = std::nullopt
        ) override;

        domain::MovementResult DetectMovement(
            const std::vector<domain::ImageBuffer>& image_sequence,
            const domain::BallPosition& reference_ball_position
        ) override;

        domain::FlightAnalysisResult AnalyzeBallFlight(
            const domain::ImageBuffer& strobed_image,
            const domain::BallPosition& calibration_reference
        ) override;

        domain::TeedBallResult DetectBallReset(
            const domain::ImageBuffer& current_image,
            const domain::BallPosition& previous_ball_position
        ) override;

        // Analyzer metadata
        std::string GetAnalyzerName() const override { return "OpenCV Image Analyzer"; }
        std::string GetVersion() const override { return "1.0.0-simplified"; }
        bool SupportsRealTime() const override { return true; }

        // OpenCV-specific configuration
        void SetHoughParameters(double param1, double param2, double dp = 1.0);
        void SetRadiusLimits(int min_radius, int max_radius);

    private:
        // Configuration parameters for Hough Circle detection
        double hough_param1_ = 100.0;
        double hough_param2_ = 30.0;
        double hough_dp_ = 1.0;
        int min_radius_ = 10;
        int max_radius_ = 100;
        
        // Core detection methods
        std::vector<domain::BallPosition> DetectCircles(const cv::Mat& image) const;
        domain::BallPosition SelectBestCandidate(
            const std::vector<domain::BallPosition>& candidates,
            const std::optional<domain::BallPosition>& expected_position) const;
        
        // Helper methods
        double CalculateConfidence(const domain::BallPosition& position, const cv::Mat& image) const;
        bool IsValidBallPosition(const domain::BallPosition& position, const cv::Mat& image) const;
        cv::Mat PreprocessImage(const cv::Mat& input) const;
        
        // Movement detection helpers
        std::vector<cv::Point2f> CalculateOpticalFlow(
            const cv::Mat& prev_frame, const cv::Mat& curr_frame) const;
        double CalculateMovementMagnitude(const std::vector<cv::Point2f>& flow) const;
        
        // Error result creators
        domain::TeedBallResult CreateErrorResult(const std::string& error_message) const;
        domain::MovementResult CreateMovementErrorResult(const std::string& error_message) const;
        domain::FlightAnalysisResult CreateFlightErrorResult(const std::string& error_message) const;
    };

} // namespace golf_sim::image_analysis::infrastructure
