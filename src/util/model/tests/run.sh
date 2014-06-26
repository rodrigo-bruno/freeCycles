#!/bin/bash

#ARGS="200 0 0 1.25 250 16 $1 4 3 64 16 16 0 0"
ARGS="0 30 1 1.25 250 4 1 2 1 64 16 16 1 0"

java -cp .:../bin freeCycles.model.Main `echo $ARGS`
