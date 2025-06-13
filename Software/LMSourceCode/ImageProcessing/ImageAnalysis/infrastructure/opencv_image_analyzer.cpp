/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2022-2025, Verdant Consultants, LLC.
 */

/**
 * @file opencv_image_analyzer.cpp
 * @brief OpenCV implementation of image analysis interface
 */

#include "opencv_image_analyzer.hpp"
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <algorithm>

namespace golf_sim::image_analysis::infrastructure {

using namespace domain;

// Simple logging functions to replace dependencies
static void LogInfo(const std::string& message) {
    std::cout << "[INFO] " << message << std::endl;
}

static void LogError(const std::string& message) {
    std::cerr << "[ERROR] " << message << std::endl;
}

OpenCVImageAnalyzer::OpenCVImageAnalyzer() {
    LogInfo("OpenCV Image Analyzer initialized");
}

TeedBallResult OpenCVImageAnalyzer::AnalyzeTeedBall(
    const ImageBuffer& image, 
    const std::optional<BallPosition>& expected_position
) {
    if (!image.IsValid()) {
        return CreateErrorResult("Invalid image buffer");
    }

    try {
        cv::Mat processed = PreprocessImage(image.data);
        std::vector<BallPosition> candidates = DetectCircles(processed);
        
        if (candidates.empty()) {
            TeedBallResult result;
            result.state = BallState::ABSENT;
            result.confidence = 0.0;
            result.analysis_method = "opencv_hough_circles";
            result.debug_info.push_back("No circles detected");
            return result;
        }

        BallPosition best = SelectBestCandidate(candidates, expected_position);
        
        TeedBallResult result;
        result.state = (best.confidence >= 0.5) ? BallState::TEED : BallState::ABSENT;
        result.position = best;
        result.confidence = best.confidence;
        result.analysis_method = "opencv_hough_circles";
        result.debug_info.push_back("Detected " + std::to_string(candidates.size()) + " circles");
        
        return result;

    } catch (const std::exception& e) {
        return CreateErrorResult("Exception: " + std::string(e.what()));
    }
}

MovementResult OpenCVImageAnalyzer::DetectMovement(
    const std::vector<ImageBuffer>& image_sequence,
    const BallPosition& reference_position
) {
    if (image_sequence.size() < 2) {
        return CreateMovementErrorResult("Insufficient images for movement detection");
    }

    try {
        MovementResult result;
        result.analysis_method = "opencv_optical_flow";

        // Simple movement detection using frame differencing
        cv::Mat prev_frame = PreprocessImage(image_sequence[0].data);
        double max_movement = 0.0;

        for (size_t i = 1; i < image_sequence.size(); ++i) {
            cv::Mat curr_frame = PreprocessImage(image_sequence[i].data);
            
            // Calculate optical flow
            std::vector<cv::Point2f> flow = CalculateOpticalFlow(prev_frame, curr_frame);
            double movement = CalculateMovementMagnitude(flow);
            
            if (movement > max_movement) {
                max_movement = movement;
            }
            
            prev_frame = curr_frame;
        }        const double movement_threshold = 2.0;  // Lower threshold for better sensitivity
        result.movement_detected = max_movement > movement_threshold;
        result.movement_confidence = std::min(1.0, max_movement / 20.0);  // Adjusted scaling
        result.movement_magnitude = max_movement;
        result.last_known_position = reference_position;

        return result;

    } catch (const std::exception& e) {
        return CreateMovementErrorResult("Exception: " + std::string(e.what()));
    }
}

FlightAnalysisResult OpenCVImageAnalyzer::AnalyzeBallFlight(
    const ImageBuffer& strobed_image,
    const BallPosition& calibration_reference
) {
    // calibration_reference is reserved for future flight path calibration
    (void)calibration_reference;  // Suppress unused parameter warning
    
    if (!strobed_image.IsValid()) {
        return CreateFlightErrorResult("Invalid strobed image");
    }

    try {
        cv::Mat processed = PreprocessImage(strobed_image.data);
        std::vector<BallPosition> candidates = DetectCircles(processed);
        
        FlightAnalysisResult result;
        result.analysis_method = "opencv_multi_ball_detection";
        
        // Filter candidates with reasonable confidence
        for (const auto& candidate : candidates) {
            if (candidate.confidence >= 0.3) {
                result.detected_balls.push_back(candidate);
            }
        }

        // Sort by x-position (flight trajectory)
        std::sort(result.detected_balls.begin(), result.detected_balls.end(),
                  [](const BallPosition& a, const BallPosition& b) {
                      return a.x_pixels < b.x_pixels;
                  });

        if (result.detected_balls.size() >= 2) {
            result.confidence = 0.8;
            result.temporal_spacing_us = 5000.0; // 5ms assumption
            
            // Basic velocity calculation
            const auto& first = result.detected_balls.front();
            const auto& last = result.detected_balls.back();
            
            double dx = last.x_pixels - first.x_pixels;
            double dy = last.y_pixels - first.y_pixels;
            double dt = result.temporal_spacing_us * (result.detected_balls.size() - 1);
            
            if (dt > 0) {
                double velocity_x = (dx / dt) * 1000000 * 0.001; // Convert to m/s
                double velocity_y = (dy / dt) * 1000000 * 0.001;
                result.velocity_vector = cv::Vec3d{velocity_x, velocity_y, 0.0};
            }
        }

        return result;

    } catch (const std::exception& e) {
        return CreateFlightErrorResult("Exception: " + std::string(e.what()));
    }
}

TeedBallResult OpenCVImageAnalyzer::DetectBallReset(
    const ImageBuffer& current_image,
    const BallPosition& previous_position
) {
    TeedBallResult result = AnalyzeTeedBall(current_image);
    
    if (result.position.has_value()) {
        double distance = result.position->DistanceFrom(previous_position);
        
        if (distance > 100.0) { // 100 pixel threshold
            result.state = BallState::RESET;
            result.debug_info.push_back("Ball position significantly changed - possible reset");
        }
    }
    
    result.analysis_method = "opencv_reset_detection";
    return result;
}

void OpenCVImageAnalyzer::SetHoughParameters(double param1, double param2, double dp) {
    hough_param1_ = param1;
    hough_param2_ = param2;
    hough_dp_ = dp;
    LogInfo("Hough parameters updated");
}

void OpenCVImageAnalyzer::SetRadiusLimits(int min_radius, int max_radius) {
    min_radius_ = min_radius;
    max_radius_ = max_radius;
    LogInfo("Radius limits updated: " + std::to_string(min_radius) + "-" + std::to_string(max_radius));
}

// Private helper methods
std::vector<BallPosition> OpenCVImageAnalyzer::DetectCircles(const cv::Mat& image) const {
    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(
        image,
        circles,
        cv::HOUGH_GRADIENT,
        hough_dp_,
        image.rows / 4,
        hough_param1_,
        hough_param2_,
        min_radius_,
        max_radius_
    );

    std::vector<BallPosition> positions;
    for (const auto& circle : circles) {
        double confidence = CalculateConfidence(
            BallPosition{circle[0], circle[1], circle[2]}, image);
        
        positions.emplace_back(
            static_cast<double>(circle[0]),
            static_cast<double>(circle[1]),
            static_cast<double>(circle[2]),
            confidence,
            std::chrono::microseconds{0},
            "opencv_hough"
        );
    }

    return positions;
}

BallPosition OpenCVImageAnalyzer::SelectBestCandidate(
    const std::vector<BallPosition>& candidates,
    const std::optional<BallPosition>& expected_position) const {
    
    if (candidates.empty()) {
        return BallPosition{}; // Default constructed
    }
    
    BallPosition best = candidates[0];
    
    if (expected_position.has_value()) {
        // Find candidate closest to expected position
        double min_distance = best.DistanceFrom(*expected_position);
        
        for (const auto& candidate : candidates) {
            double distance = candidate.DistanceFrom(*expected_position);
            if (distance < min_distance) {
                best = candidate;
                min_distance = distance;
            }
        }
    } else {
        // Find candidate with highest confidence
        for (const auto& candidate : candidates) {
            if (candidate.confidence > best.confidence) {
                best = candidate;
            }
        }
    }
    
    return best;
}

double OpenCVImageAnalyzer::CalculateConfidence(const BallPosition& position, const cv::Mat& image) const {
    // Basic confidence calculation
    double x = position.x_pixels;
    double y = position.y_pixels;
    double radius = position.radius_pixels;
    
    // Check bounds
    if (x - radius < 0 || x + radius >= image.cols ||
        y - radius < 0 || y + radius >= image.rows) {
        return 0.1;
    }
    
    // Basic confidence based on radius
    double radius_confidence = 1.0;
    if (radius < min_radius_ || radius > max_radius_) {
        radius_confidence = 0.5;
    }
    
    return std::min(1.0, radius_confidence * 0.8);
}

bool OpenCVImageAnalyzer::IsValidBallPosition(const BallPosition& position, const cv::Mat& image) const {
    return position.x_pixels >= 0 && position.x_pixels < image.cols &&
           position.y_pixels >= 0 && position.y_pixels < image.rows &&
           position.radius_pixels > 0 && position.confidence > 0.0;
}

cv::Mat OpenCVImageAnalyzer::PreprocessImage(const cv::Mat& input) const {
    cv::Mat gray, blurred;
    
    if (input.channels() == 3) {
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = input.clone();
    }
    
    cv::GaussianBlur(gray, blurred, cv::Size(9, 9), 2, 2);
    return blurred;
}

std::vector<cv::Point2f> OpenCVImageAnalyzer::CalculateOpticalFlow(
    const cv::Mat& prev_frame, const cv::Mat& curr_frame) const {
    
    std::vector<cv::Point2f> flow;
    
    try {
        // Use Lucas-Kanade optical flow
        std::vector<cv::Point2f> prev_points, next_points;
        std::vector<uchar> status;
        std::vector<float> error;
        
        // Find corner points in previous frame
        cv::goodFeaturesToTrack(prev_frame, prev_points, 100, 0.3, 7, cv::Mat(), 7, false, 0.04);
        
        if (!prev_points.empty()) {
            // Calculate optical flow
            cv::calcOpticalFlowPyrLK(prev_frame, curr_frame, prev_points, next_points, status, error);
            
            // Extract valid flow vectors
            for (size_t i = 0; i < prev_points.size(); ++i) {
                if (status[i] && error[i] < 50) {
                    cv::Point2f flow_vector = next_points[i] - prev_points[i];
                    flow.push_back(flow_vector);
                }
            }
        }
        
        // Fallback to simple frame difference if no good features found
        if (flow.empty()) {
            cv::Mat diff;
            cv::absdiff(prev_frame, curr_frame, diff);
            cv::Scalar mean_diff = cv::mean(diff);
            
            // Generate representative flow vectors based on mean difference
            if (mean_diff[0] > 1.0) {  // Lower threshold for better sensitivity
                flow.push_back(cv::Point2f{static_cast<float>(mean_diff[0]), 0.0f});
            }        }
    } catch (const cv::Exception& e) {
        // Log exception details if needed in debug builds
        (void)e;  // Suppress unused parameter warning
        
        // Fallback to simple difference
        cv::Mat diff;
        cv::absdiff(prev_frame, curr_frame, diff);
        cv::Scalar mean_diff = cv::mean(diff);
        
        if (mean_diff[0] > 1.0) {
            flow.push_back(cv::Point2f{static_cast<float>(mean_diff[0]), 0.0f});
        }
    }
    
    return flow;
}

double OpenCVImageAnalyzer::CalculateMovementMagnitude(const std::vector<cv::Point2f>& flow) const {
    if (flow.empty()) return 0.0;
    
    double total = 0.0;
    for (const auto& vector : flow) {
        total += std::sqrt(vector.x * vector.x + vector.y * vector.y);
    }
    
    return total / flow.size();
}

TeedBallResult OpenCVImageAnalyzer::CreateErrorResult(const std::string& error_message) const {
    TeedBallResult result;
    result.state = BallState::ABSENT;
    result.confidence = 0.0;
    result.analysis_method = "opencv_error";
    result.debug_info.push_back(error_message);
    LogError(error_message);
    return result;
}

MovementResult OpenCVImageAnalyzer::CreateMovementErrorResult(const std::string& error_message) const {
    MovementResult result;
    result.movement_detected = false;
    result.movement_confidence = 0.0;
    result.analysis_method = "opencv_error";
    result.debug_info.push_back(error_message);
    LogError(error_message);
    return result;
}

FlightAnalysisResult OpenCVImageAnalyzer::CreateFlightErrorResult(const std::string& error_message) const {
    FlightAnalysisResult result;
    result.confidence = 0.0;
    result.analysis_method = "opencv_error";
    result.debug_info.push_back(error_message);
    LogError(error_message);
    return result;
}

} // namespace golf_sim::image_analysis::infrastructure