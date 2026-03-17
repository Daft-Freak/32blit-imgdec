cmake_minimum_required(VERSION 3.15)
project(32blit-imgdec)

if(NOT TARGET BlitEngine)
    # find 32blit SDK if not already available
    # (mostly so I can build-test the lib)
    find_package(32BLIT CONFIG REQUIRED)
endif()

add_library(32blit-imgdec STATIC
    JPEGDEC/src/JPEGDEC.cpp

    PNGdec/src/adler32.c
    PNGdec/src/crc32.c
    PNGdec/src/inffast.c
    PNGdec/src/inflate.c
    PNGdec/src/inftrees.c
    PNGdec/src/PNGdec.cpp
    PNGdec/src/zutil.c

    imgdec.cpp
    jpeg.cpp
    png.cpp
)

target_include_directories(32blit-imgdec PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/not-duino
    ${CMAKE_CURRENT_LIST_DIR}/JPEGDEC/src
    ${CMAKE_CURRENT_LIST_DIR}/PNGdec/src
)
target_include_directories(32blit-imgdec PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(32blit-imgdec PRIVATE BlitEngine)

# add example if top-level (not included as a library)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    message("Top-level build, adding example")
    add_subdirectory(example)
endif()