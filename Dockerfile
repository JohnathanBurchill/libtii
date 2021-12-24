FROM ubuntu
RUN apt -y update
RUN apt -y upgrade
RUN apt -y install wget
RUN apt -y install gcc
RUN apt -y install g++
RUN apt -y install cmake
RUN apt -y install make
RUN apt -y install bash
RUN apt -y install libxml2
RUN apt -y install libxml2-dev
RUN apt -y install zlib1g
RUN apt -y install zlib1g-dev
RUN apt -y install libavutil-dev
RUN addgroup science
RUN useradd -G science dataflow
USER dataflow
#SHELL ["/usr/bin/bash", ""]
WORKDIR /home/dataflow/src
COPY . .
RUN mkdir build && \
cd build && \
cmake .. && \
make install 


