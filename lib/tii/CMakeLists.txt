PROJECT(tii VERSION 1.0.0 DESCRIPTION "Swarm Electric Field Instrument Thermal Ion Imager (TII) processing library")
INCLUDE(GNUInstallDirs)

# FIND_LIBRARY(MATH m)
# FIND_PACKAGE(LibXml2)
SET(LIBS ${LIBS} ${MATH} ${LIBXML2_LIBRARIES})
INCLUDE_DIRECTORIES(${INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIR})

ADD_LIBRARY(tii analysis.c filters.c gainmap.c import.c isp.c timeseries.c utility.c xml.c)
SET_TARGET_PROPERTIES(tii PROPERTIES 
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER tii.h
)
TARGET_INCLUDE_DIRECTORIES(tii PRIVATE . )
TARGET_LINK_LIBRARIES(tii ${LIBS})

install(TARGETS tii
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})