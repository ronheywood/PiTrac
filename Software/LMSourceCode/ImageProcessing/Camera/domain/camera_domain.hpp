/*
 * Domain interface for Camera bounded context
 * 
 * This file defines the common interface for camera operations,
 * independent of platform implementation.
 * 
 * Step 1 of incremental refactoring: Define domain interface only.
 * No existing files are modified in this step.
 */

#pragma once

#include <memory>
#include <vector>
#include <string>
#include <cstdint>

namespace golf_sim::camera::domain {
    
    // Domain types that abstract platform-specific implementations
    // These provide a clean interface for the application layer
    
    struct Size {
        unsigned int width, height;
        Size(unsigned int w = 0, unsigned int h = 0) : width(w), height(h) {}
    };

    struct Transform {
        int value = 0;
        Transform(int v = 0) : value(v) {}
    };

    struct PixelFormat {
        unsigned int fourcc = 0;
        PixelFormat(unsigned int f = 0) : fourcc(f) {}
    };

    struct ColorSpace {
        int value = 0;
        ColorSpace(int v = 0) : value(v) {}
    };

    /**
     * @brief Represents a single camera frame with metadata
     */
    struct CameraFrame {
        std::vector<uint8_t> data;        // Raw image data
        Size resolution;                   // Frame dimensions  
        PixelFormat format;               // Pixel format (YUYV, MJPEG, etc.)
        uint64_t timestamp_us;            // Capture timestamp in microseconds
        uint32_t sequence_number;         // Frame sequence number
        
        CameraFrame() = default;
        CameraFrame(std::vector<uint8_t> img_data, Size res, PixelFormat fmt, 
                   uint64_t ts = 0, uint32_t seq = 0)
            : data(std::move(img_data)), resolution(res), format(fmt), 
              timestamp_us(ts), sequence_number(seq) {}
    };

    /**
     * @brief Camera configuration parameters
     */
    struct CameraConfig {
        Size resolution{640, 480};
        PixelFormat format{0};            // Default format
        uint32_t fps{30};                 // Frames per second
        uint32_t buffer_count{4};         // Number of buffers for streaming
    };

    /**
     * @brief Abstract camera interface for hardware abstraction
     */
    class ICamera {
    public:
        virtual ~ICamera() = default;

        /**
         * @brief Initialize the camera with given configuration
         * @param config Camera configuration parameters
         * @return true if initialization successful, false otherwise
         */
        virtual bool Initialize(const CameraConfig& config) = 0;

        /**
         * @brief Start streaming from the camera
         * @return true if streaming started successfully, false otherwise
         */
        virtual bool StartStreaming() = 0;

        /**
         * @brief Stop streaming from the camera
         * @return true if streaming stopped successfully, false otherwise
         */
        virtual bool StopStreaming() = 0;

        /**
         * @brief Capture a single frame from the camera buffer
         * @return CameraFrame object with image data, or empty frame if failed
         */
        virtual CameraFrame CaptureFrame() = 0;

        /**
         * @brief Check if camera is currently streaming
         * @return true if streaming, false otherwise
         */
        virtual bool IsStreaming() const = 0;

        /**
         * @brief Get the current camera configuration
         * @return Current camera configuration
         */
        virtual CameraConfig GetConfig() const = 0;

        /**
         * @brief Get supported resolutions for this camera
         * @return Vector of supported resolutions
         */
        virtual std::vector<Size> GetSupportedResolutions() const = 0;

        /**
         * @brief Get camera device information
         * @return Human-readable camera device name/description
         */
        virtual std::string GetDeviceInfo() const = 0;
    };

} // namespace golf_sim::camera::domain
