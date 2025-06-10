/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Platform-specific libcamera implementation selector
 * 
 * This header automatically includes the appropriate platform-specific
 * implementation based on the compilation target.
 */

#pragma once

// Include the domain interface first
#include "libcamera_interface.hpp"

// Then include the platform-specific implementation
#ifdef __unix__
    #include "libcamera_unix_impl.hpp"
#else
    #include "libcamera_windows_impl.hpp"
#endif
