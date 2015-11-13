freeCycles
==========

freeCycles is a BOINC compatible Volunteer Computing system that enables volunteers to run MapReduce and use BitTorrent to transfer input, intermediate, and final output.

How to use freeCycles:

1 - setup a BOINC server (https://boinc.berkeley.edu/trac/wiki/ServerIntro). You can use the default project that comes with the server image;
2 - setup a BitTorrent tracker (we used opentracker, available at http://erdgeist.org/arts/software/opentracker/);
2 - clone this repo (into the BOINC server)
3 - go to src/main and use "make" to compile our MapReduce application;
4 - go to src/utl/bt and use "make" to compile our BitTorrent client;
5 - install the application (compiled in the previous step) into the server as you would install a regular BOINC application;
6 - prepare the MapReduce job: src/scripts/setup\_mr.sh <number of mappers> <number of reducers> <input file>;
7 - start all BOINC daemons plus the BitTorrent client and BitTorrent tracker

At this point, worker nodes should contact the server and receive map tasks. After all map tasks are finished, the server will start to deliver reduce tasks. When all reduce tasks are finished, no more tasks are created.
