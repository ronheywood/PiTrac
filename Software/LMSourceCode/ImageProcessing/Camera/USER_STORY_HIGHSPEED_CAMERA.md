# User Story: High-Speed Infrared Camera Integration for Ball Launch Detection

## Background

The current system uses a tightly focused natural light camera to detect ball movement, which then triggers a strobe for high-speed image capture. This approach is fragile, and difficult to experiment due to hardware timing dependencies.

## Proposal

We propose a new architecture for ball launch detection and flight analysis using two synchronized infrared cameras and a continuously running strobe. The system will:

- The strobe is always running, illuminating the entire area (no per-shot triggering).
- Camera 1 and Camera 2 exposures are staggered for partial overlap (e.g., 25ms/12.5ms, or 10ms/1ms overlap, tunable).
- When either camera delivers an image, it immediately starts a new exposure with no delay.
- Every delivered image is analyzed in real time for circles (balls).
- As soon as any image contains more than one circle, that image is marked as the launch frame.
- The launch frame and the next image delivered from the other camera are used together to calculate ball flight data. (This ensures the full flight is captured, even if it spans two images.)
- The analysis loop is paused while flight data is calculated, then restarted for the next shot.
- Sometimes the entire flight will be visible in a single image; other times, two images may need to be combined for complete flight data.
- Both cameras are hardware-triggered for precise synchronization and overlap.
- Both cameras are infrared, optimized for the strobe wavelength.

### Additional Technical Considerations

- With a 6mm lens and a 50cm field of view, a ball at 80m/s will cross the view in ~6ms. Exposure and strobe timing must be tuned for both the fastest and slowest ball speeds.
- Exposure time can be long in theory, but too many strobe pulses per exposure may cause motion blur or ghosting. The system must balance exposure, strobe frequency, and light intensity.
- Overlap between exposures (e.g., half exposure time) may not be strictly necessary if composite images are used, but this needs to be validated experimentally.
- If circle analysis can be performed in ~8ms, exposure could be as short as 10ms with minimal overlap (e.g., 1ms on each end). These parameters should be experimentally optimized.
- A proof of concept is needed to determine the optimal tradeoff between exposure, strobe pattern, and analysis speed for reliable launch detection and flight capture.

## User Story

**As a** developer or researcher,
**I want** to experiment with a high-speed, strobe-illuminated, dual-infrared camera setup,
**so that** I can reliably detect ball launch events and analyze ball flight without the complexity and fragility of legacy movement-triggered strobe logic.


### Acceptance Criteria (Tightened)
- The system can analyze a single image in ≤10ms (target) and reliably detect 1 vs 2+ circles (balls) with no false positives, even at maximum ball speed (80m/s).
- Camera exposures and strobe timing are tunable and can be experimentally optimized for both fast and slow shots.
- The strobe runs continuously and is not triggered by software or movement detection.
- The system can extract the launch frame and the next frame from the other camera, and combine them if needed, to ensure the full flight is always captured.
- The analysis loop pauses for flight calculation and restarts automatically.
- The infrastructure allows for rapid experimentation and tuning of exposure, overlap, and strobe parameters.
- The system must avoid motion blur or ghosting due to excessive strobe pulses per exposure.
- All timing, overlap, and analysis parameters are configurable and can be validated in a proof-of-concept experiment.

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
