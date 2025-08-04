# TIILIB: tools/tiim_stats/CMakeLists.txt

# Copyright (C) 2024  Johnathan K Burchill

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

PROJECT(tiim_stats)

SET(THREADS_PREFER_PTHREAD_FLAG ON)
FIND_PACKAGE(Threads REQUIRED)
FIND_LIBRARY(CURSES ncurses)

# SET(LIBS ${LIBS} ${MATH} ${LIBXML2_LIBRARIES} ${ZLIB_LIBRARIES})
INCLUDE_DIRECTORIES(${INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIR})

ADD_EXECUTABLE(tiim_stats tiim_stats.c)
TARGET_LINK_LIBRARIES(tiim_stats ${LIBS} tii -lm)

ADD_EXECUTABLE(tiim_stats_parallel tiim_stats_parallel.c)
TARGET_LINK_LIBRARIES(tiim_stats_parallel PRIVATE ${LIBS} tii -lm Threads::Threads ${CURSES})

install(TARGETS tiim_stats DESTINATION $ENV{HOME}/bin)
install(TARGETS tiim_stats_parallel DESTINATION $ENV{HOME}/bin)

