project(32blit-imgdec)

add_library(32blit-imgdec STATIC jpeg.cpp)

target_include_directories(32blit-imgdec PUBLIC ${CMAKE_CURRENT_LIST_DIR})
