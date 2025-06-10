cmake_minimum_required(VERSION 3.6)

include(GNUInstallDirs)

# Find Boost for property_tree support
find_package(Boost REQUIRED)

# Platform-specific source files
if(WIN32)
    # Windows: Only include basic post-processing stages without libcamera dependencies
    set(SRC post_processing_stage.cpp histogram.cpp)
else()
    # Unix/Linux: Include all stages including libcamera-dependent ones
    set(SRC post_processing_stage.cpp negate_stage.cpp hdr_stage.cpp pwl.cpp histogram.cpp motion_detect_stage.cpp)
endif()

set(TARGET_LIBS images)


if (NOT DEFINED ENABLE_OPENCV)
    set(ENABLE_OPENCV 1)
endif()
set(OpenCV_FOUND 0)
if (ENABLE_OPENCV)
    message(STATUS "Checking for OpenCV")
    find_package(OpenCV QUIET)
else()
    message(STATUS "Omitting check for OpenCV")
endif()

if (OpenCV_FOUND)
    # OpenCV has so many libraries, we're going to link only the ones we need.
    # But if you add more OpenCV stages, you may need more libraries here!
    if(WIN32)
        # Windows: Use the full OpenCV_LIBS from find_package
        set(OpenCV_LIBS_REDUCED ${OpenCV_LIBS})
    else()
        # Unix/Linux: Use manual library specification
        set(OpenCV_LIBS_REDUCED -lopencv_core -lopencv_imgproc -lopencv_objdetect)
    endif()
    
    message(STATUS "OpenCV library found:")
    message(STATUS "    version: ${OpenCV_VERSION}")
    message(STATUS "    libraries: ${OpenCV_LIBS_REDUCED}")
    message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")
    include_directories(${OpenCV_INCLUDE_DIRS})
    
    # Only include OpenCV stages on Unix/Linux due to libcamera dependencies
    if(NOT WIN32)
        set(SRC ${SRC} sobel_cv_stage.cpp face_detect_cv_stage.cpp annotate_cv_stage.cpp plot_pose_cv_stage.cpp object_detect_draw_cv_stage.cpp)
        message(STATUS "OpenCV stages with libcamera support included")
    else()
        message(STATUS "OpenCV stages omitted on Windows due to libcamera dependencies")
    endif()
    
    set(TARGET_LIBS ${TARGET_LIBS} ${OpenCV_LIBS_REDUCED})
    message(STATUS "OpenCV support is included")
else()
    if (ENABLE_OPENCV)
        message(WARNING "OpenCV support was enabled but no libraries found!")
    else()
        message(STATUS "OpenCV support not being included")
    endif()
endif()

if (NOT DEFINED ENABLE_TFLITE)
    set(ENABLE_TFLITE 0)
endif()
if (ENABLE_TFLITE)
    set(SRC ${SRC} tf_stage.cpp object_classify_tf_stage.cpp pose_estimation_tf_stage.cpp object_detect_tf_stage.cpp segmentation_tf_stage.cpp)
    set(TARGET_LIBS ${TARGET_LIBS} tensorflow-lite)
    message(STATUS "Adding TFLite support")
else()
    message(STATUS "TFLite support not being included")
endif()

add_library(post_processing_stages ${SRC})
set_target_properties(post_processing_stages PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

# Link Boost and other target libraries
target_link_libraries(post_processing_stages ${TARGET_LIBS})

# Add include directories for finding core headers and Boost
target_include_directories(post_processing_stages PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${Boost_INCLUDE_DIRS}
)

target_compile_definitions(post_processing_stages PUBLIC OPENCV_PRESENT=${OpenCV_FOUND})

install(TARGETS post_processing_stages LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

list(APPEND ${PROJECT_NAME}_HEADERS
    histogram.hpp
    object_detect.hpp
    post_processing_stage.hpp
    pwl.hpp
    segmentation.hpp
    tf_stage.hpp
)

install(FILES
    ${${PROJECT_NAME}_HEADERS}
    DESTINATION
    ${INCLUDE_INSTALL_DIR}/${PROJECT_NAME}/post_processing_stages
    COMPONENT Devel
)
