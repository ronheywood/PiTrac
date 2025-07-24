/*
 * Windows Camera Implementation
 * 
 * Uses Windows Media Foundation to interface with camera hardware.
 * Implements the domain camera interface for Windows platform.
 */

#pragma once

#include "../../domain/camera_domain.hpp"
#include <Windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <wrl/client.h>

namespace golf_sim::camera::infrastructure::windows {

    /**
     * @brief Windows Media Foundation camera implementation
     */
    class WindowsCamera : public domain::ICamera {
    public:
        WindowsCamera();
        virtual ~WindowsCamera();

        // ICamera interface implementation
        bool Initialize(const domain::CameraConfig& config) override;
        bool StartStreaming() override;
        bool StopStreaming() override;
        domain::CameraFrame CaptureFrame() override;
        bool IsStreaming() const override;
        domain::CameraConfig GetConfig() const override;
        std::vector<domain::Size> GetSupportedResolutions() const override;
        std::string GetDeviceInfo() const override;

    private:
        // Windows Media Foundation objects
        Microsoft::WRL::ComPtr<IMFSourceReader> source_reader_;
        Microsoft::WRL::ComPtr<IMFMediaSource> media_source_;
        
        // Camera state
        domain::CameraConfig current_config_;
        bool is_streaming_;
        bool is_initialized_;
        std::string device_name_;
        
        // Helper methods
        bool InitializeMediaFoundation();
        void CleanupMediaFoundation();
        Microsoft::WRL::ComPtr<IMFMediaSource> CreateCameraMediaSource();
        bool ConfigureSourceReader(const domain::CameraConfig& config);
        std::vector<uint8_t> ConvertSampleToBuffer(IMFSample* sample);
    };

} // namespace golf_sim::camera::infrastructure::windows
