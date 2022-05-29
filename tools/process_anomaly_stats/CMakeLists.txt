CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT(process_anomaly_stats)

SET(LIBS ${LIBS} ${MATH} )
INCLUDE_DIRECTORIES(${INCLUDE_DIRS} )

ADD_EXECUTABLE(process_anomaly_stats process_anomaly_stats.c)
TARGET_LINK_LIBRARIES(process_anomaly_stats ${LIBS} -lm)

install(TARGETS process_anomaly_stats DESTINATION $ENV{HOME}/bin)

