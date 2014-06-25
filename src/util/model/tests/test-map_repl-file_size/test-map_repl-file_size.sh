#!/bin/bash

ARGS="200 1.25 250 16 $1 4 3 256 64 64"

number_nodes=200
upload_bandwidth=1.25
processing_power=250
maps=16
reds=4
red_repl=3

for map_repl in {3..10} 
do
	for file_size in {64,128,256,512,1024}
	do
		input_size=$file_size
		interm_size=$((input_size / 8))
		output_size=$((input_size/8))
		echo "map_repl=$map_repl, file_size=$file_size" >> err
		java -cp .:../bin freeCycles.model.Main \
			$number_nodes 			\
			$upload_bandwidth 		\
			$processing_power 		\
			$maps 				\
			$map_repl 			\
			$reds 				\
			$red_repl 			\
			$input_size 			\
			$interm_size 			\
			$output_size			\
			>> log.$map_repl.$file_size 2>> err
	done
done
