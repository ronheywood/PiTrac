/*
 * Camera Domain Exceptions
 * 
 * Defines exception types for camera-related errors in the PiTrac system.
 * Provides clear, actionable error information for camera operations.
 */

#pragma once

#include <stdexcept>
#include <string>

namespace golf_sim::camera::domain {

    /**
     * Base exception for all camera-related errors
     */
    class CameraException : public std::runtime_error {
    public:
        explicit CameraException(const std::string& message)
            : std::runtime_error("Camera Error: " + message) {}
    };

    /**
     * Thrown when camera initialization fails
     */
    class CameraInitializationException : public CameraException {
    public:
        explicit CameraInitializationException(const std::string& details)
            : CameraException("Failed to initialize camera - " + details) {}
    };

    /**
     * Thrown when camera is already in use by another process
     */
    class CameraBusyException : public CameraException {
    public:
        explicit CameraBusyException(const std::string& camera_name = "Unknown")
            : CameraException("Camera '" + camera_name + "' is currently in use by another application. "
                            "Please close other camera applications and try again.") {}
    };

    /**
     * Thrown when requested camera is not found
     */
    class CameraNotFoundException : public CameraException {
    public:
        explicit CameraNotFoundException(const std::string& camera_id)
            : CameraException("Camera '" + camera_id + "' not found. "
                            "Please check camera connection and drivers.") {}
    };

    /**
     * Thrown when camera streaming operations fail
     */
    class CameraStreamingException : public CameraException {
    public:
        explicit CameraStreamingException(const std::string& operation, const std::string& details)
            : CameraException("Streaming operation '" + operation + "' failed - " + details) {}
    };

    /**
     * Thrown when camera configuration is invalid or unsupported
     */
    class CameraConfigurationException : public CameraException {
    public:
        explicit CameraConfigurationException(const std::string& config_issue)
            : CameraException("Invalid camera configuration - " + config_issue) {}
    };

    /**
     * Thrown when camera hardware or driver issues are detected
     */
    class CameraHardwareException : public CameraException {
    public:
        explicit CameraHardwareException(const std::string& hardware_issue)
            : CameraException("Camera hardware error - " + hardware_issue + 
                            ". Please check Device Manager and camera drivers.") {}
    };

} // namespace golf_sim::camera::domain
