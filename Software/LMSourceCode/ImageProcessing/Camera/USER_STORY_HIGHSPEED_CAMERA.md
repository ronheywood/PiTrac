# User Story: High-Speed Infrared Camera Integration for Ball Launch Detection

## Background

The current system uses a tightly focused natural light camera to detect ball movement, which then triggers a strobe for high-speed image capture. This approach is fragile, and difficult to experiment due to hardware timing dependencies.


## Proposal

We propose a new architecture for ball launch detection and flight analysis using two synchronized infrared cameras and a continuously running strobe. The system will:

- The strobe is always running, illuminating the entire area (no per-shot triggering).
- Camera 1 performs a 25ms exposure.
- 12.5ms after Camera 1 starts, Camera 2 begins its own 25ms exposure (resulting in a 12.5ms overlap between the two exposures).
- When either camera delivers an image, it immediately starts a new exposure with no delay.
- Every time an image is delivered from either camera, it is analyzed in real time for circles (balls).
- As soon as any image contains more than one circle, that image is marked as the launch frame.
- The launch frame and the next image delivered from the other camera are used together to calculate ball flight data. (This ensures the full flight is captured, even if it spans two images.)
- The analysis loop is paused while flight data is calculated, then restarted for the next shot.
- Sometimes the entire flight will be visible in a single image; other times, two images may need to be combined for complete flight data.
- Both cameras are hardware-triggered for precise synchronization and overlap.
- Both cameras are infrared, optimized for the strobe wavelength.

## User Story

**As a** developer or researcher,
**I want** to experiment with a high-speed, strobe-illuminated, dual-infrared camera setup,
**so that** I can reliably detect ball launch events and analyze ball flight without the complexity and fragility of legacy movement-triggered strobe logic.

### Acceptance Criteria
- The system can analyze a single image in ≤25ms and reliably detect 1 vs 2+ circles (balls) with no false positives.
- Two cameras are hardware-triggered and exposures are precisely overlapped.
- The strobe runs continuously and is not triggered by software or movement detection.
- The system can extract the launch frame from camera 1 and the corresponding frame from camera 2 ready for flight analysis in the ImageAnalysis module.
- The infrastructure allows for rapid experimentation and tuning of exposure/overlap parameters.

## Architectural Notes

### Infrastructure Layer in Camera Bounded Context
- The infrastructure layer will provide the strobe implementation (e.g., continuous pattern, hardware interface, configuration).
- The strobe logic is encapsulated and not exposed to the domain or application layers.
- The infrastructure can be swapped for different strobe hardware or patterns without affecting the rest of the system.

### HighSpeedCameraService
- Exposes a simple API for high-speed image acquisition and launch detection.
- Can be configured for different exposure/overlap strategies.
- Designed for rapid prototyping and experimentation.

### Benefits
- Removes the need for fragile, movement-triggered strobe logic.
- Greatly simplifies the experiment and tuning process.
- Enables robust, hardware-synchronized, high-speed ball launch detection.
- Decouples strobe and camera logic for maximum flexibility.

---

## Next Steps
- Implement the infrastructure strobe service in the Camera bounded context.
- Develop the HighSpeedCameraService with a pluggable strobe interface.
- Build a test harness for rapid experimentation with exposure and overlap parameters.
- Validate real-time performance and detection accuracy.
