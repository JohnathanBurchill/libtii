CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT(tii_moments)

# SET(LIBS ${LIBS} ${MATH} ${LIBXML2_LIBRARIES} ${ZLIB_LIBRARIES})
INCLUDE_DIRECTORIES(${INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIR})

ADD_EXECUTABLE(tii_moments tii_moments.c)
TARGET_LINK_LIBRARIES(tii_moments ${LIBS} tii -lm)

install(TARGETS tii_moments DESTINATION $ENV{HOME}/bin)

install(PROGRAMS parallel_process_tii_moments.sh DESTINATION $ENV{HOME}/bin)
install(PROGRAMS process_tii_moments.sh DESTINATION $ENV{HOME}/bin)

