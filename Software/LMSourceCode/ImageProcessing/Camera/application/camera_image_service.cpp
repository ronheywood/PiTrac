/*
 * Camera Image Processing Application Service Implementation
 */

#include "camera_image_service.hpp"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <cstdio>
#include <iostream>

namespace golf_sim::camera::application {

    ProcessedCameraImage CameraImageProcessor::ProcessCameraFrame(
        const golf_sim::camera::domain::CameraFrame& frame,
        const std::string& media_type_info) {
        
        if (frame.data.empty()) {
            return {cv::Mat{}, false, "unknown", "Frame data is empty"};
        }

        // Extract actual dimensions from media type
        auto [actual_width, actual_height] = ExtractDimensionsFromMediaType(media_type_info);
        
        // Try NV12 format first (common camera format)
        if (media_type_info.find("NV12") != std::string::npos) {
            return ConvertNV12Format(frame, actual_width, actual_height);
        }

        // Try RGBA/BGRA format
        size_t expected_rgba = frame.resolution.width * frame.resolution.height * 4;
        if (frame.data.size() == expected_rgba) {
            return ConvertRGBAFormat(frame);
        }

        // Try RGB format
        size_t expected_rgb = frame.resolution.width * frame.resolution.height * 3;
        if (frame.data.size() == expected_rgb) {
            return ConvertRGBFormat(frame);
        }

        return {cv::Mat{}, false, "unknown", 
                "Unsupported format. Data size: " + std::to_string(frame.data.size()) + 
                " bytes, Media type: " + media_type_info};
    }

    ImageSaveResult CameraImageProcessor::SaveImage(
        const cv::Mat& image, 
        const std::string& base_filename,
        const std::string& output_directory) {
        
        if (image.empty()) {
            return {false, "", "Image is empty"};
        }

        // Create output directory
        std::filesystem::create_directories(output_directory);
        
        // Build full filename
        std::string full_path = output_directory + "/" + base_filename + ".jpg";
        
        // Save image
        bool success = cv::imwrite(full_path, image);
        
        return {success, full_path, success ? "" : "OpenCV imwrite failed"};
    }

    std::pair<int, int> CameraImageProcessor::ExtractDimensionsFromMediaType(const std::string& media_info) {
        int width = 640, height = 480;  // defaults
        
        size_t size_pos = media_info.find("Size: ");
        if (size_pos != std::string::npos) {
            std::string size_str = media_info.substr(size_pos + 6);
            sscanf(size_str.c_str(), "%dx%d", &width, &height);
        }
        
        return {width, height};
    }

    ProcessedCameraImage CameraImageProcessor::ConvertNV12Format(
        const golf_sim::camera::domain::CameraFrame& frame,
        int actual_width, int actual_height) {
        
        size_t expected_nv12 = actual_width * actual_height * 3 / 2;
        if (frame.data.size() != expected_nv12) {
            return {cv::Mat{}, false, "NV12", "Size mismatch for NV12 format"};
        }

        try {
            // Create Y plane
            cv::Mat y_plane(actual_height, actual_width, CV_8UC1, 
                           const_cast<uint8_t*>(frame.data.data()));
            
            // Create UV plane (half width, half height, 2 channels interleaved)
            cv::Mat uv_plane(actual_height / 2, actual_width / 2, CV_8UC2, 
                            const_cast<uint8_t*>(frame.data.data()) + actual_width * actual_height);
            
            // Convert NV12 to BGR
            cv::Mat yuv_mat;
            cv::vconcat(y_plane, uv_plane.reshape(1, actual_height / 2), yuv_mat);
            
            cv::Mat bgr_image;
            cv::cvtColor(yuv_mat, bgr_image, cv::COLOR_YUV2BGR_NV12);
            
            return {bgr_image, true, "NV12", ""};
        } catch (const cv::Exception& e) {
            return {cv::Mat{}, false, "NV12", "OpenCV conversion error: " + std::string(e.what())};
        }
    }

    ProcessedCameraImage CameraImageProcessor::ConvertRGBAFormat(
        const golf_sim::camera::domain::CameraFrame& frame) {
        
        try {
            cv::Mat rgba_image(frame.resolution.height, frame.resolution.width, CV_8UC4, 
                             const_cast<uint8_t*>(frame.data.data()));
            
            cv::Mat bgr_image;
            cv::cvtColor(rgba_image, bgr_image, cv::COLOR_BGRA2BGR);
            
            return {bgr_image, true, "BGRA", ""};
        } catch (const cv::Exception& e) {
            return {cv::Mat{}, false, "BGRA", "OpenCV conversion error: " + std::string(e.what())};
        }
    }

    ProcessedCameraImage CameraImageProcessor::ConvertRGBFormat(
        const golf_sim::camera::domain::CameraFrame& frame) {
        
        try {
            cv::Mat rgb_image(frame.resolution.height, frame.resolution.width, CV_8UC3, 
                            const_cast<uint8_t*>(frame.data.data()));
            
            return {rgb_image.clone(), true, "RGB", ""};
        } catch (const cv::Exception& e) {
            return {cv::Mat{}, false, "RGB", "OpenCV conversion error: " + std::string(e.what())};
        }
    }

    void CameraTestReporter::ReportCaptureResults(
        const std::vector<std::string>& captured_files,
        int successful_count) {
        
        std::cout << "=== Image Capture Results ===\n";
        std::cout << "Successfully captured " << successful_count << " images\n";
        
        if (captured_files.empty()) {
            std::cout << "No images were captured - all cameras may be in use or unavailable\n";
        } else {
            std::cout << "Captured files:\n";
            for (const auto& file : captured_files) {
                std::cout << "  " << file << "\n";
            }
        }
    }

    bool CameraTestReporter::LaunchImageViewer(const std::string& image_path) {
        if (image_path.empty()) {
            return false;
        }

        std::string windows_path = ConvertPathForWindows(image_path);
        std::string command = "start \"Image Viewer\" \"" + windows_path + "\"";
        
        int result = std::system(command.c_str());
        return result == 0;
    }

    std::string CameraTestReporter::ConvertPathForWindows(const std::string& path) {
        std::string result = path;
        std::replace(result.begin(), result.end(), '/', '\\');
        return result;
    }

} // namespace golf_sim::camera::application
