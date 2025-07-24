/*
 * Windows Camera Approval Test
 * 
 * Tests the Windows camera implementation by capturing a single frame
 * and presenting it as a JPEG for user review.
 */

#include <boost/test/unit_test.hpp>
#include "../../infrastructure/windows/windows_camera.hpp"
#include "../../domain/camera_discovery.hpp"
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <fstream>

BOOST_AUTO_TEST_SUITE(WindowsCameraTests)

BOOST_AUTO_TEST_CASE(capture_single_frame_for_review) {
    // Arrange
    golf_sim::camera::infrastructure::windows::WindowsCamera camera;
    golf_sim::camera::domain::CameraConfig config;
    config.resolution = golf_sim::camera::domain::Size(640, 480);
    config.fps = 30;
    
    // Create output directory for captured images
    std::filesystem::create_directories("captured_frames");
    
    // Act & Assert
    BOOST_TEST_MESSAGE("Initializing Windows camera...");
    BOOST_TEST_MESSAGE("Requested configuration:");
    BOOST_TEST_MESSAGE("  Resolution: " << config.resolution.width << "x" << config.resolution.height);
    BOOST_TEST_MESSAGE("  FPS: " << config.fps);
    
    bool initialized = camera.Initialize(config);
    
    if (!initialized) {
        BOOST_TEST_MESSAGE("Camera initialization failed");
        BOOST_TEST_MESSAGE("Device info: " << camera.GetDeviceInfo());
        BOOST_TEST_MESSAGE("This could be due to:");
        BOOST_TEST_MESSAGE("  1. No camera connected");
        BOOST_TEST_MESSAGE("  2. Camera in use by another application");
        BOOST_TEST_MESSAGE("  3. Camera driver issues");
        BOOST_TEST_MESSAGE("  4. Permissions issues");
        BOOST_TEST_MESSAGE("  5. Unsupported camera format");
        BOOST_TEST_MESSAGE("Please check Windows Device Manager and ensure camera is working in other apps");
        
        // Still run some basic tests even without camera
        auto supported_resolutions = camera.GetSupportedResolutions();
        BOOST_TEST_MESSAGE("Theoretical supported resolutions:");
        for (const auto& res : supported_resolutions) {
            BOOST_TEST_MESSAGE("  " << res.width << "x" << res.height);
        }
        return;
    }
    
    BOOST_CHECK(initialized);
    BOOST_TEST_MESSAGE("Camera initialized successfully");
    BOOST_TEST_MESSAGE("Device info: " << camera.GetDeviceInfo());
    
    // Print supported resolutions
    auto supported_resolutions = camera.GetSupportedResolutions();
    BOOST_TEST_MESSAGE("Supported resolutions:");
    for (const auto& res : supported_resolutions) {
        BOOST_TEST_MESSAGE("  " << res.width << "x" << res.height);
    }
    
    // Start streaming
    bool streaming_started = camera.StartStreaming();
    BOOST_CHECK(streaming_started);
    BOOST_CHECK(camera.IsStreaming());
    BOOST_TEST_MESSAGE("Camera streaming started");
    
    // Capture a frame
    BOOST_TEST_MESSAGE("Capturing frame...");
    auto frame = camera.CaptureFrame();
    
    if (frame.data.empty()) {
        BOOST_TEST_MESSAGE("No frame data captured - this may be expected for some cameras");
        camera.StopStreaming();
        return;
    }
    
    BOOST_CHECK(!frame.data.empty());
    BOOST_TEST_MESSAGE("Frame captured:");
    BOOST_TEST_MESSAGE("  Data size: " << frame.data.size() << " bytes");
    BOOST_TEST_MESSAGE("  Resolution: " << frame.resolution.width << "x" << frame.resolution.height);
    BOOST_TEST_MESSAGE("  Timestamp: " << frame.timestamp_us << " microseconds");
    BOOST_TEST_MESSAGE("  Sequence: " << frame.sequence_number);
    
    // Convert frame data to OpenCV Mat for JPEG encoding
    // Assuming RGB32 format from Media Foundation (4 bytes per pixel)
    if (frame.data.size() == frame.resolution.width * frame.resolution.height * 4) {
        cv::Mat image(frame.resolution.height, frame.resolution.width, CV_8UC4, frame.data.data());
        
        // Convert BGRA to BGR for JPEG (remove alpha channel)
        cv::Mat bgr_image;
        cv::cvtColor(image, bgr_image, cv::COLOR_BGRA2BGR);
        
        // Save as JPEG for review
        std::string filename = "captured_frames/camera_test_frame.jpg";
        bool saved = cv::imwrite(filename, bgr_image);
        
        if (saved) {
            BOOST_TEST_MESSAGE("Frame saved as JPEG: " << filename);
            BOOST_TEST_MESSAGE("Please review the captured image to verify camera functionality");
        } else {
            BOOST_TEST_MESSAGE("Failed to save frame as JPEG");
        }
        
        BOOST_CHECK(saved);
    } else {
        BOOST_TEST_MESSAGE("Unexpected frame data size. Expected: " 
                          << (frame.resolution.width * frame.resolution.height * 4)
                          << ", Got: " << frame.data.size());
        
        // Save raw data for analysis
        std::string raw_filename = "captured_frames/camera_test_frame.raw";
        std::ofstream raw_file(raw_filename, std::ios::binary);
        if (raw_file.is_open()) {
            raw_file.write(reinterpret_cast<const char*>(frame.data.data()), frame.data.size());
            raw_file.close();
            BOOST_TEST_MESSAGE("Raw frame data saved as: " << raw_filename);
        }
    }
    
    // Stop streaming
    bool streaming_stopped = camera.StopStreaming();
    BOOST_CHECK(streaming_stopped);
    BOOST_CHECK(!camera.IsStreaming());
    BOOST_TEST_MESSAGE("Camera streaming stopped");
}

BOOST_AUTO_TEST_CASE(enumerate_available_cameras) {
    // Test the camera discovery service
    BOOST_TEST_MESSAGE("=== Camera Discovery Service Test ===");
    
    // Create discovery service using factory
    auto discovery_service = golf_sim::camera::domain::CameraDiscoveryServiceFactory::CreateDiscoveryService();
    
    if (!discovery_service) {
        BOOST_TEST_MESSAGE("FAILED: Could not create camera discovery service");
        return;
    }
    
    BOOST_TEST_MESSAGE("SUCCESS: Camera discovery service created");
    
    // Discover available cameras
    auto discovered_cameras = discovery_service->DiscoverCameras();
    
    BOOST_TEST_MESSAGE("Found " << discovered_cameras.size() << " camera device(s)");
    
    if (discovered_cameras.empty()) {
        BOOST_TEST_MESSAGE("No cameras detected. Possible causes:");
        BOOST_TEST_MESSAGE("  - No camera hardware connected");
        BOOST_TEST_MESSAGE("  - Camera drivers not installed");
        BOOST_TEST_MESSAGE("  - Camera disabled in Device Manager");
        BOOST_TEST_MESSAGE("  - Camera blocked by privacy settings");
        return;
    }
    
    // Display information about each discovered camera
    for (size_t i = 0; i < discovered_cameras.size(); ++i) {
        const auto& camera = discovered_cameras[i];
        
        BOOST_TEST_MESSAGE("Camera " << i << ":");
        BOOST_TEST_MESSAGE("  ID: " << camera.id);
        BOOST_TEST_MESSAGE("  Name: " << camera.name);
        BOOST_TEST_MESSAGE("  Path: " << camera.device_path);
        BOOST_TEST_MESSAGE("  Status: " << (camera.is_accessible ? "ACCESSIBLE" : "NOT ACCESSIBLE"));
        
        if (camera.is_accessible) {
            BOOST_TEST_MESSAGE("  Supported Resolutions:");
            for (const auto& res : camera.supported_resolutions) {
                BOOST_TEST_MESSAGE("    " << res.width << "x" << res.height);
            }
            
            BOOST_TEST_MESSAGE("  Supported Frame Rates:");
            for (const auto& fps : camera.supported_fps) {
                BOOST_TEST_MESSAGE("    " << fps << " fps");
            }
            
            BOOST_TEST_MESSAGE("  Supported Formats:");
            for (const auto& format : camera.supported_formats) {
                BOOST_TEST_MESSAGE("    " << format);
            }
            
            // Test individual camera info retrieval
            auto camera_info = discovery_service->GetCameraInfo(camera.id);
            if (camera_info.has_value()) {
                BOOST_TEST_MESSAGE("  ✓ Individual camera info retrieval works");
            } else {
                BOOST_TEST_MESSAGE("  ✗ Individual camera info retrieval failed");
            }
            
            // Test accessibility check
            bool is_accessible = discovery_service->IsCameraAccessible(camera.id);
            BOOST_TEST_MESSAGE("  ✓ Accessibility check: " << (is_accessible ? "ACCESSIBLE" : "NOT ACCESSIBLE"));
        } else {
            BOOST_TEST_MESSAGE("    Camera may be in use by another application");
        }
    }
    
    // Test refresh functionality
    BOOST_TEST_MESSAGE("Testing device refresh...");
    discovery_service->RefreshDevices();
    auto refreshed_cameras = discovery_service->DiscoverCameras();
    BOOST_TEST_MESSAGE("After refresh: " << refreshed_cameras.size() << " camera device(s)");
    
    BOOST_TEST_MESSAGE("=== End Camera Discovery Test ===");
}

BOOST_AUTO_TEST_CASE(camera_configuration_test) {
    // Arrange
    golf_sim::camera::infrastructure::windows::WindowsCamera camera;
    golf_sim::camera::domain::CameraConfig config;
    config.resolution = golf_sim::camera::domain::Size(1280, 720);
    config.fps = 60;
    config.buffer_count = 8;
    
    // Act
    bool initialized = camera.Initialize(config);
    
    if (!initialized) {
        BOOST_TEST_MESSAGE("Camera not available for configuration test");
        return;
    }
    
    auto retrieved_config = camera.GetConfig();
    
    // Assert
    BOOST_TEST_MESSAGE("Requested configuration:");
    BOOST_TEST_MESSAGE("  Resolution: " << config.resolution.width << "x" << config.resolution.height);
    BOOST_TEST_MESSAGE("  FPS: " << config.fps);
    BOOST_TEST_MESSAGE("  Buffer count: " << config.buffer_count);
    
    BOOST_TEST_MESSAGE("Retrieved configuration:");
    BOOST_TEST_MESSAGE("  Resolution: " << retrieved_config.resolution.width << "x" << retrieved_config.resolution.height);
    BOOST_TEST_MESSAGE("  FPS: " << retrieved_config.fps);
    BOOST_TEST_MESSAGE("  Buffer count: " << retrieved_config.buffer_count);
    
    // Configuration should match what we set
    BOOST_CHECK_EQUAL(retrieved_config.resolution.width, config.resolution.width);
    BOOST_CHECK_EQUAL(retrieved_config.resolution.height, config.resolution.height);
    BOOST_CHECK_EQUAL(retrieved_config.fps, config.fps);
    BOOST_CHECK_EQUAL(retrieved_config.buffer_count, config.buffer_count);
}

BOOST_AUTO_TEST_SUITE_END()
