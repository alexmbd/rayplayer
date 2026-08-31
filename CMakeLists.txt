cmake_minimum_required(VERSION 3.23)
project(rayplayer VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/cmake/dist)

# freetype
set(FT_DISABLE_ZLIB ON CACHE INTERNAL "")
set(FT_DISABLE_BZIP2 ON CACHE INTERNAL "")
set(FT_DISABLE_PNG ON CACHE INTERNAL "")
set(FT_DISABLE_HARFBUZZ ON CACHE INTERNAL "")
set(FT_DISABLE_BROTLI ON CACHE INTERNAL "")

add_subdirectory(external/freetype)
add_library(Freetype::Freetype ALIAS freetype)

# mpv
find_library(MPV_LIB mpv.dll HINTS "${PROJECT_SOURCE_DIR}/external/mpv")

# raylib
add_subdirectory(external/raylib)

# RmlUi
set(FREETYPE_INCLUDE_DIRS "external/freetype/include/freetype" CACHE STRING "")
set(FREETYPE_LIBRARY "Freetype::Freetype" CACHE STRING "")
add_subdirectory(external/RmlUi)

# rayplayer
set(RAYPLAYER_FILES
    ${PROJECT_SOURCE_DIR}/src/main.cpp
)

add_executable(${CMAKE_PROJECT_NAME} ${RAYPLAYER_FILES})

if(MINGW)
    target_link_options(${CMAKE_PROJECT_NAME} PRIVATE "-static" "-static-libgcc" "-static-libstdc++" "-Wl,-Bstatic" "-lwinpthread")    
endif()

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE stdc++exp raylib RmlUi::Core ${MPV_LIB})
target_include_directories(${CMAKE_PROJECT_NAME}
    PRIVATE 
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/external/mpv/include
    ${PROJECT_SOURCE_DIR}/external/raylib/src
    ${PROJECT_SOURCE_DIR}/external/RmlUi/Include
)