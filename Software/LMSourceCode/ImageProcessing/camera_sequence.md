# Camera System Sequence Documentation

## Overview
This document describes the sequence of interactions between components in the PiTrac dual-camera system, focusing on the new architecture that employs interface-based design and the adapter pattern.

## Key Components

### ICameraDevice Interface
The `ICameraDevice` interface serves as the primary abstraction for camera operations, defining a contract that both motion detection and strobe capture cameras must implement:

```cpp
class ICameraDevice {
    // Basic operations
    initialize()
    capture()
    
    // Mode configuration
    setMode(CameraMode)
    configure(CameraConfig)
    
    // Motion detection capabilities
    startMotionDetection()
    stopMotionDetection()
    setMotionThreshold()
    
    // Strobe control
    configureStrobeSync()
    triggerStrobe()
    
    // Sensor and frame handling
    setCropRegion()
    getLatestFrame()
    processFrame()
}
```

### Component Responsibilities

1. **DualCameraSystem**
   - Manages the lifecycle of both cameras
   - Coordinates between motion detection and shot capture
   - Processes results from both cameras
   - Main orchestrator of the camera subsystem

2. **CameraFactory**
   - Creates specialized camera instances
   - Handles platform-specific adapter creation
   - Configures cameras based on their roles
   - Ensures proper initialization sequence

3. **Motion Camera (Camera 1)**
   - Implements ICameraDevice for motion detection
   - Manages crop region for high-FPS mode
   - Processes frames for ball movement
   - Triggers shot capture sequence

4. **Strobe Camera (Camera 2)**
   - Implements ICameraDevice for shot capture
   - Handles strobe synchronization
   - Manages exposure settings
   - Captures multi-ball position frames

## Sequence Flow

### 1. System Initialization
- DualCameraSystem requests camera instances from factory
- Factory creates appropriate adapters for the platform
- Each camera is initialized with its specific configuration
- Platform-specific setup is handled by adapters

### 2. Ball Monitoring
- Motion camera configures for ball detection
- Adapter sets up crop region for optimal performance
- Camera starts continuous monitoring through adapter

### 3. Ball Motion Detection
- Motion is detected and reported to system
- System coordinates camera state transitions
- Strobe camera is prepared for capture

### 4. Shot Capture
- Strobe camera configured through adapter
- Synchronized capture sequence executed
- Frame captured with multiple ball positions

### 5. Results Processing
- System retrieves captured frame
- Results processed for trajectory analysis
- System ready for next capture

## Benefits of New Architecture

1. **Clean Separation of Concerns**
   - Hardware abstraction through interfaces
   - Platform-specific code isolated in adapters
   - Clear component boundaries

2. **Improved Testability**
   - Components can be tested in isolation
   - Mock implementations are straightforward
   - Better integration testing support

3. **Enhanced Maintainability**
   - Easy to add new camera types
   - Platform-specific code is contained
   - Clear component responsibilities

4. **Future-Proof Design**
   - Ready for Windows implementation
   - Extensible for new camera types
   - Flexible configuration options

## Implementation Notes
- Each camera type has its own adapter implementation
- Configuration is passed during initialization
- Error handling occurs at appropriate abstraction levels
- State management is handled by DualCameraSystem

## Notes
The sequence diagram is displayed in `camera_sequence.puml` and can be rendered with PlantUML-compatible tools.
