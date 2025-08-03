// Camera Domain - Independent Image Representation
// Clean camera abstractions with NO dependencies on other bounded contexts
#pragma once

#include <vector>
#include <memory>
#include <chrono>
#include <string>
#include <cstdint>

namespace GolfLaunchMonitor::Camera {
    
    using Duration = std::chrono::milliseconds;
    using CameraId = std::string;
    using Timestamp = std::chrono::microseconds;
    
    // Value Objects for camera configuration
    struct Rectangle {
        int x, y, width, height;
        Rectangle(int x, int y, int w, int h) : x(x), y(y), width(w), height(h) {}
    };
    
    struct StrobeConfiguration {
        Duration delay;           // Delay before strobe fires
        Duration duration;        // How long strobe stays on
        int intensity;           // Brightness 0-100
        
        StrobeConfiguration(Duration d, Duration dur, int i) 
            : delay(d), duration(dur), intensity(i) {}
            
        bool IsValid() const {
            return delay >= Duration(0) && 
                   duration > Duration(0) && 
                   intensity >= 0 && intensity <= 100;
        }
    };
    
    // Camera's OWN image representation - completely independent
    struct CameraImage {
        std::vector<uint8_t> pixelData;      // Raw pixel data
        int width;                           // Image width
        int height;                          // Image height
        int channels;                        // Number of channels (1=grayscale, 3=RGB, 4=RGBA)
        std::string format;                  // Format description ("RGB", "BGR", "RGBA", "NV12", etc.)
        Timestamp captureTime;               // When image was captured
        CameraId sourceCamera;               // Which camera captured this
        uint64_t sequenceNumber;             // Frame sequence number
        std::string metadata;                // Additional capture metadata
        
        CameraImage() = default;
        
        CameraImage(std::vector<uint8_t> data, int w, int h, int ch, 
                   const std::string& fmt, Timestamp ts, const CameraId& camera,
                   uint64_t seq = 0, const std::string& meta = "")
            : pixelData(std::move(data)), width(w), height(h), channels(ch)
            , format(fmt), captureTime(ts), sourceCamera(camera)
            , sequenceNumber(seq), metadata(meta) {}
        
        bool IsValid() const {
            return !pixelData.empty() && width > 0 && height > 0 && channels > 0;
        }
        
        size_t GetExpectedDataSize() const {
            return static_cast<size_t>(width * height * channels);
        }
        
        bool HasValidDataSize() const {
            return pixelData.size() == GetExpectedDataSize();
        }
    };
    
    // Camera identification and metadata
    struct CameraCapabilities {
        std::vector<std::pair<int, int>> supportedResolutions;
        std::vector<int> supportedFrameRates;
        bool supportsLongExposure;
        bool supportsStrobeSync;
        std::string deviceName;
        
        bool IsValid() const { 
            return !supportedResolutions.empty() && !deviceName.empty(); 
        }
    };
    
    // Clean camera interface - returns Camera domain's own image type
    class IImageCamera {
    public:
        virtual ~IImageCamera() = default;
        
        // Core camera operations - no external dependencies
        virtual CameraId GetId() const = 0;
        virtual std::string GetName() const = 0;
        virtual CameraCapabilities GetCapabilities() const = 0;
        
        // Image capture - returns CameraImage (Camera domain's own type)
        virtual CameraImage CaptureFrame() = 0;
        virtual bool IsStreamingActive() const = 0;
        
        // Camera lifecycle
        virtual void StartStreaming() = 0;
        virtual void StopStreaming() = 0;
        
        // Configuration
        virtual void SetCaptureArea(const Rectangle& area) = 0;
        virtual void SetResolution(int width, int height) = 0;
        virtual void SetFrameRate(int fps) = 0;
    };
    
    // Golf-specific camera roles (uses IImageCamera)
    class ITeeCamera {
    public:
        virtual ~ITeeCamera() = default;
        
        // Delegate to underlying camera for image capture
        virtual CameraImage CaptureFrame() = 0;
        virtual void SetCaptureArea(const Rectangle& area) = 0;
        virtual bool IsStreamingActive() const = 0;
        
        // Camera lifecycle
        virtual void StartStreaming() = 0;
        virtual void StopStreaming() = 0;
        
        // Golf-specific metadata
        virtual CameraId GetCameraId() const = 0;
        virtual std::string GetCameraName() const = 0;
    };
    
    class IFlightCamera {
    public:
        virtual ~IFlightCamera() = default;
        
        // Image capture with strobe coordination
        virtual CameraImage CaptureStrobedFrame(const StrobeConfiguration& strobe) = 0;
        virtual CameraImage CaptureFrame() = 0;
        
        // Long exposure support
        virtual void StartLongExposure() = 0;
        virtual void TriggerCapture() = 0;
        virtual bool IsLongExposureActive() const = 0;
        
        // Camera lifecycle
        virtual void StartStreaming() = 0;
        virtual void StopStreaming() = 0;
        
        // Golf-specific metadata
        virtual CameraId GetCameraId() const = 0;
        virtual std::string GetCameraName() const = 0;
    };
    
    // Strobe control abstraction
    class IStrobeController {
    public:
        virtual ~IStrobeController() = default;
        
        virtual void TriggerStrobe(const StrobeConfiguration& config) = 0;
        virtual bool IsStrobeReady() const = 0;
        virtual void CalibrateStrobe() = 0;
    };
    
    // Camera discovery and factory
    class ICameraFactory {
    public:
        virtual ~ICameraFactory() = default;
        
        virtual std::vector<std::unique_ptr<IImageCamera>> DiscoverCameras() = 0;
        virtual std::unique_ptr<IImageCamera> CreateCamera(const CameraId& id) = 0;
        virtual bool IsCameraAvailable(const CameraId& id) const = 0;
    };
    
    // Golf camera assignment service
    class IGolfCameraAssignmentService {
    public:
        virtual ~IGolfCameraAssignmentService() = default;
        
        virtual std::unique_ptr<ITeeCamera> AssignTeeCamera(const CameraId& id) = 0;
        virtual std::unique_ptr<IFlightCamera> AssignFlightCamera(const CameraId& id) = 0;
        virtual std::vector<CameraId> GetAvailableCameras() const = 0;
        virtual void RefreshCameraList() = 0;
    };
    
} // namespace GolfLaunchMonitor::Camera
