# CMakeLists.txt file for processor

CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT(tii_tools)

ADD_SUBDIRECTORY(tiim)
ADD_SUBDIRECTORY(tiim_stats)
ADD_SUBDIRECTORY(anomaly_stats)
ADD_SUBDIRECTORY(tii_moments)
ADD_SUBDIRECTORY(tii_dump)
ADD_SUBDIRECTORY(tii_look)

ADD_SUBDIRECTORY(get_image_stats)
