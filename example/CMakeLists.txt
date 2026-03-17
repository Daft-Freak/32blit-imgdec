# Basic parameters; check that these match your project / environment
cmake_minimum_required(VERSION 3.16)

project(imgdec-example)

set(PROJECT_SOURCE example.cpp)

# ... and any other files you want in the release here
# set(PROJECT_DISTRIBS LICENSE README.md)

# Build configuration; approach this with caution!
if(MSVC)
  add_compile_options("/W4" "/wd4244" "/wd4324" "/wd4458" "/wd4100")
else()
  add_compile_options("-Wall" "-Wextra" "-Wdouble-promotion" "-Wno-unused-parameter")
endif()

# we have everything included by the top-level CMakeLists
# find_package (32BLIT CONFIG REQUIRED PATHS ../32blit-sdk $ENV{PATH_32BLIT_SDK})
# find_package (32blit-imgdec CONFIG REQUIRED PATHS ../32blit-imgdec)

blit_executable (${PROJECT_NAME} ${PROJECT_SOURCE})
target_link_libraries (${PROJECT_NAME} 32blit-imgdec)

blit_assets_yaml (${PROJECT_NAME} assets.yml)
blit_metadata (${PROJECT_NAME} metadata.yml)
add_custom_target (flash DEPENDS ${PROJECT_NAME}.flash)

# setup release packages
# install (FILES ${PROJECT_DISTRIBS} DESTINATION .)
set (CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set (CPACK_GENERATOR "ZIP" "TGZ")
include (CPack)