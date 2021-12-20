#!/bin/bash

(cd /tmp/TII-movies-simple-historical;


HDRFILE=$1
BASE=`basename $HDRFILE`
SAT=`echo $BASE | cut -c 12`
DATE=`echo $BASE | cut -c 20-25`
SCALE=$2

MOVIEFILE=`tiim $1 $2`

/usr/local/bin/ffmpeg -y -loglevel quiet -an -i EFI${SAT}_%05d.png  -s 844x390 -c:v libx264 -vf "fps=15,format=yuv420p" -profile:v baseline -level 3 /data/Movies/Swarm/${DATE}/${MOVIEFILE}

echo "tiim'd backup movie ${MOVIEFILE}"

rm -f EFI${SAT}_?????.png

)
