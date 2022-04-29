#!/bin/bash

# Dumps packet parameters for the given input dates. 

# http://stackoverflow.com/questions/28226229/how-to-loop-through-dates-using-bash

if test "$#" -ne "6"; then
  echo "Usage: $0 satelliteLetter parameters startDate stopDate stride outputDirectory"
  exit 1
fi

satellite=$1
parameters=$2
startDate=$(date -d "$3" +%s)
stopDate=$(date -d "$4" +%s)
stride=$5
outDir=$6

processingDate=$(date +%Y%m%dT%H%M%S)

dateToProcess="$startDate"

daysToProcess=0
d1=$startDate
d2=$stopDate
while [ $d1 -le $d2 ]; do
	daysToProcess=$((daysToProcess + 1))
	d1=$((d1 + stride*86400))
done

# https://stackoverflow.com/questions/238073/how-to-add-a-progress-bar-to-a-shell-script
PROGRESS_BAR_WIDTH=30  # progress bar length in characters
draw_progress_bar() {
  # Arguments: current value, max value, unit of measurement (optional)
  local __value=$1
  local __max=$2
  local __unit=${3:-""}  # if unit is not supplied, do not display it

  # Calculate percentage
  if (( $__max < 1 )); then __max=1; fi  # anti zero division protection
  local __percentage=$(( 100 - ($__max*100 - $__value*100) / $__max ))

  # Rescale the bar according to the progress bar width
  local __num_bar=$(( $__percentage * $PROGRESS_BAR_WIDTH / 100 ))

  # Draw progress bar
  printf "["
  for b in $(seq 1 $__num_bar); do printf "#"; done
  for s in $(seq 1 $(( $PROGRESS_BAR_WIDTH - $__num_bar ))); do printf " "; done
  printf "] $__percentage%% ($__value / $__max $__unit)\r"
}
(cd /data/Swarm/EFI/Level0/Treated;
while [ "$dateToProcess" -le "$stopDate" ]; do
	
	year=$(date -d "@$dateToProcess" +%Y)
	month=$(date -d "@$dateToProcess" +%m)
	day=$(date -d "@$dateToProcess" +%d)
	daysProcessed=$((daysProcessed + 1))
	datestring=`date -I -d "@$dateToProcess"`
	draw_progress_bar $daysProcessed $daysToProcess "days: TIIM_DUMP ${satellite} $datestring"
 	tii_dump ${satellite}${year}${month}${day} ${parameters} ${outDir}
	dateToProcess=$((dateToProcess + stride * 86400))
done
)

echo "["
