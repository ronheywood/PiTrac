/*
 * Independent Camera Domain Tests
 * 
 * Tests the camera domain with NO dependencies on other bounded contexts.
 * Uses adapter pattern for integration with ImageAnalysis domain.
 */

#include <boost/test/unit_test.hpp>
#include "../../domain/independent_camera_interfaces.hpp"
#include <opencv2/opencv.hpp>
#include <filesystem>

// Forward declaration for adapter testing (no dependency on ImageAnalysis BC)
namespace ImageAnalysis {
    struct MockImageBuffer {
        cv::Mat data;
        std::chrono::microseconds timestamp;
        std::string camera_id;
        std::string metadata;
        
        bool IsValid() const { return !data.empty(); }
    };
}

BOOST_AUTO_TEST_SUITE(IndependentCameraTests)

BOOST_AUTO_TEST_CASE(camera_image_has_no_external_dependencies) {
    // Arrange & Act - Test Camera BC's own image type
    BOOST_TEST_MESSAGE("=== Testing Independent Camera Image Type ===");
    
    using namespace GolfLaunchMonitor::Camera;
    
    // Create test image data
    std::vector<uint8_t> testPixels(640 * 480 * 3);  // RGB data
    for (size_t i = 0; i < testPixels.size(); i += 3) {
        testPixels[i] = 255;     // R
        testPixels[i + 1] = 128; // G
        testPixels[i + 2] = 64;  // B
    }
    
    auto timestamp = std::chrono::microseconds(1234567890);
    
    CameraImage cameraImage(
        std::move(testPixels),
        640, 480, 3,
        "RGB",
        timestamp,
        "test_camera_1",
        42,
        "exposure=auto,format=RGB"
    );
    
    // Assert - Verify Camera BC's own image type works independently
    BOOST_CHECK(cameraImage.IsValid());
    BOOST_CHECK_EQUAL(cameraImage.width, 640);
    BOOST_CHECK_EQUAL(cameraImage.height, 480);
    BOOST_CHECK_EQUAL(cameraImage.channels, 3);
    BOOST_CHECK_EQUAL(cameraImage.format, "RGB");
    BOOST_CHECK_EQUAL(cameraImage.captureTime.count(), 1234567890);
    BOOST_CHECK_EQUAL(cameraImage.sourceCamera, "test_camera_1");
    BOOST_CHECK_EQUAL(cameraImage.sequenceNumber, 42);
    BOOST_CHECK(cameraImage.HasValidDataSize());
    
    BOOST_TEST_MESSAGE("✓ CameraImage works independently - no external dependencies");
    BOOST_TEST_MESSAGE("  Resolution: " << cameraImage.width << "x" << cameraImage.height);
    BOOST_TEST_MESSAGE("  Channels: " << cameraImage.channels);
    BOOST_TEST_MESSAGE("  Format: " << cameraImage.format);
    BOOST_TEST_MESSAGE("  Data size: " << cameraImage.pixelData.size() << " bytes");
}

BOOST_AUTO_TEST_CASE(adapter_converts_camera_image_to_analysis_format) {
    // Arrange - Test the adapter pattern (application layer responsibility)
    BOOST_TEST_MESSAGE("=== Testing Camera to ImageAnalysis Adapter ===");
    
    using namespace GolfLaunchMonitor::Camera;
    
    // Create Camera BC image
    std::vector<uint8_t> bgrPixels(320 * 240 * 3);
    for (int y = 0; y < 240; ++y) {
        for (int x = 0; x < 320; ++x) {
            size_t idx = (y * 320 + x) * 3;
            bgrPixels[idx] = static_cast<uint8_t>(x % 256);     // B
            bgrPixels[idx + 1] = static_cast<uint8_t>(y % 256); // G  
            bgrPixels[idx + 2] = static_cast<uint8_t>((x + y) % 256); // R
        }
    }
    
    CameraImage cameraImg(
        std::move(bgrPixels),
        320, 240, 3,
        "BGR",
        std::chrono::microseconds(9876543210),
        "tee_camera",
        100,
        "format=BGR,roi=tee_area"
    );
    
    // Act - Adapter function (this would be in Application layer)
    auto ConvertCameraToAnalysis = [](const CameraImage& src) -> ImageAnalysis::MockImageBuffer {
        // This adapter function has NO dependencies on Camera BC internals
        // and NO dependencies on ImageAnalysis BC internals
        
        cv::Mat image(src.height, src.width, CV_8UC3, 
                     const_cast<uint8_t*>(src.pixelData.data()));
        
        ImageAnalysis::MockImageBuffer result;
        result.data = image.clone();  // ImageAnalysis expects cv::Mat
        result.timestamp = src.captureTime;
        result.camera_id = src.sourceCamera;
        result.metadata = src.metadata + ",adapted_from=" + src.format;
        
        return result;
    };
    
    auto analysisImage = ConvertCameraToAnalysis(cameraImg);
    
    // Assert - Verify successful conversion
    BOOST_CHECK(analysisImage.IsValid());
    BOOST_CHECK_EQUAL(analysisImage.data.rows, 240);
    BOOST_CHECK_EQUAL(analysisImage.data.cols, 320);
    BOOST_CHECK_EQUAL(analysisImage.data.channels(), 3);
    BOOST_CHECK_EQUAL(analysisImage.timestamp.count(), 9876543210);
    BOOST_CHECK_EQUAL(analysisImage.camera_id, "tee_camera");
    BOOST_CHECK(analysisImage.metadata.find("adapted_from=BGR") != std::string::npos);
    
    BOOST_TEST_MESSAGE("✓ Adapter successfully converts between BC types");
    BOOST_TEST_MESSAGE("  Camera format: " << cameraImg.format);
    BOOST_TEST_MESSAGE("  Analysis format: cv::Mat " << analysisImage.data.type());
    BOOST_TEST_MESSAGE("  Metadata preserved: " << analysisImage.metadata);
    
    // Save converted image for verification
    std::filesystem::create_directories("adapter_test_output");
    cv::imwrite("adapter_test_output/adapted_image.jpg", analysisImage.data);
    BOOST_TEST_MESSAGE("✓ Converted image saved for verification");
}

BOOST_AUTO_TEST_CASE(independent_camera_interfaces_work_without_analysis_bc) {
    // Arrange - Test Camera BC interfaces in isolation
    BOOST_TEST_MESSAGE("=== Testing Camera BC Interface Independence ===");
    
    using namespace GolfLaunchMonitor::Camera;
    
    // Mock camera implementation (would normally be in infrastructure layer)
    class MockTeeCamera : public ITeeCamera {
    private:
        bool streaming = false;
        
    public:
        CameraImage CaptureFrame() override {
            if (!streaming) return CameraImage{}; // Empty = invalid
            
            // Simulate RGB image
            std::vector<uint8_t> pixels(160 * 120 * 3, 128); // Gray image
            return CameraImage(
                std::move(pixels), 160, 120, 3, "RGB",
                std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()),
                "mock_tee_camera", 1, "mock=true"
            );
        }
        
        void SetCaptureArea(const Rectangle& area) override {
            // Mock implementation
        }
        
        bool IsStreamingActive() const override { return streaming; }
        void StartStreaming() override { streaming = true; }
        void StopStreaming() override { streaming = false; }
        
        CameraId GetCameraId() const override { return "mock_tee_camera"; }
        std::string GetCameraName() const override { return "Mock Tee Camera"; }
    };
    
    // Act - Test camera operations
    MockTeeCamera teeCamera;
    
    // Assert - Camera works independently
    BOOST_CHECK(!teeCamera.IsStreamingActive());
    BOOST_CHECK_EQUAL(teeCamera.GetCameraId(), "mock_tee_camera");
    
    teeCamera.StartStreaming();
    BOOST_CHECK(teeCamera.IsStreamingActive());
    
    auto image = teeCamera.CaptureFrame();
    BOOST_CHECK(image.IsValid());
    BOOST_CHECK_EQUAL(image.width, 160);
    BOOST_CHECK_EQUAL(image.height, 120);
    BOOST_CHECK_EQUAL(image.format, "RGB");
    BOOST_CHECK_EQUAL(image.sourceCamera, "mock_tee_camera");
    
    teeCamera.StopStreaming();
    BOOST_CHECK(!teeCamera.IsStreamingActive());
    
    BOOST_TEST_MESSAGE("✓ Camera BC interfaces work completely independently");
    BOOST_TEST_MESSAGE("✓ No dependencies on ImageAnalysis BC");
    BOOST_TEST_MESSAGE("✓ Clean separation of concerns achieved");
}

BOOST_AUTO_TEST_CASE(camera_image_format_handling) {
    // Arrange - Test different image formats that cameras might produce
    BOOST_TEST_MESSAGE("=== Testing Camera Image Format Independence ===");
    
    using namespace GolfLaunchMonitor::Camera;
    
    // Test different formats cameras might produce
    struct FormatTest {
        std::string format;
        int channels;
        std::string description;
    };
    
    std::vector<FormatTest> formats = {
        {"RGB", 3, "Standard RGB format"},
        {"BGR", 3, "OpenCV default format"},
        {"RGBA", 4, "RGB with alpha channel"},
        {"BGRA", 4, "BGR with alpha channel"},
        {"GRAY", 1, "Grayscale format"},
        {"NV12", 1, "YUV 4:2:0 format (special case)"},
        {"YUY2", 2, "YUV 4:2:2 packed format"}
    };
    
    for (const auto& test : formats) {
        // Create test image for each format
        size_t pixelCount = 100 * 100 * test.channels;
        std::vector<uint8_t> testData(pixelCount, 0x80); // Mid-gray
        
        CameraImage image(
            std::move(testData), 100, 100, test.channels, test.format,
            std::chrono::microseconds(0), "format_test_camera"
        );
        
        // Assert
        BOOST_CHECK(image.IsValid());
        BOOST_CHECK_EQUAL(image.format, test.format);
        BOOST_CHECK_EQUAL(image.channels, test.channels);
        BOOST_CHECK(image.HasValidDataSize());
        
        BOOST_TEST_MESSAGE("✓ Format supported: " << test.format 
                          << " (" << test.channels << " channels) - " << test.description);
    }
    
    BOOST_TEST_MESSAGE("✓ Camera BC handles multiple formats independently");
}

BOOST_AUTO_TEST_SUITE_END()
