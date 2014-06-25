#!/bin/bash

#ARGS="200 0 0 1.25 250 16 $1 4 3 64 16 16"
ARGS="0 0 5 1.25 250 4 2 1 2 64 16 16"

java -cp .:../bin freeCycles.model.Main `echo $ARGS`
