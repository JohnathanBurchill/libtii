# CMakeLists.txt file for processor
PROJECT(tiim)

CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")

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

# LIB AVUTIL, from https://newbedev.com/cmake-configuration-for-ffmpeg-in-c-project
FIND_PACKAGE(PkgConfig REQUIRED)
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    libavformat
    libavcodec
    libswresample
    libswscale
    libavutil
)

SET(LIBS ${LIBS} ${MATH} ${LIBXML2_LIBRARIES} ${ZLIB_LIBRARIES} PkgConfig::LIBAV)

INCLUDE_DIRECTORIES(${INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIR})

ADD_EXECUTABLE(tiim tiim.c isp.c xml.c fonts.c analysis.c draw.c import.c utility.c filters.c gainmap.c video.c timeseries.c)

TARGET_LINK_LIBRARIES(tiim ${LIBS})


ADD_EXECUTABLE(tiim_stats tiim_stats.c isp.c xml.c analysis.c import.c utility.c filters.c timeseries.c)
TARGET_LINK_LIBRARIES(tiim_stats ${LIBS})

ADD_EXECUTABLE(get_image_stats get_image_stats.c)
TARGET_LINK_LIBRARIES(tiim_stats -lm)

install(TARGETS tiim DESTINATION $ENV{HOME}/bin)
install(TARGETS tiim_stats DESTINATION $ENV{HOME}/bin)
install(TARGETS get_image_stats DESTINATION $ENV{HOME}/bin)

install(PROGRAMS parallel_process_TIIM.sh DESTINATION $ENV{HOME}/bin)
install(PROGRAMS process_TIIM.sh DESTINATION $ENV{HOME}/bin)
install(PROGRAMS parallel_process_TIIM_stats.sh DESTINATION $ENV{HOME}/bin)
install(PROGRAMS process_TIIM_stats.sh DESTINATION $ENV{HOME}/bin)

