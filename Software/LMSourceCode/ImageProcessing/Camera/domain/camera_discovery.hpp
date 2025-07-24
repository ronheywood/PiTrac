/*
 * Camera Discovery Domain Interface
 * 
 * Defines the domain model for discovering and enumerating available cameras.
 * This provides a platform-agnostic interface for camera detection and capability discovery.
 */

#pragma once

#include "camera_domain.hpp"
#include <vector>
#include <string>
#include <optional>

namespace golf_sim::camera::domain {

    /**
     * Represents information about a discovered camera device
     */
    struct CameraDeviceInfo {
        std::string id;                    // Unique device identifier
        std::string name;                  // Human-readable device name
        std::string device_path;           // Platform-specific device path
        bool is_accessible;                // Whether the device can be opened
        std::vector<Size> supported_resolutions;  // Available resolutions
        std::vector<int> supported_fps;    // Available frame rates
        std::vector<std::string> supported_formats;  // Available pixel formats
        
        // Value type semantics
        CameraDeviceInfo() = default;
        CameraDeviceInfo(const std::string& device_id, 
                        const std::string& device_name,
                        const std::string& path,
                        bool accessible = true)
            : id(device_id)
            , name(device_name) 
            , device_path(path)
            , is_accessible(accessible) {}
    };

    /**
     * Domain service interface for camera discovery
     * 
     * Provides platform-agnostic camera enumeration and capability discovery.
     * Implementations handle platform-specific device detection.
     */
    class ICameraDiscoveryService {
    public:
        virtual ~ICameraDiscoveryService() = default;

        /**
         * Discover all available camera devices on the system
         * @return Vector of discovered camera devices, empty if none found
         */
        virtual std::vector<CameraDeviceInfo> DiscoverCameras() = 0;

        /**
         * Get detailed information about a specific camera device
         * @param device_id Unique identifier for the camera
         * @return Device information if found, nullopt otherwise
         */
        virtual std::optional<CameraDeviceInfo> GetCameraInfo(const std::string& device_id) = 0;

        /**
         * Check if a specific camera device is currently accessible
         * @param device_id Unique identifier for the camera
         * @return True if camera can be opened, false otherwise
         */
        virtual bool IsCameraAccessible(const std::string& device_id) = 0;

        /**
         * Refresh the internal camera device list
         * Call this if you suspect the hardware configuration has changed
         */
        virtual void RefreshDevices() = 0;
    };

    /**
     * Factory for creating platform-appropriate camera discovery services
     */
    class CameraDiscoveryServiceFactory {
    public:
        /**
         * Create a camera discovery service for the current platform
         * @return Unique pointer to platform-specific discovery service
         */
        static std::unique_ptr<ICameraDiscoveryService> CreateDiscoveryService();
    };

} // namespace golf_sim::camera::domain
