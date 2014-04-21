#!/bin/bash

# This script receives:
# - the number of reducers 'nreds';
# This script will:
# - emulate the shuffle fase done in a MapReduce workflow.

PRE_STAGE_DIR=/tmp
UPLOAD_DIR=/tmp

unzip $UPLOAD_DIR/$id-map-*.zip -d $PRE_STAGE_DIR
for ((aux=0; aux<$nreds; aux++))
do 
  zip $PRE_STAGE_DIR/$id-reduce-$aux.zip $PRE_STAGE_DIR/$id-map-*-$aux.torrent
done
rm $PRE_STAGE_DIR/$id-map-*

