# TIILIB: tools/tii_dump/CMakeLists.txt

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

PROJECT(tii_dump)

# SET(LIBS ${LIBS} ${MATH} ${LIBXML2_LIBRARIES} ${ZLIB_LIBRARIES})
INCLUDE_DIRECTORIES(${INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIR})

ADD_EXECUTABLE(tii_dump tii_dump.c)
TARGET_LINK_LIBRARIES(tii_dump ${LIBS} tii -lm)

install(TARGETS tii_dump DESTINATION $ENV{HOME}/bin)

install(PROGRAMS parallel_process_tii_dump.sh DESTINATION $ENV{HOME}/bin)
install(PROGRAMS process_tii_dump.sh DESTINATION $ENV{HOME}/bin)

