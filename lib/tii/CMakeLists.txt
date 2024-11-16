# TIILIB: lib/tii/CMakeLists.txt

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

PROJECT(tii VERSION 1.0.0 DESCRIPTION "Swarm Electric Field Instrument Thermal Ion Imager (TII) processing library")
#INCLUDE(GNUInstallDirs)

# FIND_LIBRARY(MATH m)
# FIND_PACKAGE(LibXml2)
SET(LIBS ${LIBS} ${MATH} ${LIBXML2_LIBRARIES})
INCLUDE_DIRECTORIES(${INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIR})

ADD_LIBRARY(tii analysis.c filters.c gainmap.c import.c isp.c timeseries.c utility.c xml.c detector.c)
SET_TARGET_PROPERTIES(tii PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)
TARGET_INCLUDE_DIRECTORIES(tii PRIVATE . )
TARGET_LINK_LIBRARIES(tii ${LIBS})

SET(INSTALL_TII_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include/tii)
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/include/tii/
        DESTINATION ${INSTALL_TII_INCLUDE_DIR}
        FILES_MATCHING PATTERN "*.h"
)

SET(TII_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
INSTALL(TARGETS tii
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

SET(INSTALL_PKGCONFIG_DIR ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/lib/tii/libtii.pc.in ${CMAKE_BINARY_DIR}/libtii.pc @ONLY)

INSTALL(FILES ${CMAKE_BINARY_DIR}/libtii.pc
        DESTINATION ${INSTALL_PKGCONFIG_DIR}
)

