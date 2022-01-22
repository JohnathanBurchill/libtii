# CMakeLists.txt file for processor

CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT(tii_tools)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")

if(CMAKE_BUILD_TYPE STREQUAL Debug)
    message( "-- Debugging enabled")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -std=gnu99 ")
else (CMAKE_BUILD_TYPE STREQUAL Debug)
    message( "-- Optimization enabled")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -std=gnu99 ")
endif(CMAKE_BUILD_TYPE STREQUAL Debug)


# requires -lm on linux
FIND_LIBRARY(MATH m)
FIND_PACKAGE(ZLIB REQUIRED)
FIND_PACKAGE(LibXml2)

INCLUDE_DIRECTORIES(include)

ADD_SUBDIRECTORY(lib)
ADD_SUBDIRECTORY(libtiigraphics)

ADD_SUBDIRECTORY(tiim)
ADD_SUBDIRECTORY(tiim_stats)
add_SUBDIRECTORY(get_image_stats)

