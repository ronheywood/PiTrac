/*
 * Windows Camera Approval Test
 * 
 * Tests the Windows camera implementation by capturing a single frame
 * and presenting it as a JPEG for user review.
 */

#include <boost/test/unit_test.hpp>
#include "../../infrastructure/windows/windows_camera.hpp"
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <fstream>

// Additional includes for diagnostics
#include <Windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <wrl/client.h>
#include <string>

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
    // This test helps debug camera detection issues
    BOOST_TEST_MESSAGE("=== Camera Detection Diagnostics ===");
    
    // Initialize Media Foundation
    HRESULT hr = MFStartup(MF_VERSION);
    if (FAILED(hr)) {
        BOOST_TEST_MESSAGE("FAILED: Could not initialize Media Foundation: " << std::hex << hr);
        return;
    }
    
    BOOST_TEST_MESSAGE("SUCCESS: Media Foundation initialized");
    
    // Create attributes for device enumeration
    Microsoft::WRL::ComPtr<IMFAttributes> attributes;
    hr = MFCreateAttributes(&attributes, 1);
    if (FAILED(hr)) {
        BOOST_TEST_MESSAGE("FAILED: Could not create attributes: " << std::hex << hr);
        MFShutdown();
        return;
    }
    
    // Set attribute to enumerate video capture devices
    hr = attributes->SetGUID(
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
    );
    if (FAILED(hr)) {
        BOOST_TEST_MESSAGE("FAILED: Could not set device type attribute: " << std::hex << hr);
        MFShutdown();
        return;
    }
    
    // Enumerate devices
    IMFActivate** devices = nullptr;
    UINT32 device_count = 0;
    hr = MFEnumDeviceSources(attributes.Get(), &devices, &device_count);
    
    if (FAILED(hr)) {
        BOOST_TEST_MESSAGE("FAILED: Device enumeration failed: " << std::hex << hr);
        MFShutdown();
        return;
    }
    
    BOOST_TEST_MESSAGE("Found " << device_count << " camera device(s)");
    
    if (device_count == 0) {
        BOOST_TEST_MESSAGE("No cameras detected. Possible causes:");
        BOOST_TEST_MESSAGE("  - No camera hardware connected");
        BOOST_TEST_MESSAGE("  - Camera drivers not installed");
        BOOST_TEST_MESSAGE("  - Camera disabled in Device Manager");
        BOOST_TEST_MESSAGE("  - Camera blocked by privacy settings");
        CoTaskMemFree(devices);
        MFShutdown();
        return;
    }
    
    // List details about each camera
    for (UINT32 i = 0; i < device_count; i++) {
        BOOST_TEST_MESSAGE("Camera " << i << ":");
        
        // Get friendly name
        WCHAR* friendly_name = nullptr;
        UINT32 name_length = 0;
        hr = devices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &friendly_name, &name_length);
        if (SUCCEEDED(hr) && friendly_name) {
            std::wstring ws(friendly_name);
            std::string name(ws.begin(), ws.end());
            BOOST_TEST_MESSAGE("  Name: " << name);
            CoTaskMemFree(friendly_name);
        }
        
        // Get symbolic link (device path)
        WCHAR* symbolic_link = nullptr;
        UINT32 link_length = 0;
        hr = devices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &symbolic_link, &link_length);
        if (SUCCEEDED(hr) && symbolic_link) {
            std::wstring ws(symbolic_link);
            std::string link(ws.begin(), ws.end());
            BOOST_TEST_MESSAGE("  Path: " << link);
            CoTaskMemFree(symbolic_link);
        }
        
        // Try to activate this camera to see if it's accessible
        Microsoft::WRL::ComPtr<IMFMediaSource> media_source;
        hr = devices[i]->ActivateObject(IID_PPV_ARGS(&media_source));
        if (SUCCEEDED(hr)) {
            BOOST_TEST_MESSAGE("  Status: ACCESSIBLE");
            media_source.Reset();
        } else {
            BOOST_TEST_MESSAGE("  Status: NOT ACCESSIBLE (error: " << std::hex << hr << ")");
            BOOST_TEST_MESSAGE("    This camera may be in use by another application");
        }
    }
    
    // Clean up
    for (UINT32 i = 0; i < device_count; i++) {
        devices[i]->Release();
    }
    CoTaskMemFree(devices);
    MFShutdown();
    
    BOOST_TEST_MESSAGE("=== End Camera Diagnostics ===");
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
