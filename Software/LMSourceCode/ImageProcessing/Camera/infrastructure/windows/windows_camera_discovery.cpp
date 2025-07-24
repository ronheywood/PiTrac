/*
 * Windows Camera Discovery Service Implementation
 */

#include "windows_camera_discovery.hpp"
#include <iostream>
#include <string>
#include <locale>
#include <codecvt>

namespace golf_sim::camera::infrastructure::windows {

    WindowsCameraDiscoveryService::WindowsCameraDiscoveryService() 
        : is_mf_initialized_(false) {
        InitializeMediaFoundation();
    }

    WindowsCameraDiscoveryService::~WindowsCameraDiscoveryService() {
        ShutdownMediaFoundation();
    }

    bool WindowsCameraDiscoveryService::InitializeMediaFoundation() {
        if (is_mf_initialized_) {
            return true;
        }

        HRESULT hr = MFStartup(MF_VERSION);
        if (SUCCEEDED(hr)) {
            is_mf_initialized_ = true;
            return true;
        }

        std::cerr << "Failed to initialize Media Foundation: " << std::hex << hr << std::endl;
        return false;
    }

    void WindowsCameraDiscoveryService::ShutdownMediaFoundation() {
        if (is_mf_initialized_) {
            MFShutdown();
            is_mf_initialized_ = false;
        }
    }

    std::vector<domain::CameraDeviceInfo> WindowsCameraDiscoveryService::DiscoverCameras() {
        if (!InitializeMediaFoundation()) {
            return {};
        }

        auto device_activators = EnumerateDeviceActivators();
        std::vector<domain::CameraDeviceInfo> discovered_cameras;

        for (size_t i = 0; i < device_activators.size(); ++i) {
            auto device_info = CreateDeviceInfo(device_activators[i].Get(), static_cast<int>(i));
            discovered_cameras.push_back(std::move(device_info));
        }

        // Clean up activators
        for (auto& activator : device_activators) {
            activator.Reset();
        }

        // Cache the results
        cached_devices_ = discovered_cameras;
        return discovered_cameras;
    }

    std::optional<domain::CameraDeviceInfo> WindowsCameraDiscoveryService::GetCameraInfo(const std::string& device_id) {
        // First check cache
        for (const auto& device : cached_devices_) {
            if (device.id == device_id) {
                return device;
            }
        }

        // If not in cache, refresh and try again
        RefreshDevices();
        for (const auto& device : cached_devices_) {
            if (device.id == device_id) {
                return device;
            }
        }

        return std::nullopt;
    }

    bool WindowsCameraDiscoveryService::IsCameraAccessible(const std::string& device_id) {
        auto camera_info = GetCameraInfo(device_id);
        return camera_info.has_value() && camera_info->is_accessible;
    }

    void WindowsCameraDiscoveryService::RefreshDevices() {
        cached_devices_.clear();
        DiscoverCameras();
    }

    std::vector<Microsoft::WRL::ComPtr<IMFActivate>> WindowsCameraDiscoveryService::EnumerateDeviceActivators() {
        std::vector<Microsoft::WRL::ComPtr<IMFActivate>> activators;

        // Create attributes for device enumeration
        Microsoft::WRL::ComPtr<IMFAttributes> attributes;
        HRESULT hr = MFCreateAttributes(&attributes, 1);
        if (FAILED(hr)) {
            std::cerr << "Failed to create attributes for device enumeration: " << std::hex << hr << std::endl;
            return activators;
        }

        // Set attribute to enumerate video capture devices
        hr = attributes->SetGUID(
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
        );
        if (FAILED(hr)) {
            std::cerr << "Failed to set device type attribute: " << std::hex << hr << std::endl;
            return activators;
        }

        // Enumerate devices
        IMFActivate** devices = nullptr;
        UINT32 device_count = 0;
        hr = MFEnumDeviceSources(attributes.Get(), &devices, &device_count);
        if (FAILED(hr)) {
            std::cerr << "Device enumeration failed: " << std::hex << hr << std::endl;
            // Clean up devices array even if enumeration failed
            if (devices) {
                CoTaskMemFree(devices);
            }
            return activators;
        }

        // Convert to vector of smart pointers
        for (UINT32 i = 0; i < device_count; ++i) {
            Microsoft::WRL::ComPtr<IMFActivate> activator;
            activator.Attach(devices[i]); // Transfer ownership
            activators.push_back(activator);
        }

        CoTaskMemFree(devices);
        return activators;
    }

    domain::CameraDeviceInfo WindowsCameraDiscoveryService::CreateDeviceInfo(IMFActivate* device_activator, int device_index) {
        domain::CameraDeviceInfo device_info;
        
        // Generate unique ID
        device_info.id = "windows_camera_" + std::to_string(device_index);

        // Get friendly name
        WCHAR* friendly_name = nullptr;
        UINT32 name_length = 0;
        HRESULT hr = device_activator->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &friendly_name, &name_length);
        if (SUCCEEDED(hr) && friendly_name) {
            device_info.name = WideStringToString(std::wstring(friendly_name));
            CoTaskMemFree(friendly_name);
        } else {
            device_info.name = "Unknown Camera " + std::to_string(device_index);
        }

        // Get symbolic link (device path)
        WCHAR* symbolic_link = nullptr;
        UINT32 link_length = 0;
        hr = device_activator->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &symbolic_link, &link_length);
        if (SUCCEEDED(hr) && symbolic_link) {
            device_info.device_path = WideStringToString(std::wstring(symbolic_link));
            CoTaskMemFree(symbolic_link);
        } else {
            device_info.device_path = "unknown_path_" + std::to_string(device_index);
        }

        // Test accessibility
        device_info.is_accessible = TestDeviceAccessibility(device_activator);

        // Get supported capabilities (only if accessible)
        if (device_info.is_accessible) {
            device_info.supported_resolutions = GetSupportedResolutions(device_activator);
            device_info.supported_fps = GetSupportedFrameRates(device_activator);
            device_info.supported_formats = GetSupportedFormats(device_activator);
        }

        return device_info;
    }

    std::string WindowsCameraDiscoveryService::WideStringToString(const std::wstring& wide_str) {
        if (wide_str.empty()) {
            return std::string();
        }

        int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wide_str[0], static_cast<int>(wide_str.size()), nullptr, 0, nullptr, nullptr);
        std::string str_to(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, &wide_str[0], static_cast<int>(wide_str.size()), &str_to[0], size_needed, nullptr, nullptr);
        return str_to;
    }

    std::vector<domain::Size> WindowsCameraDiscoveryService::GetSupportedResolutions(IMFActivate* device_activator) {
        // Return common resolutions that most cameras support
        // In a full implementation, we would query the device's actual capabilities
        return {
            domain::Size(320, 240),   // QVGA
            domain::Size(640, 480),   // VGA
            domain::Size(800, 600),   // SVGA
            domain::Size(1024, 768),  // XGA
            domain::Size(1280, 720),  // HD
            domain::Size(1920, 1080)  // Full HD
        };
    }

    std::vector<int> WindowsCameraDiscoveryService::GetSupportedFrameRates(IMFActivate* device_activator) {
        // Return common frame rates
        // In a full implementation, we would query the device's actual capabilities
        return { 15, 30, 60 };
    }

    std::vector<std::string> WindowsCameraDiscoveryService::GetSupportedFormats(IMFActivate* device_activator) {
        // Return common formats
        // In a full implementation, we would query the device's actual capabilities
        return { "YUY2", "NV12", "RGB24", "RGB32", "MJPG" };
    }

    bool WindowsCameraDiscoveryService::TestDeviceAccessibility(IMFActivate* device_activator) {
        Microsoft::WRL::ComPtr<IMFMediaSource> media_source;
        HRESULT hr = device_activator->ActivateObject(IID_PPV_ARGS(&media_source));
        
        if (SUCCEEDED(hr) && media_source) {
            // Successfully activated, camera is accessible
            media_source.Reset();
            return true;
        }
        
        // Failed to activate, camera may be in use or have issues
        return false;
    }

} // namespace golf_sim::camera::infrastructure::windows
