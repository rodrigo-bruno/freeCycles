#!/bin/bash

ARGS="10 10 1000 16 3 4 3 1000 1000 1000"

java -cp .:../bin freeCycles.model.Main `echo $ARGS`
