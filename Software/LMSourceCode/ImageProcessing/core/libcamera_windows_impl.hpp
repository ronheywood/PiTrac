/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Windows implementation of libcamera abstractions
 * 
 * This file provides stub implementations of libcamera types for Windows platforms
 * where libcamera is not available. These implementations provide the minimal
 * functionality needed for compilation and basic operation.
 */

#pragma once

#ifndef __unix__

#include "libcamera_interface.hpp"

namespace libcamera {
    // Windows stub implementations are provided by the base interface
    // No additional implementation needed since the interface provides default behavior
}// namespace libcamera

#endif // !__unix__
