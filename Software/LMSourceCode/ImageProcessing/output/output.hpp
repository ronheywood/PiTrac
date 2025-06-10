/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (C) 2020, Raspberry Pi (Trading) Ltd.
 *
 * output.hpp - video stream output base class
 */

#pragma once

#include <cstdio>
#include <atomic>
#include <queue>
#include <fstream>
#include <map>
#include <memory>
#include <vector>

#include "core/video_options.hpp"

#ifdef __unix__
#include <libcamera/controls.h>
// Forward declaration handled by included headers
#else
// Windows-compatible alternatives for libcamera types
namespace libcamera {
    struct ControlId {
        std::string name() const { return "unknown"; }
    };
    
    struct ControlValue {
        std::string toString() const { return "0"; }
    };
    
    using ControlIdMap = std::map<unsigned int, std::shared_ptr<ControlId>>;
    
    struct ControlList {
        // Dummy implementation for Windows compatibility
        const ControlIdMap* idMap() const { 
            static ControlIdMap dummy_map;
            return &dummy_map; 
        }
        
        // Iterator support for range-based for loops
        using value_type = std::pair<unsigned int, ControlValue>;
        using iterator = std::vector<value_type>::iterator;
        using const_iterator = std::vector<value_type>::const_iterator;
        
        const_iterator begin() const { 
            static std::vector<value_type> dummy_data;
            return dummy_data.begin(); 
        }
        const_iterator end() const { 
            static std::vector<value_type> dummy_data;
            return dummy_data.end(); 
        }
    };
}
#endif

class Output
{
public:
	static Output *Create(VideoOptions const *options);

	Output(VideoOptions const *options);
	virtual ~Output();
	virtual void Signal(); // a derived class might redefine what this means
	void OutputReady(void *mem, size_t size, int64_t timestamp_us, bool keyframe);
	void MetadataReady(libcamera::ControlList &metadata);

protected:
	enum Flag
	{
		FLAG_NONE = 0,
		FLAG_KEYFRAME = 1,
		FLAG_RESTART = 2
	};
	virtual void outputBuffer(void *mem, size_t size, int64_t timestamp_us, uint32_t flags);
	virtual void timestampReady(int64_t timestamp);
	VideoOptions const *options_;
	FILE *fp_timestamps_;

private:
	enum State
	{
		DISABLED = 0,
		WAITING_KEYFRAME = 1,
		RUNNING = 2
	};
	State state_;
	std::atomic<bool> enable_;
	int64_t time_offset_;
	int64_t last_timestamp_;
	std::streambuf *buf_metadata_;
	std::ofstream of_metadata_;
	bool metadata_started_ = false;
	std::queue<libcamera::ControlList> metadata_queue_;
};

void start_metadata_output(std::streambuf *buf, std::string fmt);
void write_metadata(std::streambuf *buf, std::string fmt, libcamera::ControlList &metadata, bool first_write);
void stop_metadata_output(std::streambuf *buf, std::string fmt);
