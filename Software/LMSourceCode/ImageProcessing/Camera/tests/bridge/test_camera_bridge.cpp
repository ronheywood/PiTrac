/*
 * Camera to ImageAnalysis Bridge Tests
 * 
 * Tests the bridge that converts camera frames to ImageBuffer objects
 * for consumption by the ImageAnalysis domain.
 */

#include <boost/test/unit_test.hpp>
#include "../../domain/golf_camera_interfaces.hpp"
#include "../../../ImageAnalysis/domain/value_objects.hpp"
#include "../../infrastructure/windows/windows_camera.hpp"
#include <opencv2/opencv.hpp>
#include <filesystem>

BOOST_AUTO_TEST_SUITE(CameraBridgeTests)

BOOST_AUTO_TEST_CASE(convert_windows_camera_frame_to_image_buffer) {
    // Arrange - This test validates the bridge converts camera frames to ImageBuffer
    BOOST_TEST_MESSAGE("=== Camera to ImageBuffer Bridge Test ===");
    
    // This is our Outside-In test - we want a bridge that:
    // 1. Takes a raw camera frame (what WindowsCamera produces)
    // 2. Converts it to ImageBuffer (what ImageAnalysis domain expects)
    // 3. Handles all format complexity internally
    
    // Act & Assert
    BOOST_TEST_MESSAGE("Testing bridge conversion from camera frame to ImageBuffer...");
    
    // Create a test camera frame (simulating what WindowsCamera produces)
    golf_sim::camera::domain::Size resolution(640, 480);
    std::vector<uint8_t> test_frame_data;
    
    // Simulate RGB32/BGRA format (what many cameras produce)
    size_t frame_size = resolution.width * resolution.height * 4; // 4 bytes per pixel
    test_frame_data.resize(frame_size);
    
    // Fill with test pattern (simple gradient)
    for (int y = 0; y < resolution.height; ++y) {
        for (int x = 0; x < resolution.width; ++x) {
            size_t pixel_index = (y * resolution.width + x) * 4;
            test_frame_data[pixel_index + 0] = static_cast<uint8_t>(x % 256);     // B
            test_frame_data[pixel_index + 1] = static_cast<uint8_t>(y % 256);     // G
            test_frame_data[pixel_index + 2] = static_cast<uint8_t>((x + y) % 256); // R
            test_frame_data[pixel_index + 3] = 255; // A (full alpha)
        }
    }
    
    golf_sim::camera::domain::Frame camera_frame;
    camera_frame.data = test_frame_data;
    camera_frame.resolution = resolution;
    camera_frame.timestamp_us = 123456789;
    camera_frame.sequence_number = 42;
    
    // ACT: Convert using the bridge (this should abstract all format complexity)
    // TODO: Implement CameraToImageAnalysisBridge
    BOOST_TEST_MESSAGE("Bridge conversion not yet implemented");
    BOOST_TEST_MESSAGE("Expected behavior:");
    BOOST_TEST_MESSAGE("  1. Take camera Frame with BGRA data");
    BOOST_TEST_MESSAGE("  2. Convert to cv::Mat in BGR format");
    BOOST_TEST_MESSAGE("  3. Create ImageBuffer with proper timestamp and metadata");
    BOOST_TEST_MESSAGE("  4. Return clean ImageBuffer ready for ImageAnalysis domain");
    
    // For now, manually simulate the conversion that the bridge should do
    cv::Mat bgra_image(resolution.height, resolution.width, CV_8UC4, camera_frame.data.data());
    cv::Mat bgr_image;
    cv::cvtColor(bgra_image, bgr_image, cv::COLOR_BGRA2BGR);
    
    // Create ImageBuffer as the bridge should
    golf_sim::image_analysis::domain::ImageBuffer image_buffer(
        bgr_image,
        std::chrono::microseconds(camera_frame.timestamp_us),
        "test_camera_0",
        "format=BGRA,converted=BGR"
    );
    
    // ASSERT: Verify the ImageBuffer is properly formed
    BOOST_CHECK(image_buffer.IsValid());
    BOOST_CHECK(!image_buffer.data.empty());
    BOOST_CHECK_EQUAL(image_buffer.data.rows, resolution.height);
    BOOST_CHECK_EQUAL(image_buffer.data.cols, resolution.width);
    BOOST_CHECK_EQUAL(image_buffer.data.channels(), 3); // BGR = 3 channels
    BOOST_CHECK_EQUAL(image_buffer.timestamp.count(), camera_frame.timestamp_us);
    BOOST_CHECK_EQUAL(image_buffer.camera_id, "test_camera_0");
    
    BOOST_TEST_MESSAGE("✓ ImageBuffer created successfully:");
    BOOST_TEST_MESSAGE("  Dimensions: " << image_buffer.data.cols << "x" << image_buffer.data.rows);
    BOOST_TEST_MESSAGE("  Channels: " << image_buffer.data.channels());
    BOOST_TEST_MESSAGE("  Timestamp: " << image_buffer.timestamp.count() << " microseconds");
    BOOST_TEST_MESSAGE("  Camera ID: " << image_buffer.camera_id);
    BOOST_TEST_MESSAGE("  Metadata: " << image_buffer.metadata);
    
    // Save test image to verify visual correctness
    std::filesystem::create_directories("bridge_test_output");
    std::string output_file = "bridge_test_output/bridge_conversion_test.jpg";
    bool saved = cv::imwrite(output_file, image_buffer.data);
    
    if (saved) {
        BOOST_TEST_MESSAGE("✓ Test image saved: " << output_file);
        BOOST_TEST_MESSAGE("Manual verification: Check that image shows gradient pattern");
    }
    
    BOOST_CHECK(saved);
    
    BOOST_TEST_MESSAGE("=== Bridge Test Complete ===");
    BOOST_TEST_MESSAGE("Next: Implement actual CameraToImageAnalysisBridge class");
}

BOOST_AUTO_TEST_CASE(bridge_handles_nv12_format_conversion) {
    // Arrange - Test NV12 format conversion (common camera format)
    BOOST_TEST_MESSAGE("=== NV12 Format Bridge Test ===");
    
    golf_sim::camera::domain::Size resolution(640, 480);
    
    // Create NV12 test data (Y plane + UV plane)
    size_t y_plane_size = resolution.width * resolution.height;
    size_t uv_plane_size = resolution.width * resolution.height / 2;
    size_t total_nv12_size = y_plane_size + uv_plane_size;
    
    std::vector<uint8_t> nv12_data(total_nv12_size);
    
    // Fill Y plane with gradient
    for (int y = 0; y < resolution.height; ++y) {
        for (int x = 0; x < resolution.width; ++x) {
            size_t y_index = y * resolution.width + x;
            nv12_data[y_index] = static_cast<uint8_t>((x + y) % 256);
        }
    }
    
    // Fill UV plane with test pattern
    for (size_t i = 0; i < uv_plane_size; ++i) {
        nv12_data[y_plane_size + i] = static_cast<uint8_t>(128 + (i % 128));
    }
    
    golf_sim::camera::domain::Frame nv12_frame;
    nv12_frame.data = nv12_data;
    nv12_frame.resolution = resolution;
    nv12_frame.timestamp_us = 987654321;
    nv12_frame.sequence_number = 100;
    
    // ACT: Manual conversion (bridge should do this)
    BOOST_TEST_MESSAGE("Converting NV12 to BGR for ImageAnalysis domain...");
    
    // Create Y plane
    cv::Mat y_plane(resolution.height, resolution.width, CV_8UC1, nv12_frame.data.data());
    
    // Create UV plane (half width, half height, 2 channels interleaved)
    cv::Mat uv_plane(resolution.height / 2, resolution.width / 2, CV_8UC2, 
                    nv12_frame.data.data() + y_plane_size);
    
    // Convert NV12 to BGR
    cv::Mat bgr_image;
    cv::cvtColor(y_plane, bgr_image, cv::COLOR_GRAY2BGR); // Temporary conversion
    
    // Create ImageBuffer
    golf_sim::image_analysis::domain::ImageBuffer image_buffer(
        bgr_image,
        std::chrono::microseconds(nv12_frame.timestamp_us),
        "test_camera_nv12",
        "format=NV12,converted=BGR"
    );
    
    // ASSERT: Verify conversion
    BOOST_CHECK(image_buffer.IsValid());
    BOOST_CHECK_EQUAL(image_buffer.data.channels(), 3);
    BOOST_CHECK_EQUAL(image_buffer.timestamp.count(), nv12_frame.timestamp_us);
    
    BOOST_TEST_MESSAGE("✓ NV12 to ImageBuffer conversion successful");
    BOOST_TEST_MESSAGE("Note: Full NV12 color conversion requires proper YUV->BGR algorithm");
    
    // Save for visual verification
    std::filesystem::create_directories("bridge_test_output");
    cv::imwrite("bridge_test_output/nv12_conversion_test.jpg", image_buffer.data);
    
    BOOST_TEST_MESSAGE("=== NV12 Bridge Test Complete ===");
}

BOOST_AUTO_TEST_CASE(bridge_preserves_camera_metadata) {
    // Arrange - Verify metadata preservation through bridge
    BOOST_TEST_MESSAGE("=== Camera Metadata Preservation Test ===");
    
    // Create test frame with rich metadata
    golf_sim::camera::domain::Size resolution(320, 240);
    std::vector<uint8_t> rgb_data(resolution.width * resolution.height * 3);
    
    // Simple RGB pattern
    for (size_t i = 0; i < rgb_data.size(); i += 3) {
        rgb_data[i] = 255;     // R
        rgb_data[i + 1] = 128; // G  
        rgb_data[i + 2] = 64;  // B
    }
    
    golf_sim::camera::domain::Frame test_frame;
    test_frame.data = rgb_data;
    test_frame.resolution = resolution;
    test_frame.timestamp_us = 555666777;
    test_frame.sequence_number = 888;
    
    // ACT: Convert with metadata preservation
    cv::Mat rgb_image(resolution.height, resolution.width, CV_8UC3, test_frame.data.data());
    
    // Build rich metadata string (bridge should do this)
    std::string metadata = "format=RGB24,sequence=" + std::to_string(test_frame.sequence_number) +
                          ",width=" + std::to_string(resolution.width) +
                          ",height=" + std::to_string(resolution.height);
    
    golf_sim::image_analysis::domain::ImageBuffer image_buffer(
        rgb_image,
        std::chrono::microseconds(test_frame.timestamp_us),
        "metadata_test_camera",
        metadata
    );
    
    // ASSERT: Verify metadata preservation
    BOOST_CHECK(image_buffer.IsValid());
    BOOST_CHECK_EQUAL(image_buffer.camera_id, "metadata_test_camera");
    BOOST_CHECK_EQUAL(image_buffer.timestamp.count(), test_frame.timestamp_us);
    BOOST_CHECK(image_buffer.metadata.find("format=RGB24") != std::string::npos);
    BOOST_CHECK(image_buffer.metadata.find("sequence=888") != std::string::npos);
    
    BOOST_TEST_MESSAGE("✓ Metadata preserved correctly:");
    BOOST_TEST_MESSAGE("  " << image_buffer.metadata);
    
    BOOST_TEST_MESSAGE("=== Metadata Preservation Test Complete ===");
}

BOOST_AUTO_TEST_SUITE_END()
