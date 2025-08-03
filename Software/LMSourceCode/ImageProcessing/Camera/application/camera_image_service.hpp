/*
 * Camera Image Processing Application Service
 * 
 * Handles format conversion and image processing operations
 * that were previously embedded in tests.
 */

#pragma once

#include "../domain/camera_domain.hpp"
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

namespace golf_sim::camera::application {

    struct ProcessedCameraImage {
        cv::Mat image;
        bool conversion_successful;
        std::string format_detected;
        std::string error_message;
    };

    struct ImageSaveResult {
        bool success;
        std::string filename;
        std::string error_message;
    };

    class CameraImageProcessor {
    public:
        // Convert camera frame to OpenCV Mat - extracts format detection logic from tests
        static ProcessedCameraImage ProcessCameraFrame(
            const golf_sim::camera::domain::CameraFrame& frame,
            const std::string& media_type_info);

        // Save processed image - extracts file I/O logic from tests  
        static ImageSaveResult SaveImage(
            const cv::Mat& image, 
            const std::string& base_filename,
            const std::string& output_directory = "captured_frames");

        // Extract dimensions from media type string - removes parsing logic from tests
        static std::pair<int, int> ExtractDimensionsFromMediaType(const std::string& media_info);

    private:
        static ProcessedCameraImage ConvertNV12Format(
            const golf_sim::camera::domain::CameraFrame& frame,
            int actual_width, int actual_height);
            
        static ProcessedCameraImage ConvertRGBAFormat(
            const golf_sim::camera::domain::CameraFrame& frame);
            
        static ProcessedCameraImage ConvertRGBFormat(
            const golf_sim::camera::domain::CameraFrame& frame);
    };

    class CameraTestReporter {
    public:
        // Handle test result reporting - removes UI logic from tests
        static void ReportCaptureResults(
            const std::vector<std::string>& captured_files,
            int successful_count);

        // Launch image viewer - removes system interaction from tests
        static bool LaunchImageViewer(const std::string& image_path);

    private:
        static std::string ConvertPathForWindows(const std::string& path);
    };

} // namespace golf_sim::camera::application
