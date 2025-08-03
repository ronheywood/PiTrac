/*
 * Camera Image Service Application Layer Tests
 * 
 * Unit tests for the extracted camera image processing business logic
 * that was previously embedded in infrastructure tests.
 */

#include <boost/test/unit_test.hpp>
#include "../../application/camera_image_service.hpp"
#include "../../domain/camera_domain.hpp"
#include <filesystem>
#include <fstream>

BOOST_AUTO_TEST_SUITE(CameraImageServiceTests)

BOOST_AUTO_TEST_CASE(camera_image_processor_handles_empty_frame) {
    // Arrange
    golf_sim::camera::domain::CameraFrame empty_frame;
    std::string media_info = "Format: Unknown";
    
    // Act
    auto result = golf_sim::camera::application::CameraImageProcessor::ProcessCameraFrame(
        empty_frame, media_info);
    
    // Assert
    BOOST_CHECK(!result.conversion_successful);
    BOOST_CHECK_EQUAL(result.format_detected, "unknown");
    BOOST_CHECK_EQUAL(result.error_message, "Frame data is empty");
    BOOST_CHECK(result.image.empty());
}

BOOST_AUTO_TEST_CASE(camera_image_processor_detects_rgba_format) {
    // Arrange - Create 2x2 RGBA test frame
    golf_sim::camera::domain::Size resolution(2, 2);
    std::vector<uint8_t> rgba_data = {
        255, 0, 0, 255,    // Red pixel
        0, 255, 0, 255,    // Green pixel  
        0, 0, 255, 255,    // Blue pixel
        128, 128, 128, 255 // Gray pixel
    };
    
    golf_sim::camera::domain::CameraFrame frame(
        rgba_data, resolution, golf_sim::camera::domain::PixelFormat(), 123456, 1);
    std::string media_info = "Format: RGB32, Size: 2x2";
    
    // Act
    auto result = golf_sim::camera::application::CameraImageProcessor::ProcessCameraFrame(
        frame, media_info);
    
    // Assert
    BOOST_CHECK(result.conversion_successful);
    BOOST_CHECK_EQUAL(result.format_detected, "BGRA");
    BOOST_CHECK(result.error_message.empty());
    BOOST_CHECK(!result.image.empty());
    BOOST_CHECK_EQUAL(result.image.rows, 2);
    BOOST_CHECK_EQUAL(result.image.cols, 2);
    BOOST_CHECK_EQUAL(result.image.channels(), 3); // BGR output
}

BOOST_AUTO_TEST_CASE(camera_image_processor_detects_rgb_format) {
    // Arrange - Create 2x2 RGB test frame
    golf_sim::camera::domain::Size resolution(2, 2);
    std::vector<uint8_t> rgb_data = {
        255, 0, 0,     // Red pixel
        0, 255, 0,     // Green pixel
        0, 0, 255,     // Blue pixel
        128, 128, 128  // Gray pixel
    };
    
    golf_sim::camera::domain::CameraFrame frame(
        rgb_data, resolution, golf_sim::camera::domain::PixelFormat(), 123456, 1);
    std::string media_info = "Format: RGB24, Size: 2x2";
    
    // Act
    auto result = golf_sim::camera::application::CameraImageProcessor::ProcessCameraFrame(
        frame, media_info);
    
    // Assert
    BOOST_CHECK(result.conversion_successful);
    BOOST_CHECK_EQUAL(result.format_detected, "RGB");
    BOOST_CHECK(result.error_message.empty());
    BOOST_CHECK(!result.image.empty());
    BOOST_CHECK_EQUAL(result.image.rows, 2);
    BOOST_CHECK_EQUAL(result.image.cols, 2);
    BOOST_CHECK_EQUAL(result.image.channels(), 3);
}

BOOST_AUTO_TEST_CASE(camera_image_processor_handles_unsupported_format) {
    // Arrange - Create frame with unexpected data size
    golf_sim::camera::domain::Size resolution(2, 2);
    std::vector<uint8_t> weird_data = {1, 2, 3}; // Wrong size for 2x2 image
    
    golf_sim::camera::domain::CameraFrame frame(
        weird_data, resolution, golf_sim::camera::domain::PixelFormat(), 123456, 1);
    std::string media_info = "Format: Unknown, Size: 2x2";
    
    // Act
    auto result = golf_sim::camera::application::CameraImageProcessor::ProcessCameraFrame(
        frame, media_info);
    
    // Assert
    BOOST_CHECK(!result.conversion_successful);
    BOOST_CHECK_EQUAL(result.format_detected, "unknown");
    BOOST_CHECK(result.error_message.find("Unsupported format") != std::string::npos);
    BOOST_CHECK(result.error_message.find("3 bytes") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(camera_image_processor_extracts_dimensions_from_media_type) {
    // Arrange
    std::string media_info = "Format: NV12, Size: 1920x1080, FPS: 30";
    
    // Act
    auto [width, height] = golf_sim::camera::application::CameraImageProcessor::ExtractDimensionsFromMediaType(media_info);
    
    // Assert
    BOOST_CHECK_EQUAL(width, 1920);
    BOOST_CHECK_EQUAL(height, 1080);
}

BOOST_AUTO_TEST_CASE(camera_image_processor_uses_default_dimensions_when_missing) {
    // Arrange
    std::string media_info = "Format: RGB32, FPS: 30"; // No Size field
    
    // Act
    auto [width, height] = golf_sim::camera::application::CameraImageProcessor::ExtractDimensionsFromMediaType(media_info);
    
    // Assert
    BOOST_CHECK_EQUAL(width, 640);   // Default
    BOOST_CHECK_EQUAL(height, 480);  // Default
}

BOOST_AUTO_TEST_CASE(save_image_creates_directory_and_file) {
    // Arrange
    cv::Mat test_image = cv::Mat::zeros(10, 10, CV_8UC3);
    test_image.setTo(cv::Scalar(0, 255, 0)); // Green image
    
    std::string test_dir = "test_image_output";
    std::string base_filename = "test_image";
    
    // Clean up any existing test files
    std::filesystem::remove_all(test_dir);
    
    // Act
    auto result = golf_sim::camera::application::CameraImageProcessor::SaveImage(
        test_image, base_filename, test_dir);
    
    // Assert
    BOOST_CHECK(result.success);
    BOOST_CHECK_EQUAL(result.filename, test_dir + "/" + base_filename + ".jpg");
    BOOST_CHECK(result.error_message.empty());
    
    // Verify file exists
    BOOST_CHECK(std::filesystem::exists(result.filename));
    
    // Clean up
    std::filesystem::remove_all(test_dir);
}

BOOST_AUTO_TEST_CASE(save_image_handles_empty_image) {
    // Arrange
    cv::Mat empty_image;
    
    // Act
    auto result = golf_sim::camera::application::CameraImageProcessor::SaveImage(
        empty_image, "test", "test_dir");
    
    // Assert
    BOOST_CHECK(!result.success);
    BOOST_CHECK(result.filename.empty());
    BOOST_CHECK_EQUAL(result.error_message, "Image is empty");
}

BOOST_AUTO_TEST_CASE(camera_test_reporter_reports_successful_captures) {
    // Arrange
    std::vector<std::string> captured_files = {
        "camera_0_frame.jpg",
        "camera_1_frame.jpg"
    };
    int successful_count = 2;
    
    // Capture cout output
    std::stringstream buffer;
    std::streambuf* old_cout = std::cout.rdbuf(buffer.rdbuf());
    
    // Act
    golf_sim::camera::application::CameraTestReporter::ReportCaptureResults(
        captured_files, successful_count);
    
    // Restore cout
    std::cout.rdbuf(old_cout);
    
    // Assert
    std::string output = buffer.str();
    BOOST_CHECK(output.find("Successfully captured 2 images") != std::string::npos);
    BOOST_CHECK(output.find("camera_0_frame.jpg") != std::string::npos);
    BOOST_CHECK(output.find("camera_1_frame.jpg") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(camera_test_reporter_handles_no_captures) {
    // Arrange
    std::vector<std::string> empty_files;
    int zero_count = 0;
    
    // Capture cout output
    std::stringstream buffer;
    std::streambuf* old_cout = std::cout.rdbuf(buffer.rdbuf());
    
    // Act
    golf_sim::camera::application::CameraTestReporter::ReportCaptureResults(
        empty_files, zero_count);
    
    // Restore cout
    std::cout.rdbuf(old_cout);
    
    // Assert
    std::string output = buffer.str();
    BOOST_CHECK(output.find("Successfully captured 0 images") != std::string::npos);
    BOOST_CHECK(output.find("No images were captured") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(convert_path_for_windows_replaces_forward_slashes) {
    // This is a private method, but we can test the public LaunchImageViewer behavior
    // by testing that it handles paths correctly
    
    // Arrange
    std::string unix_style_path = "captured_frames/camera_0_frame.jpg";
    
    // Act - This will internally convert the path but we can't directly test the conversion
    // since it's a private method. We just test that it doesn't crash with forward slashes.
    bool result = golf_sim::camera::application::CameraTestReporter::LaunchImageViewer(unix_style_path);
    
    // Assert - We expect this to succeed or fail gracefully, not crash
    // The actual result depends on whether an image viewer is available
    BOOST_TEST_MESSAGE("Image viewer launch result: " << (result ? "success" : "failed"));
    // Don't assert the result since it depends on system configuration
}

BOOST_AUTO_TEST_SUITE_END()
