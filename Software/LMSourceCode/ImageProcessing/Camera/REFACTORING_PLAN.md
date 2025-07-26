# Camera Bounded Context - Refactoring Plan

## 📋 Clean Code & Architecture Review Results

Based on comprehensive code review, the Camera bounded context has several areas that need improvement to meet Clean Code standards and proper domain-driven design principles.

---

## 🚨 CRITICAL ISSUES - HIGH PRIORITY

### 1. Business Logic in Tests
```markdown
### 🔴 CRITICAL: Remove Business Logic from Tests

#### ❌ Current Issues:
- **File capture and processing in tests** - Image saving, format conversion
- **UI interactions in tests** - Opening image viewers, system commands  
- **Complex testing workflows** - Multi-step processes that should be in application layer
- **Platform-specific operations** - Windows-specific file operations in tests

#### ✅ Required Actions:
- [ ] **Extract image processing to application services** - Move JPEG conversion out of tests
- [ ] **Remove UI interactions from tests** - No `system()` calls in tests
- [ ] **Create test doubles for complex operations** - Mock file I/O and external processes
- [ ] **Use pure assertions only** - Tests should only verify behavior, not perform business operations
```

### 2. Domain Purity Violations
```markdown
### 🔴 CRITICAL: Clean Domain Layer

#### ❌ Current Issues:
- **Hardcoded resolutions in infrastructure** - `GetSupportedResolutions()` returns fixed list
- **Missing domain validation** - No validation of camera configurations
- **Anemic domain model** - Domain types lack behavior and validation
- **Platform coupling in domain** - Windows-specific concepts bleeding through

#### ✅ Required Actions:
- [ ] **Add domain validation logic** - Validate `CameraConfig` parameters
- [ ] **Remove hardcoded values** - Make resolution discovery dynamic
- [ ] **Add domain behaviors** - Validation methods on domain types
- [ ] **Implement domain invariants** - Ensure valid state transitions
```

### 3. Exception Handling Inconsistencies
```markdown
### 🔴 CRITICAL: Consistent Exception Strategy

#### ❌ Current Issues:
- **Mixed return/exception patterns** - Some methods return bool, others throw
- **Infrastructure returns booleans instead of throwing** - Should throw domain exceptions
- **No structured error information** - Lost context in boolean returns
- **Missing specific exception types** - Need more granular exception hierarchy

#### ✅ Required Actions:
- [ ] **Standardize on exceptions** - Infrastructure should throw, not return false
- [ ] **Add detailed exception context** - Include HRESULT codes, device names, etc.
- [ ] **Complete exception hierarchy** - Add missing exception types
- [ ] **Implement proper exception translation** - Infrastructure to domain exceptions
```

---

## 🛠️ INFRASTRUCTURE ISSUES - MEDIUM PRIORITY

### 4. Windows Implementation Issues
```markdown
### 🟡 Infrastructure Refactoring Needed

#### ❌ Current Issues:
- **Inconsistent error handling** - Mix of HRESULT checking and boolean returns
- **Resource management issues** - Manual cleanup instead of RAII
- **Complex methods** - `ConfigureSourceReader()` too long and complex
- **Magic numbers and constants** - Hardcoded format GUIDs and parameters

#### ✅ Required Actions:
- [ ] **Extract Windows-specific error handling** - Create HRESULT wrapper class
- [ ] **Implement RAII (Resource acquisition is initialization) for Media Foundation** - Automatic cleanup on scope exit 
- [ ] **Break down complex methods** - Split `ConfigureSourceReader()` into smaller functions
- [ ] **Create named constants** - Replace magic numbers with semantic names
- [ ] **Add proper logging** - Replace `std::cerr` with structured logging
```

### 5. Discovery Service Issues
```markdown
### 🟡 Discovery Service Refactoring

#### ❌ Current Issues:
- **Inconsistent accessibility testing** - Three different levels of testing needed
- **Caching without invalidation strategy** - Manual refresh only
- **Hardcoded device selection** - Always selects first camera
- **Missing error context** - No information about why devices are inaccessible

#### ✅ Required Actions:
- [ ] **Standardize accessibility testing** - Single, reliable test method
- [ ] **Implement intelligent caching** - Automatic cache invalidation
- [ ] **Add device selection strategy** - Allow selection of specific cameras
- [ ] **Enhance error reporting** - Detailed accessibility failure reasons
```

---

## 🧪 TEST IMPROVEMENTS - MEDIUM PRIORITY

### 6. Test Structure and Organization
```markdown
### 🟡 Test Architecture Improvements

#### ❌ Current Issues:
- **Giant test methods** - Some tests over 100 lines
- **Mixed concerns in single tests** - Testing multiple unrelated features
- **Poor test naming** - Generic names like "capture_single_frame_for_review"
- **Lack of test categories** - No separation of unit vs integration tests

#### ✅ Required Actions:
- [ ] **Split large test methods** - One assertion per test method
- [ ] **Implement test categories** - Unit, Integration, System tests
- [ ] **Improve test naming** - Should/When/Given naming pattern
- [ ] **Create focused test fixtures** - Specific setup for each test concern
```

### 7. Test Doubles and Mocking
```markdown
### 🟡 Test Isolation Improvements

#### ❌ Current Issues:
- **No mocking framework** - Testing against real hardware
- [ ] **Integration tests only** - No true unit tests for domain logic
- **Hardware dependencies in CI** - Tests require cameras to pass
- **No test data builders** - Hardcoded test data throughout

#### ✅ Required Actions:
- [ ] **Introduce mocking framework** - Mock Windows Media Foundation APIs
- [ ] **Create test doubles** - Fake camera implementations for unit tests
- [ ] **Implement test data builders** - Fluent builders for test scenarios
- [ ] **Add parameterized tests** - Test multiple scenarios efficiently
```

---

## 🏗️ ARCHITECTURE IMPROVEMENTS - LOW PRIORITY

### 8. Domain Model Enhancement
```markdown
### 🔵 Domain Model Maturity

#### ✅ Future Enhancements:
- [ ] **Add value objects** - Immutable types for resolution, format, etc.
- [ ] **Implement aggregate patterns** - Camera as aggregate root
- [ ] **Add domain events** - Camera state change notifications
- [ ] **Create specification pattern** - Complex camera capability queries
```

### 9. API Design Improvements
```markdown
### 🔵 Interface Design Polish

#### ✅ Future Enhancements:
- [ ] **Add builder patterns** - Fluent configuration builders
- [ ] **Implement adapter patterns** - Multiple camera backend support
- [ ] **Add async interfaces** - Non-blocking camera operations
- [ ] **Create fluent APIs** - Method chaining for common operations
```

### 10. Cross-Platform Support
```markdown
### 🔵 Platform Abstraction

#### ✅ Future Work:
- [ ] **Implement Unix camera support** - Linux/macOS implementations
- [ ] **Add capability negotiation** - Platform-specific feature detection
- [ ] **Create platform test suites** - Platform-specific test scenarios
- [ ] **Add platform abstraction layer** - Hide platform differences from domain
```

---

## 📝 IMPLEMENTATION STRATEGY

### Phase 1: Foundation (Weeks 1-2)
```markdown
### 🎯 Core Infrastructure Cleanup
- [ ] **Fix exception handling** - Consistent exception throwing
- [ ] **Remove business logic from tests** - Extract to application services
- [ ] **Clean domain model** - Add validation and behavior
- [ ] **Implement proper resource management** - RAII patterns
```

### Phase 2: Test Improvements (Weeks 3-4)
```markdown
### 🎯 Test Architecture Overhaul
- [ ] **Introduce mocking framework** - Isolate unit tests
- [ ] **Restructure test methods** - One assertion per test
- [ ] **Create test doubles** - Mock implementations
- [ ] **Add test data builders** - Fluent test setup
```

### Phase 3: Architecture Enhancement (Weeks 5-6)
```markdown
### 🎯 Advanced Architecture Patterns
- [ ] **Implement value objects** - Immutable domain types
- [ ] **Add domain events** - State change notifications
- [ ] **Create fluent APIs** - Improved usability
- [ ] **Add async support** - Non-blocking operations
```

### Phase 4: Platform Support (Future)
```markdown
### 🎯 Cross-Platform Implementation
- [ ] **Unix camera support** - Linux/macOS implementations
- [ ] **Capability negotiation** - Platform feature detection
- [ ] **Platform test suites** - Comprehensive platform testing
```

---

## 🔧 REFACTORING TODOS BY FILE

### Domain Layer
```markdown
#### camera_domain.hpp
- [ ] **Add validation methods** - `CameraConfig::IsValid()`
- [ ] **Implement value object semantics** - Immutable types
- [ ] **Add domain invariants** - State validation rules
- [ ] **Create builder patterns** - Fluent configuration building

#### camera_exceptions.hpp
- [ ] **Add error context fields** - HRESULT, device info, etc.
- [ ] **Implement exception chaining** - Preserve inner exceptions
- [ ] **Add structured error codes** - Enumerated error categories
- [ ] **Create exception factories** - Consistent exception creation
```

### Infrastructure Layer
```markdown
#### windows_camera.cpp
- [ ] **Extract HRESULT error handling** - Create HResultWrapper class
- [ ] **Break down ConfigureSourceReader()** - Split into smaller methods
- [ ] **Implement RAII for MF resources** - Automatic cleanup
- [ ] **Replace hardcoded formats** - Named constants and discovery
- [ ] **Add structured logging** - Replace std::cerr with proper logging
- [ ] **Throw domain exceptions** - Instead of returning false

#### windows_camera_discovery.cpp
- [ ] **Standardize accessibility testing** - Single reliable method
- [ ] **Add intelligent caching** - Cache invalidation strategy
- [ ] **Enhance error reporting** - Detailed failure context
- [ ] **Implement device selection** - Allow specific camera selection
```

### Test Layer
```markdown
#### test_windows_camera.cpp
- [ ] **Remove image processing logic** - Extract to application services
- [ ] **Eliminate UI interactions** - No system() calls
- [ ] **Split giant test methods** - One concern per test
- [ ] **Add mocking framework** - Mock Media Foundation APIs
- [ ] **Create test doubles** - Fake camera implementations
- [ ] **Implement Should/When/Given naming** - Clear test intent
- [ ] **Add parameterized tests** - Multiple scenario testing
```

---

## ✅ SUCCESS CRITERIA

### Code Quality Metrics
- [ ] **No business logic in tests** - Tests only verify behavior
- [ ] **Consistent exception handling** - All failures throw domain exceptions
- [ ] **Method length < 20 lines** - Small, focused methods
- [ ] **Class cohesion high** - Single responsibility adherence
- [ ] **Dependency direction correct** - Infrastructure depends on domain

### Architecture Compliance
- [ ] **Domain purity maintained** - No infrastructure concerns in domain
- [ ] **Separation of concerns clear** - Each layer has distinct responsibility
- [ ] **SOLID principles followed** - Especially SRP, OCP, DIP
- [ ] **Clean interfaces** - Simple, focused interfaces
- [ ] **Proper abstraction levels** - Appropriate detail hiding

### Test Quality
- [ ] **Fast unit tests** - No hardware dependencies
- [ ] **Isolated test methods** - One assertion per test
- [ ] **Good test coverage** - All domain logic covered
- [ ] **Clear test intent** - Tests document expected behavior
- [ ] **Reliable tests** - No flaky tests due to hardware
