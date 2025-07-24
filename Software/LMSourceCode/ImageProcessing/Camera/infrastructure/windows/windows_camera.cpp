/*
 * Windows Camera Implementation
 * 
 * Uses Windows Media Foundation to interface with camera hardware.
 * Implements the domain camera interface for Windows platform.
 */

#include "windows_camera.hpp"
#include <iostream>
#include <chrono>

#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

namespace golf_sim::camera::infrastructure::windows {

    WindowsCamera::WindowsCamera() 
        : is_streaming_(false), is_initialized_(false), device_name_("Windows Camera") {
    }

    WindowsCamera::~WindowsCamera() {
        if (is_streaming_) {
            StopStreaming();
        }
        CleanupMediaFoundation();
    }

    bool WindowsCamera::Initialize(const domain::CameraConfig& config) {
        if (is_initialized_) {
            return true; // Already initialized
        }

        if (!InitializeMediaFoundation()) {
            return false;
        }

        // Create camera media source
        media_source_ = CreateCameraMediaSource();
        if (!media_source_) {
            std::cerr << "Failed to create camera media source" << std::endl;
            return false;
        }

        // Create source reader
        HRESULT hr = MFCreateSourceReaderFromMediaSource(
            media_source_.Get(),
            nullptr,
            &source_reader_
        );

        if (FAILED(hr)) {
            std::cerr << "Failed to create source reader: " << std::hex << hr << std::endl;
            return false;
        }

        // Configure the source reader with desired format
        if (!ConfigureSourceReader(config)) {
            return false;
        }

        current_config_ = config;
        is_initialized_ = true;
        return true;
    }

    bool WindowsCamera::StartStreaming() {
        if (!is_initialized_) {
            std::cerr << "Camera not initialized" << std::endl;
            return false;
        }

        if (is_streaming_) {
            return true; // Already streaming
        }

        // With Media Foundation, we don't explicitly "start" streaming
        // The source reader will read frames on demand
        is_streaming_ = true;
        return true;
    }

    bool WindowsCamera::StopStreaming() {
        if (!is_streaming_) {
            return true; // Already stopped
        }

        is_streaming_ = false;
        return true;
    }

    domain::CameraFrame WindowsCamera::CaptureFrame() {
        if (!is_streaming_ || !source_reader_) {
            return domain::CameraFrame(); // Return empty frame
        }

        Microsoft::WRL::ComPtr<IMFSample> sample;
        DWORD stream_flags = 0;
        LONGLONG timestamp = 0;

        HRESULT hr = source_reader_->ReadSample(
            MF_SOURCE_READER_FIRST_VIDEO_STREAM,
            0,                          // No flags
            nullptr,                    // Don't need stream index
            &stream_flags,
            &timestamp,
            &sample
        );

        if (FAILED(hr)) {
            std::cerr << "Failed to read sample: " << std::hex << hr << std::endl;
            return domain::CameraFrame();
        }

        if (stream_flags & MF_SOURCE_READERF_ERROR) {
            std::cerr << "Source reader error flag set" << std::endl;
            return domain::CameraFrame();
        }

        if (!sample) {
            // No sample available right now, return empty frame
            return domain::CameraFrame();
        }

        // Convert sample to our domain frame format
        auto frame_data = ConvertSampleToBuffer(sample.Get());
        if (frame_data.empty()) {
            return domain::CameraFrame();
        }

        // Create timestamp in microseconds
        auto now = std::chrono::high_resolution_clock::now();
        auto timestamp_us = std::chrono::duration_cast<std::chrono::microseconds>(
            now.time_since_epoch()).count();

        static uint32_t sequence = 0;
        return domain::CameraFrame(
            std::move(frame_data),
            current_config_.resolution,
            current_config_.format,
            static_cast<uint64_t>(timestamp_us),
            ++sequence
        );
    }

    bool WindowsCamera::IsStreaming() const {
        return is_streaming_;
    }

    domain::CameraConfig WindowsCamera::GetConfig() const {
        return current_config_;
    }

    std::vector<domain::Size> WindowsCamera::GetSupportedResolutions() const {
        // Return common resolutions supported by most cameras
        return {
            domain::Size(320, 240),   // QVGA
            domain::Size(640, 480),   // VGA
            domain::Size(800, 600),   // SVGA
            domain::Size(1024, 768),  // XGA
            domain::Size(1280, 720),  // HD 720p
            domain::Size(1920, 1080)  // HD 1080p
        };
    }

    std::string WindowsCamera::GetDeviceInfo() const {
        return device_name_;
    }

    // Private helper methods

    bool WindowsCamera::InitializeMediaFoundation() {
        HRESULT hr = MFStartup(MF_VERSION);
        if (FAILED(hr)) {
            std::cerr << "Failed to initialize Media Foundation: " << std::hex << hr << std::endl;
            return false;
        }
        return true;
    }

    void WindowsCamera::CleanupMediaFoundation() {
        MFShutdown();
    }

    Microsoft::WRL::ComPtr<IMFMediaSource> WindowsCamera::CreateCameraMediaSource() {
        Microsoft::WRL::ComPtr<IMFAttributes> attributes;
        IMFActivate** devices = nullptr; // Raw pointer array, not ComPtr
        Microsoft::WRL::ComPtr<IMFMediaSource> media_source;
        UINT32 device_count = 0;

        // Create attributes for device enumeration
        HRESULT hr = MFCreateAttributes(&attributes, 1);
        if (FAILED(hr)) {
            std::cerr << "Failed to create attributes: " << std::hex << hr << std::endl;
            return nullptr;
        }

        // Set attribute to enumerate video capture devices
        hr = attributes->SetGUID(
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
        );
        if (FAILED(hr)) {
            std::cerr << "Failed to set device type attribute: " << std::hex << hr << std::endl;
            return nullptr;
        }

        // Enumerate devices
        hr = MFEnumDeviceSources(attributes.Get(), &devices, &device_count);
        if (FAILED(hr)) {
            std::cerr << "No camera devices found or enumeration failed: " << std::hex << hr << std::endl;
            // Clean up devices array even if enumeration failed
            if (devices) {
                CoTaskMemFree(devices);
            }
            return nullptr;
        }
        
        if (device_count == 0) {
            std::cerr << "No camera devices found" << std::endl;
            // Clean up devices array
            if (devices) {
                CoTaskMemFree(devices);
            }
            return nullptr;
        }

        // Use the first available camera
        hr = devices[0]->ActivateObject(IID_PPV_ARGS(&media_source));
        
        // Clean up device list
        for (UINT32 i = 0; i < device_count; i++) {
            devices[i]->Release();
        }
        CoTaskMemFree(devices);

        if (FAILED(hr)) {
            std::cerr << "Failed to activate camera device: " << std::hex << hr << std::endl;
            return nullptr;
        }

        return media_source;
    }

    bool WindowsCamera::ConfigureSourceReader(const domain::CameraConfig& config) {
        if (!source_reader_) {
            return false;
        }

        // Instead of forcing a specific format, let's see what the camera supports
        // and try to find the best match
        
        // First, try to get the current/native media type
        Microsoft::WRL::ComPtr<IMFMediaType> native_type;
        HRESULT hr = source_reader_->GetCurrentMediaType(
            MF_SOURCE_READER_FIRST_VIDEO_STREAM,
            &native_type
        );
        
        if (SUCCEEDED(hr)) {
            // Camera has a current format, let's use it
            std::cerr << "Using camera's native format" << std::endl;
            return true;
        }
        
        // If no current format, try common formats that most cameras support
        std::vector<GUID> formats_to_try = {
            MFVideoFormat_YUY2,    // YUV 4:2:2 (very common)
            MFVideoFormat_NV12,    // YUV 4:2:0 (common)
            MFVideoFormat_RGB24,   // RGB 24-bit
            MFVideoFormat_RGB32,   // RGB 32-bit with alpha
            MFVideoFormat_MJPG     // Motion JPEG (very common)
        };
        
        for (const auto& format : formats_to_try) {
            Microsoft::WRL::ComPtr<IMFMediaType> media_type;
            
            // Create media type for this format
            hr = MFCreateMediaType(&media_type);
            if (FAILED(hr)) continue;

            // Set major type to video
            hr = media_type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
            if (FAILED(hr)) continue;

            // Set subtype to current format we're trying
            hr = media_type->SetGUID(MF_MT_SUBTYPE, format);
            if (FAILED(hr)) continue;

            // Set frame size
            hr = MFSetAttributeSize(media_type.Get(), MF_MT_FRAME_SIZE, 
                                   config.resolution.width, config.resolution.height);
            if (FAILED(hr)) continue;

            // Try to set this media type
            hr = source_reader_->SetCurrentMediaType(
                MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                nullptr,
                media_type.Get()
            );

            if (SUCCEEDED(hr)) {
                std::cerr << "Successfully configured camera format" << std::endl;
                return true;
            }
        }
        
        // If we get here, none of our preferred formats worked
        // Try to use any available media type from the camera
        DWORD type_index = 0;
        while (true) {
            Microsoft::WRL::ComPtr<IMFMediaType> available_type;
            hr = source_reader_->GetNativeMediaType(
                MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                type_index,
                &available_type
            );
            
            if (FAILED(hr)) {
                break; // No more media types available
            }
            
            // Try to set this available type
            hr = source_reader_->SetCurrentMediaType(
                MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                nullptr,
                available_type.Get()
            );
            
            if (SUCCEEDED(hr)) {
                std::cerr << "Using camera's available format at index " << type_index << std::endl;
                return true;
            }
            
            type_index++;
        }

        std::cerr << "Failed to configure any camera format" << std::endl;
        return false;
    }

    std::vector<uint8_t> WindowsCamera::ConvertSampleToBuffer(IMFSample* sample) {
        if (!sample) {
            return {};
        }

        Microsoft::WRL::ComPtr<IMFMediaBuffer> buffer;
        HRESULT hr = sample->ConvertToContiguousBuffer(&buffer);
        if (FAILED(hr)) {
            std::cerr << "Failed to convert sample to buffer: " << std::hex << hr << std::endl;
            return {};
        }

        BYTE* byte_buffer = nullptr;
        DWORD buffer_length = 0;
        hr = buffer->Lock(&byte_buffer, nullptr, &buffer_length);
        if (FAILED(hr)) {
            std::cerr << "Failed to lock buffer: " << std::hex << hr << std::endl;
            return {};
        }

        // Copy data to vector
        std::vector<uint8_t> result(byte_buffer, byte_buffer + buffer_length);

        // Unlock buffer
        buffer->Unlock();

        return result;
    }

} // namespace golf_sim::camera::infrastructure::windows
