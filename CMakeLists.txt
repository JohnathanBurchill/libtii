# CMakeLists.txt file for processor
PROJECT(tiim)

CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

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
FIND_LIBRARY(SPNG spng)
FIND_PACKAGE(LibXml2)
FIND_PATH(SPNG_INCLUDE spng.h)

SET(LIBS ${LIBS} ${MATH} ${LIBXML2_LIBRARIES} ${SPNG} )


INCLUDE_DIRECTORIES(${INCLUDE_DIRS} ${SPNG_INCLUDE} ${LIBXML2_INCLUDE_DIR})

ADD_EXECUTABLE(tiim tiim.c isp.c xml.c png.c fonts.c analysis.c draw.c import.c utility.c)
TARGET_LINK_LIBRARIES(tiim ${LIBS})

install(TARGETS tiim DESTINATION $ENV{HOME}/bin)
install(PROGRAMS Swarm_fast_movie_generator.sh DESTINATION $ENV{HOME}/bin)
install(PROGRAMS Swarm_fast_movie_generator_historical.sh DESTINATION $ENV{HOME}/bin)

