# Migration Plan: Pulse Strobe and Camera Configuration to Infrastructure Layer

## Goals
- Move all pulse strobe logic and camera hardware configuration into the Camera bounded context's infrastructure layer
- Decouple domain/application logic from hardware details
- Enable robust, automated testing (including in CI/CD)
- Support headless, hardware-free testability for pulse and serial bus logic

---

## Migration Steps

### 1. Identify and Isolate Hardware Logic
- Locate all code related to strobe pulse generation, GPIO, SPI/serial bus, and camera hardware configuration (e.g., in `pulse_strobe.cpp`, `gs_camera.cpp`)
- Extract these into new infrastructure classes/interfaces under `golf_sim::camera::infrastructure`
- Define clear boundaries: domain/application layers interact only with abstract interfaces

### 2. Define Infrastructure Interfaces
- `IStrobeController` — Abstracts strobe pulse pattern, timing, and triggering
- `ISerialBusAdapter` — Abstracts SPI/GPIO/serial bus communication
- `ICameraHardwareConfig` — Abstracts camera hardware settings (resolution, exposure, trigger mode)

### 3. Implement Concrete Infrastructure Classes
- `PulseStrobeController` — Implements `IStrobeController` using real hardware (GPIO/SPI)
- `SerialBusAdapter` — Implements `ISerialBusAdapter` for actual bus communication
- `CameraHardwareConfig` — Implements `ICameraHardwareConfig` for real camera setup

### 4. Provide Mock/Fake Implementations for Testing
- `MockStrobeController` — Simulates pulse patterns, records calls, and exposes state for assertions
- `MockSerialBusAdapter` — Simulates bus communication, can inject errors, and verify protocol
- `MockCameraHardwareConfig` — Simulates camera config, allows test-time overrides


### 5. Refactor Legacy Domain/Application to Use Interfaces

**a. Identify all direct hardware and strobe logic in domain/application code:**
   - Search for any direct calls to GPIO, SPI, or strobe-related functions (e.g., `PulseStrobe::SendCameraStrobeTriggerAndShutter`, `lgpio_*`, `OpenSpi`, etc.) in domain and application layers.
   - Locate any camera configuration logic (resolution, exposure, trigger mode) that is not already abstracted.

**b. Replace direct calls with interface usage:**
   - Refactor code to call `IStrobeController`, `ISerialBusAdapter`, and `ICameraHardwareConfig` interfaces instead of concrete or legacy classes.
   - For example, replace `PulseStrobe::SendCameraStrobeTriggerAndShutter(...)` with `strobeController->TriggerStrobe(...)`.
   - Move any protocol, timing, or pattern logic out of the domain/application and into the infrastructure implementation.

**c. Inject dependencies:**
   - Use dependency injection (constructor or setter) to provide the required interface implementations to domain/application services.
   - In production, inject real hardware adapters; in tests, inject mocks/fakes.

**d. Remove legacy coupling:**
   - Eliminate all `#ifdef __unix__` and platform-specific code from domain/application layers.
   - Remove any direct hardware includes (e.g., `<lgpio.h>`, `<windows.h>`) from non-infrastructure code.

**e. Update tests:**
   - Refactor existing tests to use mock interfaces for strobe and serial bus logic.
   - Add new tests to verify that domain/application logic interacts correctly with the interfaces (e.g., correct strobe pattern requested, correct camera config applied).

**f. Document all interface boundaries:**
   - Clearly document which classes are infrastructure and which are domain/application.
   - Provide usage examples for both real and mock implementations.

### 6. Automated Testing Strategy
- **Unit Tests:**
  - Test all infrastructure logic using mocks (no hardware required)
  - Verify pulse pattern generation, timing, and edge cases
  - Test serial bus protocol, error handling, and recovery
- **Integration Tests:**
  - Use real hardware adapters on supported platforms (optional)
  - Run in CI with mocks to ensure headless testability
- **CI/CD (GitHub Actions):**
  - All tests must pass in a headless environment (no GPIO/SPI/camera required)
  - Use the `MockStrobeController` and `MockSerialBusAdapter` in CI
  - Validate pulse sequence logic, timing calculations, and protocol compliance
  - Provide test harnesses that simulate strobe/camera events and verify system response

### 7. Example: Headless Pulse/Serial Bus Test
```cpp
TEST_CASE("PulseStrobeController generates correct pulse sequence") {
    MockStrobeController strobe;
    strobe.ConfigurePattern({5, 10, 5}); // ms
    strobe.Start();
    REQUIRE(strobe.GetPulseLog() == std::vector<int>{5, 10, 5});
}

TEST_CASE("SerialBusAdapter transmits and receives data correctly") {
    MockSerialBusAdapter bus;
    bus.Send({0xA5, 0x5A});
    auto received = bus.Receive();
    REQUIRE(received == std::vector<uint8_t>{0xA5, 0x5A});
}
```

### 8. Documentation and Developer Guidance
- Document all interfaces and mock usage
- Provide examples for running tests locally and in CI
- Ensure all new code is covered by automated tests before merging

---

## Summary
This plan ensures that all pulse strobe and camera configuration logic is cleanly separated into the infrastructure layer, with robust automated testing (including in CI/CD) and full support for headless, hardware-free test runs. This enables rapid, safe experimentation and future hardware changes with minimal risk.
