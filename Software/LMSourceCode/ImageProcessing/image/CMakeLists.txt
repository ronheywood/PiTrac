cmake_minimum_required(VERSION 3.6)

include(GNUInstallDirs)

if(NOT WIN32)
    # Unix-specific image libraries
    find_library(EXIF_LIBRARY exif REQUIRED)
    find_library(JPEG_LIBRARY jpeg REQUIRED)
    find_library(TIFF_LIBRARY tiff REQUIRED)
    find_library(PNG_LIBRARY png REQUIRED)
    set(IMAGE_LIBS jpeg exif png tiff)
    set(IMAGE_SOURCES bmp.cpp yuv.cpp jpeg.cpp png.cpp dng.cpp)
else()
    # Windows: These libraries are typically part of the build or not needed
    # OpenCV handles most image formats on Windows
    message(STATUS "Using OpenCV for image processing on Windows")
    set(IMAGE_LIBS "")
    # Use a minimal stub file for Windows
    set(IMAGE_SOURCES image_stub.cpp)
endif()

add_library(images ${IMAGE_SOURCES})
set_target_properties(images PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})
target_link_libraries(images ${IMAGE_LIBS})

install(TARGETS images LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

list(APPEND ${PROJECT_NAME}_HEADERS
    image.hpp
)

install(FILES
    ${${PROJECT_NAME}_HEADERS}
    DESTINATION
    ${INCLUDE_INSTALL_DIR}/${PROJECT_NAME}/image
    COMPONENT Devel
)

