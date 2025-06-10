/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Unix/Linux infrastructure implementation for camera operations
 * 
 * This file provides the actual libcamera implementations for Unix/Linux platforms.
 * It bridges between our domain interface and the real libcamera library.
 * 
 * Step 3 of incremental refactoring: Unix infrastructure layer.
 * This allows Unix builds to continue using real libcamera while we prepare 
 * for eventual migration to domain interface.
 */

#pragma once

#ifdef __unix__

// Include actual libcamera headers for Unix/Linux
#include <libcamera/camera.h>
#include <libcamera/controls.h>
#include <libcamera/request.h>
#include <libcamera/color_space.h>
#include <libcamera/pixel_format.h>
#include <libcamera/transform.h>

// Include our domain interface
#include "../../domain/camera_domain.hpp"

namespace libcamera_infrastructure {
    
    // Unix implementation uses actual libcamera types directly
    // This namespace provides the bridge between domain and infrastructure
    
    // For now, we simply alias the real types
    // Later, we can add conversion functions if needed
    using CameraSize = libcamera::Size;
    using CameraTransform = libcamera::Transform;
    using CameraPixelFormat = libcamera::PixelFormat;
    using CameraColorSpace = libcamera::ColorSpace;
    using CameraControlList = libcamera::ControlList;
    using CameraRequest = libcamera::Request;
    
} // namespace libcamera_infrastructure

#endif // __unix__
