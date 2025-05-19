# Camera Architecture Refactoring Plan

## Current Architecture

```
High Level Interface
└── GolfSimCamera (gs_camera.h)
    └── CameraHardware (camera_hardware.h/cpp)
        └── LibCameraInterface (libcamera_interface.h/cpp)
            └── Platform-Specific Implementations
                ├── LibcameraJpegApp (still images)
                ├── LibcameraEncoder (video encoding)
                └── RPiCamApp (base Raspberry Pi camera ops)
```

## Architectural Strengths to Preserve

1. **Clear Separation of Camera Roles**
   - Motion detection and strobe capture are distinct
   - Each camera has well-defined responsibilities
   - Specialized modes for each camera type

2. **Flexible Camera Modes**
   - Support for different operational modes (full res, cropped, calibration)
   - Dynamic mode switching during operation
   - Extensible mode system for future requirements

3. **Hardware Abstraction Layers**
   - Basic camera abstraction through GolfSimCamera
   - Platform-specific implementation isolation
   - Separation of image processing from hardware control

4. **Performance Optimizations**
   - Efficient frame handling for motion detection
   - Specialized capture modes for high-speed operation
   - Optimized strobe synchronization

## Identified Opportunities

1. Platform-specific code management
   - Avoid use of preprocessor directives
   - Tight coupling between hardware and platform layers
2. Cross-platform support
   - Windows implementation currently stubbed out
   - Need for consistent behavior across platforms
3. Interface design
   - Lack of clear contracts for camera use cases
   - Need for better separation between camera types

## Camera Roles and Modes

### Camera 1 (Motion Detection)
- **Purpose**: Watch teed-up ball and detect initial motion
- **Modes**:
  1. Full Resolution Watching (Default)
     - Used for initial ball placement and calibration
  2. Cropped High FPS Motion Detection
     - Dynamically cropped sensor region for higher framerates
     - Real-time motion detection processing
  3. Calibration Mode
     - Still image capture for system calibration

### Camera 2 (Shot Capture)
- **Purpose**: Capture ball flight data with strobed lighting
- **Modes**:
  1. Still Image Mode
     - Used for calibration and setup
  2. Strobed Capture Mode
     - Multiple ball positions in single frame
     - Synchronized with strobe lighting
  3. External Strobe Mode
     - Special mode for multi-system setups

## Proposed Architecture

### 1. Pure Interface Definitions

The architecture uses two main types of interfaces that serve different purposes:

1. **ICameraDevice** - The logical camera abstraction
   - Defines what a camera can do from the application's perspective
   - Provides high-level operations like initialization and capture
   - Handles business operations and mode settings
   - Used by application code without knowledge of hardware details
   - Specialized into `IMotionCamera` and `IStrobeCamera` for specific use cases

2. **ICameraAdapter** - The hardware abstraction layer
   - Handles platform-specific implementation details
   - Manages low-level hardware operations
   - Deals with hardware buffers and metadata
   - Isolates platform-specific code (RPi vs Windows)
   - Implemented differently for each platform

The relationship follows the Adapter pattern:
- `ICameraDevice` implementations use an `ICameraAdapter` internally
- The adapter translates high-level operations into platform-specific calls
- This separation allows platform implementations to change without affecting application code
- Enables testing with mock adapters and keeps platform-specific code contained

Following the Interface Segregation Principle, we'll split the camera interfaces into a base interface and specialized interfaces for each camera type:

```cpp
// Base camera interface - common operations
class ICameraDevice {
public:
    virtual ~ICameraDevice() = default;
    
    // Basic operations
    virtual bool initialize() = 0;
    virtual bool capture() = 0;
    
    // Mode configuration
    virtual void setMode(CameraMode mode) = 0;
    virtual void configure(const CameraConfig& config) = 0;
    
    // Sensor control
    virtual bool setCropRegion(const Region& crop) = 0;
    virtual bool setExposure(const ExposureSettings& settings) = 0;
    
    // Frame handling
    virtual std::optional<Frame> getLatestFrame() = 0;
    virtual void processFrame(const Frame& frame) = 0;
};

// Motion detection camera interface
class IMotionCamera : public ICameraDevice {
public:
    virtual ~IMotionCamera() = default;
    
    // Motion detection specific operations
    virtual bool startMotionDetection() = 0;
    virtual bool stopMotionDetection() = 0;
    virtual void setMotionThreshold(float threshold) = 0;
    virtual void setDetectionRegion(const Region& region) = 0;
    
    // Motion events
    virtual void onMotionDetected(const MotionEvent& event) = 0;
};

// Strobe camera interface
class IStrobeCamera : public ICameraDevice {
public:
    virtual ~IStrobeCamera() = default;
    
    // Strobe control operations
    virtual bool configureStrobeSync() = 0;
    virtual bool triggerStrobe() = 0;
    virtual void setStrobeSequence(const StrobeSequence& sequence) = 0;
    virtual bool isStrobeReady() = 0;
    
    // Multi-exposure frame handling
    virtual std::optional<MultiExposureFrame> captureStrobeSequence() = 0;
};

// Platform-specific adapter interface
class ICameraAdapter {
public:
    virtual ~ICameraAdapter() = default;
    
    // Core camera operations
    virtual bool openCamera() = 0;
    virtual bool configureCamera(const CameraConfig& config) = 0;
    virtual bool startCamera() = 0;
    virtual bool stopCamera() = 0;
    
    // Hardware-specific features
    virtual bool setSensorMode(SensorMode mode) = 0;
    virtual bool configureStreaming(const StreamConfig& config) = 0;
    
    // Data handling
    virtual bool setupBuffers(size_t count) = 0;
    virtual bool queueBuffer(Buffer& buffer) = 0;
    virtual bool processMetadata(const Metadata& meta) = 0;
};
```

### 2. Concrete Adapter Implementations

```cpp
// RPi Motion Detection Camera Adapter
class RpiMotionCameraAdapter : public ICameraAdapter {
private:
    std::unique_ptr<RPiCamEncoder> encoder_;
    std::unique_ptr<Motion_Detect> motion_detector_;
    Region crop_region_;
    
public:
    bool openCamera() override;
    bool configureCamera(const CameraConfig& config) override;
    bool setupMotionDetection(const MotionConfig& config);
    bool processCroppedFrame(const Frame& frame);
};

// RPi Strobe Camera Adapter
class RpiStrobeCameraAdapter : public ICameraAdapter {
private:
    std::unique_ptr<LibcameraJpegApp> still_app_;
    std::unique_ptr<StrobeController> strobe_controller_;
    
public:
    bool openCamera() override;
    bool configureCamera(const CameraConfig& config) override;
    bool setupStrobeSync(const StrobeConfig& config);
    bool captureStrobedSequence();
};

// Windows Motion Camera Adapter (future)
class WindowsMotionCameraAdapter : public ICameraAdapter {
    // Windows implementation for motion detection camera
};

// Windows Strobe Camera Adapter (future)
class WindowsStrobeCameraAdapter : public ICameraAdapter {
    // Windows implementation for strobe camera
};
```

### 3. Factory Pattern Implementation

```cpp
// Configuration structures
struct MotionCameraConfig {
    CameraMode mode;              // Full res, cropped, calibration
    Region crop_region;           // For high FPS mode
    float motion_threshold;       // Motion sensitivity
    int fps;                     // Target framerate
};

struct StrobeCameraConfig {
    CameraMode mode;             // Still, strobed, external
    StrobeSettings strobe;       // Strobe timing/sync
    ExposureSettings exposure;   // Exposure settings
    bool external_trigger;       // External trigger mode
};

// Factory for creating specialized cameras
class CameraFactory {
public:
    // Create motion detection camera (Camera 1)
    static std::unique_ptr<ICameraDevice> createMotionCamera(
        const MotionCameraConfig& config
    );
    
    // Create strobe capture camera (Camera 2)
    static std::unique_ptr<ICameraDevice> createStrobeCamera(
        const StrobeCameraConfig& config
    );

private:
    // Platform-specific adapter creation
    static std::unique_ptr<ICameraAdapter> createMotionAdapter();
    static std::unique_ptr<ICameraAdapter> createStrobeAdapter();
};

// High-level camera manager
class DualCameraSystem {
private:
    std::unique_ptr<ICameraDevice> motion_camera_;
    std::unique_ptr<ICameraDevice> strobe_camera_;
    CameraState system_state_;
    
public:
    bool initialize();
    bool startBallMonitoring();
    bool handleMotionDetected();
    bool captureShot();
    void processResults();
};
```

## Benefits

1. **Clean Separation of Concerns**
   - Clear boundaries between interface and implementation
   - Platform-specific code isolated in adapters
   - Reduced preprocessor directive usage

2. **Improved Testability**
   - Interfaces enable mock implementations
   - Easier unit testing through abstraction
   - Better integration testing capabilities

3. **Enhanced Maintainability**
   - Easier to add new camera types
   - Simpler platform-specific implementations
   - Better code organization

4. **Future-Proof Design**
   - Ready for Windows implementation
   - Scalable for new camera types
   - Flexible for future requirements

## Implementation Strategy

### Phase 1: Interface Creation
1. Define ICameraDevice interface
2. Define ICameraAdapter interface
3. Create CameraConfig structure

### Phase 2: RPi Adaptation
1. Refactor existing RPi implementation into LibCameraAdapter
2. Update GolfSimCamera to use new interfaces
3. Implement CameraFactory

### Phase 3: Windows Support
1. Create WindowsCameraAdapter implementation
2. Test cross-platform compatibility
3. Document Windows camera setup

### Phase 4: Testing & Validation
1. Create unit tests for interfaces
2. Implement integration tests
3. Validate both platforms

## Migration Notes

- Maintain backward compatibility during transition
- Implement changes incrementally
- Add comprehensive documentation
- Create test cases before implementation

## Future Considerations

1. Support for additional camera types
2. Enhanced error handling
3. Runtime camera configuration
4. Performance optimizations
5. Extended platform support

## Related Files

- `gs_camera.h`
- `camera_hardware.h/cpp`
- `libcamera_interface.h/cpp`
- `ball_watcher.cpp`
