#!/bin/bash

number_nodes=0
session_time=120
new_volunteer_rate=1
upload_bandwidth=1.25
processing_power=250
maps=16
map_repl=3
reds=4
red_repl=3
input_size=64
interm_size=16
output_size=16
time_repl_task=600
time_repl_idata=0

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
        $time_repl_task          		\
        $time_repl_idata         		\
        >> log 2>> err
