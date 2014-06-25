#!/bin/bash

ARGS="200 1.25 250 16 $1 4 3 256 64 64"

java -cp .:../bin freeCycles.model.Main `echo $ARGS`
