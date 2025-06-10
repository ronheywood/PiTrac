/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Domain interface for libcamera abstractions
 * 
 * This file defines the abstract interface for camera operations,
 * independent of the underlying platform implementation.
 */

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <optional>

namespace libcamera {
    // Forward declarations for the domain interface
    // Actual implementations are provided by platform-specific files

    // Basic types
    struct Size {
        unsigned int width, height;
        Size(unsigned int w = 0, unsigned int h = 0) : width(w), height(h) {}
    };

    struct Transform {
        int value = 0;
        Transform(int v = 0) : value(v) {}
    };

    struct PixelFormat {
        unsigned int fourcc = 0;
        PixelFormat(unsigned int f = 0) : fourcc(f) {}
    };

    struct ColorSpace {
        int value = 0;
        ColorSpace(int v = 0) : value(v) {}
    };    // Control system types
    struct ControlId {
        std::string name() const { return "stub_control"; }
    };

    struct ControlValue {
        std::string toString() const { return "0"; }
    };

    using ControlIdMap = std::map<unsigned int, std::shared_ptr<ControlId>>;

    struct ControlList {
        // Iterator support for range-based for loops
        using value_type = std::pair<unsigned int, ControlValue>;
        using iterator = std::vector<value_type>::iterator;
        using const_iterator = std::vector<value_type>::const_iterator;
        
        const ControlIdMap* idMap() const { 
            static ControlIdMap dummy_map;
            return &dummy_map; 
        }
        
        const_iterator begin() const { 
            static std::vector<value_type> dummy_data;
            return dummy_data.begin(); 
        }
        
        const_iterator end() const { 
            static std::vector<value_type> dummy_data;
            return dummy_data.end(); 
        }
    };

    // Request system types
    struct Request {
        using BufferMap = std::map<void*, void*>;
        
        BufferMap buffers() { 
            static BufferMap dummy_buffers;
            return dummy_buffers; 
        }
        
        ControlList metadata() { 
            static ControlList dummy_metadata;
            return dummy_metadata; 
        }
        
        void reuse() { /* no-op */ }
    };

    // Utility types
    template<typename T>
    struct Span {
        T* data_;
        size_t size_;
        
        Span(T* data = nullptr, size_t size = 0) : data_(data), size_(size) {}
        T* data() const { return data_; }
        size_t size() const { return size_; }
        bool empty() const { return size_ == 0; }
    };

} // namespace libcamera
