#!/bin/bash

# This script receives:
# - the number of reducers 'nreds';
# This script will:
# - emulate the shuffle fase done in a MapReduce workflow.

PRE_STAGE_DIR=/tmp/freeCycles-boinc-slot
UPLOAD_DIR=/tmp/freeCycles-boinc-slot

if [ "$#" -ne 2 ]; then
  echo "Illegal number of parameters."
  echo "Usage ./shuffle.sh jobID nreds"
  exit 
fi

id=$1
nreds=$2

unzip $UPLOAD_DIR/$id-map-*.zip -d $PRE_STAGE_DIR
cd $PRE_STAGE_DIR
for ((aux=0; aux<$nreds; aux++))
do 
  echo $id-map-*-$aux* > .files
  zip -j0 $id-reduce-$aux.zip $id-map-*-$aux* .files
done
cd -
rm $PRE_STAGE_DIR/.files $PRE_STAGE_DIR/$id-map-*-*

