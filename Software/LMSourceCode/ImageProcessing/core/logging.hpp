#pragma once

#include <iostream>

#ifdef __unix__
#include "core/rpicam_app.hpp"

#define LOG(level, text)                                                                                               \
	do                                                                                                                 \
	{                                                                                                                  \
		if (RPiCamApp::GetVerbosity() >= level)                                                                     \
			std::cerr << text << std::endl;                                                                            \
	} while (0)
#else
// Windows-compatible logging without RPiCamApp dependency
#define LOG(level, text)                                                                                               \
	do                                                                                                                 \
	{                                                                                                                  \
		if (level <= 2) /* Default verbosity level for Windows */                                                     \
			std::cerr << text << std::endl;                                                                            \
	} while (0)
#endif

#define LOG_ERROR(text) std::cerr << text << std::endl
