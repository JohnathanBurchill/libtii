# TIICT Processor: util/anomaly_stats_parallel/CMakeLists.txt

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

project(TIIM)

CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

SET(THREADS_PREFER_PTHREAD_FLAG ON)
FIND_PACKAGE(Threads REQUIRED)
FIND_LIBRARY(CURSES ncurses)
ADD_EXECUTABLE(anomaly_stats_parallel main.c)
TARGET_LINK_LIBRARIES(anomaly_stats_parallel PRIVATE Threads::Threads ${CURSES})

install(TARGETS anomaly_stats_parallel DESTINATION $ENV{HOME}/bin)
