#!/bin/bash

# Dumps TII packet parameters to a file
# http://stackoverflow.com/questions/28226229/how-to-loop-through-dates-using-bash

if test "$#" -ne "6"; then
  echo "Usage: $0 satelliteLetter parameterIDs startDate stopDate numCpus outputDirectory"
  exit 1
fi

satellite=$1
parameters=$2
startDate=$3
stopDate=$4
numCpus=$5
outDir=$6

processingDate=$(date +%Y%m%dT%H%M%S)

dateToProcess="$startDate"
cpuNum=0

while [ $cpuNum -lt $numCpus ]; do
	cpuNum=$((cpuNum + 1))
	xterm -geometry 80x1 -e "process_tii_dump.sh ${satellite} ${parameters} ${dateToProcess} ${stopDate} $numCpus ${outDir}" &
	dateToProcess=$(date -I -d "$dateToProcess + 1 day")
done



