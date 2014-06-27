#!/bin/bash

number_nodes=100
session_time=0
new_volunteer_rate=0
upload_bandwidth=1.25
processing_power=250
maps=4
map_repl=1
reds=2
red_repl=1
input_size=128
interm_size=16
output_size=16
time_to_repl=1

java -cp .:../bin freeCycles.model.Main 	\
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
