CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT(anomaly_stats)

# SET(LIBS ${LIBS} ${MATH} ${LIBXML2_LIBRARIES} ${ZLIB_LIBRARIES})
INCLUDE_DIRECTORIES(${INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIR})

ADD_EXECUTABLE(anomaly_stats anomaly_stats.c)
TARGET_LINK_LIBRARIES(anomaly_stats ${LIBS} tii -lm)

install(TARGETS anomaly_stats DESTINATION $ENV{HOME}/bin)

install(PROGRAMS parallel_process_anomaly_stats.sh DESTINATION $ENV{HOME}/bin)
install(PROGRAMS process_anomaly_stats.sh DESTINATION $ENV{HOME}/bin)

