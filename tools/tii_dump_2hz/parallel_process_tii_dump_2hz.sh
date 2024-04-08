#!/bin/bash

# TIILIB: tools/tii_dump_2hz/parallel_process_tii_dump_2hz.sh

# Copyright (C) 2024  Johnathan K Burchill

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

# processes TII images to determine presence of anomalies for specified satellite for a range of dates
# http://stackoverflow.com/questions/28226229/how-to-loop-through-dates-using-bash

if test "$#" -ne "6"; then
  echo "Usage: $0 satelliteLetter startDate stopDate numCpus outputDirectory averagingWindowSeconds"
  exit 1
fi

satellite=$1
startDate=$2
stopDate=$3
numCpus=$4
outDir=$5
intervalSeconds=$6

processingDate=$(date +%Y%m%dT%H%M%S)

dateToProcess="$startDate"
cpuNum=0

while [ $cpuNum -lt $numCpus ]; do
	cpuNum=$((cpuNum + 1))
	xterm -geometry 80x1 -e "process_tii_dump_2hz.sh ${satellite} ${dateToProcess} ${stopDate} $numCpus ${outDir} ${intervalSeconds}" &
	dateToProcess=$(date -I -d "$dateToProcess + 1 day")
done



