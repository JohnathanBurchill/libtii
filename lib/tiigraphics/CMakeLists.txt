# TIILIB: lib/tiigraphics/CMakeLists.txt

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

PROJECT(tiigraphics VERSION 1.0.0 DESCRIPTION "Swarm Electric Field Instrument Thermal Ion Imager (TII) graphics library")
INCLUDE(GNUInstallDirs)

SET(LIBS ${LIBS} ${MATH} ${LIBXML2_LIBRARIES})
INCLUDE_DIRECTORIES(${INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIR})

ADD_LIBRARY(tiigraphics fonts.c draw.c video.c png.c spng.c)
SET_TARGET_PROPERTIES(tiigraphics PROPERTIES 
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER tiigraphics.h
)
TARGET_INCLUDE_DIRECTORIES(tiigraphics PRIVATE . )
TARGET_LINK_LIBRARIES(tiigraphics tii)

#install(TARGETS tiigraphics
#    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
# install(FILES ${CMAKE_BINARY_DIR}/mylib.pc
#     DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
