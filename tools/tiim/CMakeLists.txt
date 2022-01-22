CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT(tiim)

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

ADD_EXECUTABLE(tiim tiim.c)
target_include_directories(tiim PRIVATE include)
TARGET_LINK_LIBRARIES(tiim ${LIBS} tii tiigraphics)

install(TARGETS tiim DESTINATION $ENV{HOME}/bin)

install(PROGRAMS parallel_process_TIIM.sh DESTINATION $ENV{HOME}/bin)
install(PROGRAMS process_TIIM.sh DESTINATION $ENV{HOME}/bin)

