#!/bin/bash

# processes TII NOM and TIC Level 0 binary files for specified satellite for a range of dates
# http://stackoverflow.com/questions/28226229/how-to-loop-through-dates-using-bash

satellite=$1
startDate=$2
stopDate=$3
numCpus=$4

if test "$#" -ne "4"; then
  echo "Usage: $0 satelliteLetter startDate stopDate numCpus"
  exit 1
fi

processingDate=$(date +%Y%m%dT%H%M%S)

dateToProcess="$startDate"
cpuNum=0

while [ $cpuNum -lt $numCpus ]; do
	cpuNum=$((cpuNum + 1))
	xterm -geometry 80x1 -e "process_TIIM.sh ${satellite} ${dateToProcess} ${stopDate} $numCpus" &
	dateToProcess=$(date -I -d "$dateToProcess + 1 day")
done



