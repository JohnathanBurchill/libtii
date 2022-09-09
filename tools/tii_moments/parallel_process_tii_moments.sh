#!/bin/bash

# TIILIB: tools/tii_moments/parallel_process_tii_moments.sh

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

# processes TII images to determine presence of anomalies for specified satellite for a range of dates
# http://stackoverflow.com/questions/28226229/how-to-loop-through-dates-using-bash

if test "$#" -ne "6"; then
  echo "Usage: $0 satelliteLetter gainMapId startDate stopDate numCpus outputDirectory"
  exit 1
fi

satellite=$1
gainMapId=$2
startDate=$3
stopDate=$4
numCpus=$5
outDir=$6

processingDate=$(date +%Y%m%dT%H%M%S)

dateToProcess="$startDate"
cpuNum=0

while [ $cpuNum -lt $numCpus ]; do
	cpuNum=$((cpuNum + 1))
	xterm -geometry 80x1 -e "process_tii_moments.sh ${satellite} ${gainMapId} ${dateToProcess} ${stopDate} $numCpus ${outDir}" &
	dateToProcess=$(date -I -d "$dateToProcess + 1 day")
done



