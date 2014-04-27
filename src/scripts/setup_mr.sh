#!/bin/bash

# This script receives:
# - the number of mappers 'nmaps';
# - the number of reducers 'nreds';
# - the input file 'ifile'.
# This script will:
# - prepare the jobtracker.xml file (describing all map and reduce jobs);
# - split the file in 'nmaps' files with the same number of lines;
# - create .torrent files for each of the file parts and will place them inside
# a well known location.

PRE_STAGE_DIR=/tmp
UPLOAD_DIR=/tmp
MAKE_TORRENT=/home/underscore/git/freeCycles/src/util/bt/make_torrent
TRACKER=udp://boinc.rnl.ist.utl.pt:6969
JOBTRACKER_FILE=/tmp/jobtracker.xml

function split_input_file {
  # number of lines per split = total number of lines divided by the number
  # of mappers plus one.
  linesf=$(( $(( $(wc -l < $ifile) / $nmaps)) + 1))
  prefix=$id-map-
  split -l $linesf $ifile $prefix
}

function prepare_splits {
  aux=0
  for file in $id-map-*
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
  # write reduce task information
  # remember that some magic will happen (see shuffle.sh)
  for ((aux=0; aux<$nreds; aux++))
  do
    echo "<reduce>"
    echo "<name>$id-reduce-$aux</name>"
    echo "<status>w</status>"
    echo "<input>$PRE_STAGE_DIR/$id-reduce-$aux.zip</input>"
    echo "<output>$UPLOAD_DIR/$id-reduce-$aux.torrent</input>"
    echo "</reduce>"
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

if [ "$#" -ne 3 ]; then
  echo "Illegal number of parameters."
  echo "Usage ./setup-mr.sh nmaps nreds ifile"
  exit 
fi

nmaps=$1
nreds=$2
ifile=$3
id=$(date +%s)

cd $PRE_STAGE_DIR
split_input_file
prepare_splits
print_jobtracker >> $JOBTRACKER_FILE
make_torrents

