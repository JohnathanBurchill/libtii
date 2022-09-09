# TIILIB: Dockerfile

# Copyright (C) 2022  Johnathan K Burchill

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
VOLUME ["/home/dataflow/src"]
ENTRYPOINT ["/bin/bash"]



