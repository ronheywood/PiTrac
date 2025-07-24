/*
 * Camera Discovery Service Domain Tests
 * 
 * Tests the camera discovery service domain interface and validates
 * the platform-specific implementations.
 */

#include <boost/test/unit_test.hpp>
#include "../../domain/camera_discovery.hpp"

BOOST_AUTO_TEST_SUITE(CameraDiscoveryTests)

BOOST_AUTO_TEST_CASE(camera_discovery_service_validation) {
    // Test the new camera discovery service domain interface
    BOOST_TEST_MESSAGE("=== Camera Discovery Service Validation ===");
    
    // Create discovery service using factory
    auto discovery_service = golf_sim::camera::domain::CameraDiscoveryServiceFactory::CreateDiscoveryService();
    
    BOOST_CHECK(discovery_service != nullptr);
    BOOST_TEST_MESSAGE("✓ Discovery service factory works");
    
    if (!discovery_service) {
        BOOST_TEST_MESSAGE("Cannot continue without discovery service");
        return;
    }
    
    // Test basic discovery functionality
    auto cameras = discovery_service->DiscoverCameras();
    BOOST_TEST_MESSAGE("Discovered " << cameras.size() << " cameras");
    
    // Validate that each discovered camera has required fields
    for (const auto& camera : cameras) {
        BOOST_CHECK(!camera.id.empty());
        BOOST_CHECK(!camera.name.empty());
        BOOST_CHECK(!camera.device_path.empty());
        
        BOOST_TEST_MESSAGE("Camera: " << camera.name << " (ID: " << camera.id << ")");
        BOOST_TEST_MESSAGE("  Accessible: " << (camera.is_accessible ? "Yes" : "No"));
        BOOST_TEST_MESSAGE("  Resolutions: " << camera.supported_resolutions.size());
        BOOST_TEST_MESSAGE("  Frame rates: " << camera.supported_fps.size());
        BOOST_TEST_MESSAGE("  Formats: " << camera.supported_formats.size());
        
        // Test individual camera lookup
        if (camera.is_accessible) {
            auto found_camera = discovery_service->GetCameraInfo(camera.id);
            BOOST_CHECK(found_camera.has_value());
            
            if (found_camera.has_value()) {
                BOOST_CHECK_EQUAL(found_camera->id, camera.id);
                BOOST_CHECK_EQUAL(found_camera->name, camera.name);
                BOOST_TEST_MESSAGE("  ✓ Individual lookup works");
            }
            
            // Test accessibility check
            bool accessible = discovery_service->IsCameraAccessible(camera.id);
            BOOST_CHECK_EQUAL(accessible, camera.is_accessible);
            BOOST_TEST_MESSAGE("  ✓ Accessibility check works");
        }
    }
    
    // Test lookup of non-existent camera
    auto non_existent = discovery_service->GetCameraInfo("does_not_exist");
    BOOST_CHECK(!non_existent.has_value());
    BOOST_TEST_MESSAGE("✓ Non-existent camera lookup returns nullopt");
    
    // Test accessibility of non-existent camera
    bool accessible = discovery_service->IsCameraAccessible("does_not_exist");
    BOOST_CHECK_EQUAL(accessible, false);
    BOOST_TEST_MESSAGE("✓ Non-existent camera accessibility returns false");
    
    BOOST_TEST_MESSAGE("=== Discovery Service Validation Complete ===");
}

BOOST_AUTO_TEST_SUITE_END()
