/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2022-2025, Verdant Consultants, LLC.
 */

/**
 * @file test_approval_with_pitrac_images.cpp
 * @brief Approval tests using real PiTrac test images
 * 
 * This test file implements approval testing using the actual PiTrac test images
 * located at C:\kata\PiTrac\Software\LMSourceCode\Images\. Each test processes
 * a real golf ball image and generates received/approved artifacts for validation.
 */

#define BOOST_TEST_MODULE ApprovalTestsWithPiTracImages
#include <boost/test/unit_test.hpp>
#include "../application/image_analysis_service.hpp"
#include "../infrastructure/opencv_image_analyzer.hpp"
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <sstream>
#include <iomanip>

using namespace golf_sim::image_analysis;
namespace fs = std::filesystem;

BOOST_AUTO_TEST_SUITE(ApprovalTestsWithPiTracImages)

// Test configuration - using relative paths from build directory
const std::string PITRAC_IMAGES_DIR = "../../../Images/";
const std::string APPROVAL_ARTIFACTS_DIR = "approval_artifacts/";

// Approval test fixture
struct ApprovalTestFixture {
    ApprovalTestFixture() {
        // Create approval artifacts directory if it doesn't exist
        fs::create_directories(APPROVAL_ARTIFACTS_DIR);
        
        // Initialize the image analysis service with just an analyzer
        // For testing purposes, we'll use the simple constructor that just takes an analyzer
        analyzer = std::make_unique<infrastructure::OpenCVImageAnalyzer>();
        
        test_timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
        );
    }
    
    // Load image from PiTrac test images directory
    cv::Mat LoadPiTracImage(const std::string& filename) {
        std::string full_path = PITRAC_IMAGES_DIR + filename;
        cv::Mat image = cv::imread(full_path, cv::IMREAD_COLOR);
        
        if (image.empty()) {
            BOOST_FAIL("Failed to load PiTrac test image: " + full_path);
        }
        
        return image;
    }    // Create analysis result summary for approval
    std::string CreateTeedBallResultSummary(const domain::TeedBallResult& result) {
        std::ostringstream summary;
        
        summary << "=== Teed Ball Analysis Result Summary ===\n";
        summary << "Ball State: ";
        switch (result.state) {
            case domain::BallState::ABSENT: summary << "ABSENT"; break;
            case domain::BallState::TEED: summary << "TEED"; break;
            case domain::BallState::RESET: summary << "RESET"; break;
            case domain::BallState::MOVING: summary << "MOVING"; break;
            default: summary << "INVALID"; break;
        }
        summary << "\n";
        
        summary << "Has Ball: " << (result.HasBall() ? "YES" : "NO") << "\n";
        summary << "Confidence: " << std::fixed << std::setprecision(3) << result.confidence << "\n";
        summary << "Analysis Method: " << result.analysis_method << "\n";
        summary << "Confidence Level: ";
        
        auto confidence_level = result.GetConfidenceLevel();
        switch (confidence_level) {
            case domain::ConfidenceLevel::LOW: summary << "LOW"; break;
            case domain::ConfidenceLevel::MEDIUM: summary << "MEDIUM"; break;
            case domain::ConfidenceLevel::HIGH: summary << "HIGH"; break;
            case domain::ConfidenceLevel::VERY_HIGH: summary << "VERY_HIGH"; break;
            default: summary << "UNKNOWN"; break;
        }
        summary << "\n";
        
        if (result.position.has_value()) {
            const auto& pos = result.position.value();
            summary << "Ball Position:\n";
            summary << "  X: " << std::fixed << std::setprecision(2) << pos.x_pixels << " pixels\n";
            summary << "  Y: " << std::fixed << std::setprecision(2) << pos.y_pixels << " pixels\n";
            summary << "  Radius: " << std::fixed << std::setprecision(2) << pos.radius_pixels << " pixels\n";
            summary << "  Confidence: " << std::fixed << std::setprecision(3) << pos.confidence << "\n";
            summary << "  Detection Method: " << pos.detection_method << "\n";
            summary << "  Valid: " << (pos.IsValid() ? "YES" : "NO") << "\n";
        } else {
            summary << "Ball Position: NOT DETECTED\n";
        }
        
        if (!result.debug_info.empty()) {
            summary << "Debug Information:\n";
            for (const auto& debug : result.debug_info) {
                summary << "  - " << debug << "\n";
            }
        }
        
        summary << "============================================\n";
        
        return summary.str();
    }      // Save visualization image with detected ball highlighted
    void SaveVisualizationImage(const cv::Mat& original_image, 
                                const domain::TeedBallResult& result, 
                                const std::string& output_filename) {
        cv::Mat visualization = original_image.clone();
        
        if (result.HasBall() && result.position.has_value()) {
            const auto& pos = result.position.value();
            
            // Draw detected ball circle in green
            cv::Point center(static_cast<int>(pos.x_pixels), static_cast<int>(pos.y_pixels));
            int radius = static_cast<int>(pos.radius_pixels);
            
            cv::circle(visualization, center, radius, cv::Scalar(0, 255, 0), 2);  // Green circle
            cv::circle(visualization, center, 2, cv::Scalar(0, 255, 0), -1);      // Green center dot
            
            // Add confidence text
            std::string confidence_text = "Conf: " + std::to_string(pos.confidence).substr(0, 5);
            cv::putText(visualization, confidence_text, 
                       cv::Point(center.x + radius + 5, center.y), 
                       cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);
            
            // Add ball state text
            std::string state_text = "State: ";
            switch (result.state) {
                case domain::BallState::TEED: state_text += "TEED"; break;
                case domain::BallState::RESET: state_text += "RESET"; break;
                case domain::BallState::MOVING: state_text += "MOVING"; break;
                default: state_text += "OTHER"; break;
            }
            cv::putText(visualization, state_text, 
                       cv::Point(center.x + radius + 5, center.y + 20), 
                       cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);
        } else {
            // Add "NO BALL DETECTED" text if no ball found
            cv::putText(visualization, "NO BALL DETECTED",
                       cv::Point(20, 40), 
                       cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 255), 2);
        }
          // Save visualization
        std::string viz_path = APPROVAL_ARTIFACTS_DIR + output_filename;
        cv::imwrite(viz_path, visualization);
    }    // Helper function to compare two images for approval testing
    bool CompareImages(const std::string& image1_path, const std::string& image2_path) {
        if (!fs::exists(image1_path) || !fs::exists(image2_path)) {
            return false;
        }
        
        cv::Mat img1 = cv::imread(image1_path, cv::IMREAD_COLOR);
        cv::Mat img2 = cv::imread(image2_path, cv::IMREAD_COLOR);
        
        if (img1.empty() || img2.empty()) {
            return false;
        }
        
        // Helper function to check if an image is completely black (empty baseline)
        auto isEmptyBaseline = [](const cv::Mat& img) {
            cv::Scalar mean_val = cv::mean(img);
            return (mean_val[0] == 0 && mean_val[1] == 0 && mean_val[2] == 0);
        };
        
        // Special case: Check if one image is an empty baseline (all black pixels)
        // This indicates a missing baseline that was created as placeholder
        if (isEmptyBaseline(img1) || isEmptyBaseline(img2)) {
            return false; // Empty baseline should always fail comparison
        }
        
        // Check if dimensions match
        if (img1.size() != img2.size()) {
            return false;
        }
        
        // Check if images are identical
        cv::Mat diff;
        cv::absdiff(img1, img2, diff);
        
        // Convert to grayscale for easier analysis
        cv::Mat gray_diff;
        cv::cvtColor(diff, gray_diff, cv::COLOR_BGR2GRAY);
        
        // Check if there are any non-zero differences
        double min_val, max_val;
        cv::minMaxLoc(gray_diff, &min_val, &max_val);
        
        // Images are identical if max difference is 0
        return max_val == 0;
    }
      // Core approval test method
    void RunApprovalTest(const std::string& image_filename, const std::string& test_name) {
        // Load the PiTrac test image
        cv::Mat test_image = LoadPiTracImage(image_filename);
        
        // Create image buffer for analysis
        domain::ImageBuffer image_buffer(test_image, test_timestamp, test_name);
        
        // Perform analysis using the analyzer directly
        auto result = analyzer->AnalyzeTeedBall(image_buffer);
        
        // Generate analysis result summary
        std::string summary = CreateTeedBallResultSummary(result);
        
        // Save received artifact (current test run)
        std::string received_filename = test_name + ".received.txt";
        std::string received_path = APPROVAL_ARTIFACTS_DIR + received_filename;
        std::ofstream received_file(received_path);
        received_file << summary;
        received_file.close();
        
        // Save visualization image
        std::string viz_filename = test_name + ".received.png";
        SaveVisualizationImage(test_image, result, viz_filename);
          // Check if approved artifact exists
        std::string approved_filename = test_name + ".approved.txt";
        std::string approved_path = APPROVAL_ARTIFACTS_DIR + approved_filename;
        std::string approved_viz_filename = test_name + ".approved.png";
        std::string approved_viz_path = APPROVAL_ARTIFACTS_DIR + approved_viz_filename;
          if (fs::exists(approved_path) && fs::exists(approved_viz_path)) {
            // Read approved content
            std::ifstream approved_file(approved_path);
            std::string approved_content((std::istreambuf_iterator<char>(approved_file)),
                                        std::istreambuf_iterator<char>());
            
            // Compare received vs approved text
            bool text_matches = (summary == approved_content);
            
            // Compare received vs approved images
            std::string viz_received_path = APPROVAL_ARTIFACTS_DIR + viz_filename;
            bool images_match = CompareImages(approved_viz_path, viz_received_path);
            
            if (!text_matches || !images_match) {
                // Launch VS Code diff for review
                LaunchVSCodeDiff(received_path, approved_path, test_name, false);
                
                std::string failure_msg = "Approval test failed for " + test_name + "\n";
                if (!text_matches) {
                    failure_msg += "Text content differs between approved and received files.\n";
                }
                if (!images_match) {
                    failure_msg += "Image content differs between approved and received files.\n";
                }
                failure_msg += "Received file: " + received_path + "\n";
                failure_msg += "Approved file: " + approved_path + "\n";
                failure_msg += "VS Code diff launched for review. Check the differences and approve if intended.";
                
                BOOST_FAIL(failure_msg);
            } else {
                // Test passed - optionally clean up received files
                BOOST_TEST_MESSAGE("Approval test passed for " + test_name);
            }} else {            // Check for partial baseline (missing files)
            if (fs::exists(approved_path) && !fs::exists(approved_viz_path)) {
                // Load received image to get proper dimensions for empty baseline
                std::string viz_received_path = APPROVAL_ARTIFACTS_DIR + viz_filename;
                cv::Mat received_img = cv::imread(viz_received_path, cv::IMREAD_COLOR);
                
                // Create empty approved PNG file for comparison with proper dimensions
                cv::Mat empty_image;
                if (!received_img.empty()) {
                    empty_image = cv::Mat::zeros(received_img.size(), CV_8UC3);
                } else {
                    empty_image = cv::Mat::zeros(480, 640, CV_8UC3); // Default size if received image fails to load
                }
                cv::imwrite(approved_viz_path, empty_image);
                
                // Launch VS Code to show both images (empty approved vs received)
                std::string img_diff_command = "code \"" + approved_viz_path + "\" \"" + viz_received_path + "\"";
                system(img_diff_command.c_str());
                
                BOOST_FAIL("Missing approved visualization file created as empty baseline: " + approved_viz_path + 
                          "\nText baseline exists but image baseline was missing." +
                          "\nVS Code opened to compare empty approved vs received image." +
                          "\nTo approve: copy \"" + viz_received_path + "\" \"" + approved_viz_path + "\"");
            }if (!fs::exists(approved_path) && fs::exists(approved_viz_path)) {
                // Create empty approved text file for comparison
                std::ofstream empty_file(approved_path);
                empty_file << "# Empty baseline - no text analysis result exists yet\n";
                empty_file << "# Review the received content and approve if correct\n";
                empty_file.close();
                
                // Launch diff to show what's missing for text file
                LaunchVSCodeDiff(received_path, approved_path, test_name + "_missing_text", false);
                
                BOOST_FAIL("Missing approved text file created as empty baseline: " + approved_path + 
                          "\nImage baseline exists but text baseline was missing." +
                          "\nVS Code diff launched to review the received content.");
            }
            
            // No approved files exist - this is the first run (new test)
            if (!fs::exists(approved_path) && !fs::exists(approved_viz_path)) {
                // Launch diff to review new baseline for text
                LaunchVSCodeDiff(received_path, approved_path, test_name + "_new_baseline", true);
                
                // Copy received to approved to create baseline
                std::string copy_command = "copy \"" + received_path + "\" \"" + approved_path + "\"";
                system(copy_command.c_str());
                
                // Also copy visualization
                std::string copy_viz_command = "copy \"" + APPROVAL_ARTIFACTS_DIR + viz_filename + "\" \"" + APPROVAL_ARTIFACTS_DIR + approved_viz_filename + "\"";
                system(copy_viz_command.c_str());
                
                BOOST_TEST_MESSAGE("Created baseline approved files for " + test_name);
                BOOST_TEST_MESSAGE("VS Code diff launched to review the new baseline.");
                BOOST_TEST_MESSAGE("Received: " + received_path);
                BOOST_TEST_MESSAGE("Approved: " + approved_path);
            }
        }}
    
    // Launch VS Code diff tool for approval workflow
    void LaunchVSCodeDiff(const std::string& received_path, const std::string& approved_path, 
                         const std::string& test_name, bool is_baseline_missing = false) {
        // Check if we're in a CI environment (skip interactive diff)
        if (std::getenv("CI") || std::getenv("GITHUB_ACTIONS") || std::getenv("TF_BUILD")) {
            BOOST_TEST_MESSAGE("CI environment detected - skipping interactive diff for " + test_name);
            return;
        }
        
        BOOST_TEST_MESSAGE("Launching VS Code diff for " + test_name + "...");
        
        if (is_baseline_missing) {
            // Create empty baseline file for comparison
            std::string empty_baseline = approved_path + ".empty";
            std::ofstream empty_file(empty_baseline);
            empty_file << "# This is a new test - no baseline exists yet\n";
            empty_file << "# Review the received content and approve if correct\n";
            empty_file.close();
            
            // Launch diff with empty baseline
            std::string diff_command = "code --diff \"" + empty_baseline + "\" \"" + received_path + "\"";
            system(diff_command.c_str());
            
            BOOST_TEST_MESSAGE("To approve this baseline, run:");
            BOOST_TEST_MESSAGE("  copy \"" + received_path + "\" \"" + approved_path + "\"");
        } else {
            // Launch diff between approved and received
            std::string diff_command = "code --diff \"" + approved_path + "\" \"" + received_path + "\"";
            system(diff_command.c_str());
            
            BOOST_TEST_MESSAGE("To approve changes, run:");
            BOOST_TEST_MESSAGE("  copy \"" + received_path + "\" \"" + approved_path + "\"");
        }
        
        // Also open image files if they exist
        std::string received_img = received_path;
        std::string approved_img = approved_path;
        
        // Replace .txt with .png for image paths
        size_t txt_pos = received_img.find(".txt");
        if (txt_pos != std::string::npos) {
            received_img.replace(txt_pos, 4, ".png");
            approved_img.replace(approved_img.find(".txt"), 4, ".png");
            
            if (fs::exists(received_img)) {
                std::string img_command = "code \"" + received_img + "\"";
                system(img_command.c_str());
                
                if (!is_baseline_missing && fs::exists(approved_img)) {
                    std::string approved_img_command = "code \"" + approved_img + "\"";
                    system(approved_img_command.c_str());
                }            }
        }
    }
    
    std::unique_ptr<infrastructure::OpenCVImageAnalyzer> analyzer;
    std::chrono::microseconds test_timestamp;
};

// Test cases using real PiTrac images
BOOST_FIXTURE_TEST_CASE(test_log_ball_final_found_ball_img, ApprovalTestFixture) {
    RunApprovalTest("log_ball_final_found_ball_img.png", "log_ball_final_found_ball_img");
}

BOOST_FIXTURE_TEST_CASE(test_gs_log_img_log_ball_final_found_ball_img, ApprovalTestFixture) {
    RunApprovalTest("gs_log_img__log_ball_final_found_ball_img.png", "gs_log_img_log_ball_final_found_ball_img");
}

BOOST_FIXTURE_TEST_CASE(test_log_cam2_last_strobed_img, ApprovalTestFixture) {
    RunApprovalTest("log_cam2_last_strobed_img.png", "log_cam2_last_strobed_img");
}

BOOST_FIXTURE_TEST_CASE(test_log_cam2_last_strobed_img_232_fast, ApprovalTestFixture) {
    RunApprovalTest("log_cam2_last_strobed_img_232_fast.png", "log_cam2_last_strobed_img_232_fast");
}

BOOST_FIXTURE_TEST_CASE(test_spin_ball_1_gray_image1, ApprovalTestFixture) {
    RunApprovalTest("spin_ball_1_gray_image1.png", "spin_ball_1_gray_image1");
}

BOOST_FIXTURE_TEST_CASE(test_spin_ball_2_gray_image1, ApprovalTestFixture) {
    RunApprovalTest("spin_ball_2_gray_image1.png", "spin_ball_2_gray_image1");
}

BOOST_FIXTURE_TEST_CASE(test_log_ball_final_found_ball_img_232_fast, ApprovalTestFixture) {
    RunApprovalTest("log_ball_final_found_ball_img_232_fast.png", "log_ball_final_found_ball_img_232_fast");
}

// Integration test for movement analysis using multiple images
BOOST_FIXTURE_TEST_CASE(test_movement_analysis_with_strobed_images, ApprovalTestFixture) {
    // Load both strobed images for movement analysis
    cv::Mat strobed_img1 = LoadPiTracImage("log_cam2_last_strobed_img.png");
    cv::Mat strobed_img2 = LoadPiTracImage("log_cam2_last_strobed_img_232_fast.png");
    
    // Create image sequence
    std::vector<domain::ImageBuffer> sequence;
    sequence.emplace_back(strobed_img1, test_timestamp, "strobed_sequence_1");
    sequence.emplace_back(strobed_img2, test_timestamp + std::chrono::microseconds(33333), "strobed_sequence_2");
    
    // First get ball position from the first image to use as reference
    auto first_result = analyzer->AnalyzeTeedBall(sequence[0]);
    domain::BallPosition reference_position;
    if (first_result.position.has_value()) {
        reference_position = first_result.position.value();
    } else {
        // Create a default position if no ball detected
        reference_position = domain::BallPosition(320, 240, 20, 0.5, test_timestamp, "default");
    }
    
    // Analyze movement
    auto movement_result = analyzer->DetectMovement(sequence, reference_position);
      // Create movement analysis summary
    std::ostringstream summary;
    summary << "=== Movement Analysis Result ===\n";
    summary << "Movement Detected: " << (movement_result.movement_detected ? "YES" : "NO") << "\n";
    summary << "Confidence: " << std::fixed << std::setprecision(3) << movement_result.movement_confidence << "\n";
    summary << "Movement Magnitude: " << movement_result.movement_magnitude << "\n";
    summary << "Time Since First Movement: " << movement_result.time_since_first_movement.count() << " microseconds\n";
    summary << "Analysis Method: " << movement_result.analysis_method << "\n";
    summary << "Motion Vectors: " << movement_result.motion_vectors.size() << "\n";
    
    if (movement_result.last_known_position.has_value()) {
        const auto& pos = movement_result.last_known_position.value();
        summary << "Last Known Position:\n";
        summary << "  X: " << pos.x_pixels << " pixels\n";
        summary << "  Y: " << pos.y_pixels << " pixels\n";
        summary << "  Radius: " << pos.radius_pixels << " pixels\n";
        summary << "  Confidence: " << pos.confidence << "\n";
    }
    
    summary << "================================\n";
    
    // Save as approval test
    std::string test_name = "movement_analysis_strobed_sequence";
    std::string received_path = APPROVAL_ARTIFACTS_DIR + test_name + ".received.txt";
    std::ofstream received_file(received_path);
    received_file << summary.str();
    received_file.close();
    
    // Create baseline if needed or compare
    std::string approved_path = APPROVAL_ARTIFACTS_DIR + test_name + ".approved.txt";
    if (!fs::exists(approved_path)) {
        std::string copy_command = "copy \"" + received_path + "\" \"" + approved_path + "\"";
        system(copy_command.c_str());
        BOOST_TEST_MESSAGE("Created baseline for movement analysis test");
    } else {
        std::ifstream approved_file(approved_path);
        std::string approved_content((std::istreambuf_iterator<char>(approved_file)),
                                    std::istreambuf_iterator<char>());
        BOOST_CHECK_EQUAL(summary.str(), approved_content);
    }
}

BOOST_AUTO_TEST_SUITE_END()
