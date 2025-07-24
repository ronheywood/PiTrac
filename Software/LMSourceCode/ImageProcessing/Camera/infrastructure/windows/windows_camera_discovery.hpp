/*
 * Windows Camera Discovery Service Implementation
 * 
 * Implements camera discovery using Windows Media Foundation APIs.
 * Provides comprehensive camera enumeration and capability detection.
 */

#pragma once

#include "../../domain/camera_discovery.hpp"
#include <Windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <wrl/client.h>
#include <memory>

namespace golf_sim::camera::infrastructure::windows {

    /**
     * Windows Media Foundation implementation of camera discovery
     */
    class WindowsCameraDiscoveryService : public domain::ICameraDiscoveryService {
    public:
        WindowsCameraDiscoveryService();
        virtual ~WindowsCameraDiscoveryService();

        // ICameraDiscoveryService implementation
        std::vector<domain::CameraDeviceInfo> DiscoverCameras() override;
        std::optional<domain::CameraDeviceInfo> GetCameraInfo(const std::string& device_id) override;
        bool IsCameraAccessible(const std::string& device_id) override;
        void RefreshDevices() override;

    private:
        bool is_mf_initialized_;
        std::vector<domain::CameraDeviceInfo> cached_devices_;
        
        // Helper methods
        bool InitializeMediaFoundation();
        void ShutdownMediaFoundation();
        std::vector<Microsoft::WRL::ComPtr<IMFActivate>> EnumerateDeviceActivators();
        domain::CameraDeviceInfo CreateDeviceInfo(IMFActivate* device_activator, int device_index);
        std::string WideStringToString(const std::wstring& wide_str);
        std::vector<domain::Size> GetSupportedResolutions(IMFActivate* device_activator);
        std::vector<int> GetSupportedFrameRates(IMFActivate* device_activator);
        std::vector<std::string> GetSupportedFormats(IMFActivate* device_activator);
        bool TestDeviceAccessibility(IMFActivate* device_activator);
    };

} // namespace golf_sim::camera::infrastructure::windows
