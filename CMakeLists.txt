# CMakeLists.txt file for processor
PROJECT(tiim)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")

ADD_DEFINITIONS(-DHAVE_GETOPT_H)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
    message( "-- Debugging enabled")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -std=gnu99 ")
else (CMAKE_BUILD_TYPE STREQUAL Debug)
    message( "-- Optimization enabled")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -std=gnu99 ")
endif(CMAKE_BUILD_TYPE STREQUAL Debug)

# requires -lm on linux
FIND_LIBRARY(MATH m)
FIND_PACKAGE(LibXml2)

SET(LIBS ${LIBS} ${MATH} ${LIBXML2_LIBRARIES} )


INCLUDE_DIRECTORIES(${INCLUDE_DIRS})

ADD_EXECUTABLE(tiim tiim.c isp.c xml.c)
TARGET_LINK_LIBRARIES(tiim ${LIBS})

install(TARGETS tiim DESTINATION $ENV{HOME}/bin)

