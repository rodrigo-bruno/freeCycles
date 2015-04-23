#!/bin/bash

# This script receives:
# - a directory where input files are staged;
# This script will:
# - prepare the jobtracker.xml file describing all tasks (map tasks) ;
# - create .torrent file for each input file and will place it inside a well 
# known location.

PRE_STAGE_DIR=/tmp
UPLOAD_DIR=/tmp
MAKE_TORRENT=/home/underscore/git/freeCycles/src/util/bt/make_torrent
TRACKER=udp://boinc.rnl.ist.utl.pt:6969
JOBTRACKER_FILE=/tmp/jobtracker.xml

function prepare_inputs {
  aux=0
  for file in $idir
  do
    mv $file $id-map-$aux
    aux=$((aux + 1))
  done 
}

function print_jobtracker {
  aux=0
  # place outter tag
  echo "<mr>"
  # write the mr identifier
  echo "<id>"$id"</id>"
  # write the shuffle status (initialized to false)
  echo "<shuffled>0</shuffled>"
  # write map task information
  for file in $id-map-*
  do
    echo "<map>"
    echo "<name>$file</name>"
    echo "<status>w</status>"
    echo "<input>$PRE_STAGE_DIR/$file.torrent</input>"
    echo "<output>$UPLOAD_DIR/$file.zip</output>"
    echo "</map>"
  done

  # close outter tag
  echo "</mr>" >> $JOBTRACKER_FILE
}

function make_torrents {
  for file in $id-map-*
  do
    $MAKE_TORRENT $file -t $TRACKER -o $file.torrent
  done

}

if [ "$#" -ne 1 ]; then
  echo "Illegal number of parameters."
  echo "Usage ./setup-parellel.sh idir"
  exit 
fi

idir=$1
id=$(date +%s)

cd $PRE_STAGE_DIR
prepare_splits
print_jobtracker >> $JOBTRACKER_FILE
make_torrents

