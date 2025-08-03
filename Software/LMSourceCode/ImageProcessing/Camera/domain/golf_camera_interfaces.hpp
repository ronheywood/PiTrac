// Golf Launch Monitor Camera Interfaces
// Clean camera abstractions that bridge to ImageAnalysis domain
#pragma once

#include <vector>
#include <memory>
#include <chrono>
#include <string>

// Forward declaration to ImageAnalysis domain
namespace golf_sim::image_analysis::domain {
    struct ImageBuffer;  // This comes from ImageAnalysis bounded context
}

namespace GolfLaunchMonitor::Camera {
    
    using Duration = std::chrono::milliseconds;
    using CameraId = std::string;
    
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
    
    // Clean camera interface - abstracts ALL format complexity
    // Returns ImageBuffer (defined in ImageAnalysis domain) for analysis
    class IImageCamera {
    public:
        virtual ~IImageCamera() = default;
        
        // Core camera operations - no format exposure
        virtual CameraId GetId() const = 0;
        virtual std::string GetName() const = 0;
        virtual CameraCapabilities GetCapabilities() const = 0;
        
        // Image capture - returns ImageBuffer for ImageAnalysis domain
        virtual golf_sim::image_analysis::domain::ImageBuffer CaptureFrame() = 0;
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
        virtual golf_sim::image_analysis::domain::ImageBuffer CaptureFrame() = 0;
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
        virtual golf_sim::image_analysis::domain::ImageBuffer CaptureStrobedFrame(const StrobeConfiguration& strobe) = 0;
        virtual golf_sim::image_analysis::domain::ImageBuffer CaptureFrame() = 0;
        
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
