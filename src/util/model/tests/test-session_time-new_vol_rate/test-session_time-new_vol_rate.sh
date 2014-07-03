#!/bin/bash

#number_nodes=120
number_nodes=120
#session_time=0
session_time=120
new_volunteer_rate=1
upload_bandwidth=1.25
processing_power=250
maps=16
map_repl=3
reds=4
red_repl=3
input_size=256
interm_size=64
output_size=64
time_to_repl=1

java -cp .:../../bin freeCycles.model.Main 	\
	$number_nodes                   	\
	$session_time                   	\
	$new_volunteer_rate                  	\
        $upload_bandwidth			\
        $processing_power		        \
        $maps                           	\
        $map_repl                       	\
        $reds                           	\
        $red_repl                       	\
	$input_size                     	\
        $interm_size                    	\
        $output_size                    	\
        $time_to_repl	          		\
        >> log 2>> err
