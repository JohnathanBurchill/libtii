#!/bin/bash

# processes TII images to determine presence of anomalies for specified satellite for a range of dates
# http://stackoverflow.com/questions/28226229/how-to-loop-through-dates-using-bash

if test "$#" -ne "5"; then
  echo "Usage: $0 satelliteLetter gainMapId startDate stopDate numCpus"
  exit 1
fi

satellite=$1
gainMapId=$2
startDate=$3
stopDate=$4
numCpus=$5

processingDate=$(date +%Y%m%dT%H%M%S)

dateToProcess="$startDate"
cpuNum=0

while [ $cpuNum -lt $numCpus ]; do
	cpuNum=$((cpuNum + 1))
	xterm -geometry 80x1 -e "process_tii_moments.sh ${satellite} ${gainMapId} ${dateToProcess} ${stopDate} $numCpus" &
	dateToProcess=$(date -I -d "$dateToProcess + 1 day")
done



