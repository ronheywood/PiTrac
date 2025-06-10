/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Windows compatibility stub for core library
 * 
 * NOTE: This file is deprecated. libcamera platform compatibility
 * is now handled through the modular libcamera_platform.hpp system.
 * This file remains only to satisfy CMake's requirement for at least
 * one source file in the core library.
 */

// This file provides a minimal implementation for Windows builds
// where libcamera functionality is not available

// Empty implementation - all functionality provided via headers
namespace {
    // Placeholder to ensure the library has at least one symbol
    void deprecated_windows_stub() {
        // This function exists solely to provide a symbol for the library
        // Platform compatibility is now handled via libcamera_platform.hpp
    }
}
