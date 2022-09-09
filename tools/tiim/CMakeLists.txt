# TIILIB: tools/tiim/CMakeLists.txt

# Copyright (C) 2022  Johnathan K Burchill

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

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

