# LibCamera Platform Abstraction Layer

## Overview

This document describes the modular platform abstraction layer for libcamera types, which consolidates previously duplicated stub implementations into a clean domain/infrastructure separation.

## Architecture

### Domain Layer (Interface)
- **File**: `core/libcamera_interface.hpp`
- **Purpose**: Defines pure domain interface with concrete implementations
- **Types**: Size, Transform, PixelFormat, ColorSpace, ControlId, ControlValue, ControlList, Request, Span

### Infrastructure Layer (Platform Implementations)

#### Unix/Linux Implementation
- **File**: `core/libcamera_unix_impl.hpp`
- **Purpose**: Includes actual libcamera headers for Unix/Linux platforms
- **Behavior**: Uses real libcamera types directly

#### Windows Implementation  
- **File**: `core/libcamera_windows_impl.hpp`
- **Purpose**: Provides stub behavior for Windows platforms
- **Behavior**: Uses default implementations from the interface

### Platform Selector
- **File**: `core/libcamera_platform.hpp`
- **Purpose**: Automatically includes the appropriate platform implementation
- **Usage**: Include this file instead of platform-specific headers

## Usage

Replace old platform-specific includes:

```cpp
// OLD - Duplicated in multiple files
#ifdef __unix__
#include <libcamera/controls.h>
#else
namespace libcamera {
    struct ControlList { /* stub */ };
}
#endif
```

With the new consolidated module:

```cpp
// NEW - Single include handles all platforms
#include "core/libcamera_platform.hpp"
```

## Benefits

1. **DRY Principle**: Eliminates code duplication across multiple files
2. **Maintainability**: Single point of truth for platform abstractions
3. **Modularity**: Clean separation between domain and infrastructure concerns
4. **Extensibility**: Easy to add new platforms or modify existing ones
5. **Type Safety**: Consistent interfaces across all platforms

## Files Updated

The following files were updated to use the new modular system:

- `core/options.hpp`
- `core/completed_request.hpp` 
- `core/stream_info.hpp`
- `output/output.hpp`
- `preview/preview.hpp`

## Legacy Support

The `core/windows_stub.cpp` file remains for CMake compatibility but is marked as deprecated. All libcamera platform compatibility is now handled through the header-only module system.

## Testing

The new architecture has been tested and verified to:
- Build successfully on Windows
- Maintain executable functionality
- Produce no compilation errors
- Preserve existing behavior

## Future Enhancements

- Consider adding platform-specific optimizations
- Extend to support additional platforms if needed
- Migrate remaining libcamera usages to use the centralized module
