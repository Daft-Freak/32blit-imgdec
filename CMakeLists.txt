project(32blit-imgdec)

add_library(32blit-imgdec STATIC
    JPEGDEC/src/JPEGDEC.cpp

    jpeg.cpp
)

target_include_directories(32blit-imgdec PRIVATE ${CMAKE_CURRENT_LIST_DIR}/not-duino ${CMAKE_CURRENT_LIST_DIR}/JPEGDEC/src)
target_include_directories(32blit-imgdec PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(32blit-imgdec PRIVATE BlitEngine)
