/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Unix/Linux implementation of libcamera abstractions
 * 
 * This file provides the actual libcamera implementations for Unix/Linux platforms.
 * It includes the real libcamera headers and implements the interface.
 */

#pragma once

#ifdef __unix__

#include "libcamera_interface.hpp"
#include <libcamera/controls.h>
#include <libcamera/request.h>
#include <libcamera/color_space.h>
#include <libcamera/pixel_format.h>

// The Unix implementation uses the actual libcamera types directly
// No additional wrapper implementations needed since libcamera provides everything

#endif // __unix__
