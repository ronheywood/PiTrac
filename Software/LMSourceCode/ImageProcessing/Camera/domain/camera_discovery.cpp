/*
 * Camera Discovery Service Factory Implementation
 */

#include "camera_discovery.hpp"

#ifdef _WIN32
#include "../infrastructure/windows/windows_camera_discovery.hpp"
#elif defined(__linux__) || defined(__APPLE__)
// Future: Add Unix implementation
#include "../infrastructure/unix/unix_camera_discovery.hpp"
#endif

namespace golf_sim::camera::domain {

    std::unique_ptr<ICameraDiscoveryService> CameraDiscoveryServiceFactory::CreateDiscoveryService() {
#ifdef _WIN32
        return std::make_unique<infrastructure::windows::WindowsCameraDiscoveryService>();
#elif defined(__linux__) || defined(__APPLE__)
        // Future: Return Unix implementation
        static_assert(false, "Unix camera discovery not yet implemented");
        return nullptr;
#else
        static_assert(false, "Unsupported platform for camera discovery");
        return nullptr;
#endif
    }

} // namespace golf_sim::camera::domain
