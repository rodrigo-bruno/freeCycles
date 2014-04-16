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

JOBTRACKER_FILE=/tmp/jobtracker.xml
PRE_STAGE_DIR=/tmp/pre_stage
UPLOAD_DIR=/tmp/upload

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
    # make_torrent
    aux=$((aux + 1))
  done 
}

function print_jobtracker {
  aux=0
  # place outter tag
  echo "<mr>"
  # write the mr identifier
  echo "<id>"$id"</id>"
  # write map task information
  for file in $id-map-*
  do
    echo "<map>"
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
    echo "<status>w</status>"
    echo "<input>$PRE_STATE_DIR/$id-reduce-$aux.zip</input>"
    echo "<output>$UPLOAD_DIR/$id-reduce-$aux.torrent</input>"
    echo "</reduce>"
  done

  # close outter tag
  echo "</mr>" >> $JOBTRACKER_FILE
}

if [ "$#" -ne 3 ]; then
  echo "Illegal number of parameters."
  echo "Usage ./setup-mr.sh nmaps nreds ifile"
  exit 
fi


# shuffle will be:
# cd $UPLOAD_DIR
# unzip $UPLOAD_DIR/$id-map-*.zip
# for ((aux=0; aux<$nreds; aux++))
# do 
#  zip $id-reduce-$aux.zip $id-map-*$aux.torrent
# done

nmaps=$1
nreds=$2
ifile=$3
id=$(date +%s)

cd $PRE_STAGE_DIR
split_input_file
prepare_splits
print_jobtracker >> $JOBTRACKER_FILE

