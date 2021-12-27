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
RUN apt -y install libavcodec-dev
RUN apt -y install libswscale-dev
RUN apt -y install libavformat-dev
RUN apt -y install libswresample-dev
RUN apt -y install libx264-dev
RUN apt -y install git
RUN apt -y install gdb
RUN apt -y install cgdb
RUN apt -y install vim
RUN apt -y install pkg-config
RUN addgroup science
RUN useradd -G science dataflow
USER dataflow
WORKDIR /home/dataflow/src
#RUN mkdir build && \
#cd build && \
#cmake .. && \
#make install 
VOLUME ["/home/dataflow/src"]
ENTRYPOINT ["/bin/bash"]



