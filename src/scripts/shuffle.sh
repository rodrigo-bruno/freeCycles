#!/bin/bash

# This script receives:
# - the number of reducers 'nreds';
# This script will:
# - emulate the shuffle fase done in a MapReduce workflow.

PRE_STAGE_DIR=/tmp
UPLOAD_DIR=/tmp

if [ "$#" -ne 2 ]; then
  echo "Illegal number of parameters."
  echo "Usage ./shuffle.sh jobID nreds"
  exit 
fi

id=$1
nreds=$2

unzip $UPLOAD_DIR/$id-map-*.zip -d $PRE_STAGE_DIR
for ((aux=0; aux<$nreds; aux++))
do 
  echo $id-map-*-$aux.torrent > .files
  zip $PRE_STAGE_DIR/$id-reduce-$aux.zip $PRE_STAGE_DIR/$id-map-*-$aux.torrent .files
done
rm $PRE_STAGE_DIR/$id-map-*

