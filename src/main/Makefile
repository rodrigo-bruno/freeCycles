all: simple_app simple_work_generator simple_assimilator

#BOINC_BUILD = /home/boincadm/boinc-master
BOINC_BUILD = /home/underscore/CloudPT/freeCycles-software/boinc-master

BOOST_MACROS = -DBOOST_ASIO_HASH_MAP_BUCKETS=1021 -DBOOST_EXCEPTION_DISABLE -DBOOST_ASIO_ENABLE_CANCELIO -DBOOST_ASIO_DYN_LINK
LIBTORRENT_MACROS = -DTORRENT_USE_OPENSSL -DWITH_SHIPPED_GEOIP_H -DTORRENT_LINKING_SHARED
MACROS = $(BOOST_MACROS) $(LIBTORRENT_MACROS)

BOINC_LIBS = /usr/lib/libboinc_api.a /usr/lib/libboinc.a -pthread
LIBTORRENT_LIBS = -L/usr/local/lib/ -L/usr/lib/ -ltorrent-rasterbar -lboost_system-mt
#OPENCV_LIBS = -L/usr/local/lib/ -L/usr/lib/ -lcvaux -lcv -lcxcore -lhighgui
OPENCV_LIBS = -L/usr/local/lib/ -L/usr/lib/ -lopencv_core -lopencv_imgproc -lopencv_highgui

INCLUDES = -I/usr/include -I/usr/local/include -I/usr/local/include/libtorrent -I/usr/include/boinc

FLAGS = -Wall -g3 -pedantic -w -Wextra -Werror

simple_app: simple_app.o
	g++ simple_app.o -o simple_app $(BOINC_LIBS) $(LIBTORRENT_LIBS)
	
simple_app.o: simple_app.cpp mr_tasktracker.h data_handler.h control.h benchmarks.h
	g++ -c simple_app.cpp $(MACROS) $(INCLUDES) $(FLAGS)

simple_work_generator: simple_work_generator.cpp mr_jobtracker.h mr_parser.h
	cp simple_work_generator.cpp $(BOINC_BUILD)/sched/sample_work_generator.cpp 
	cp mr_jobtracker.h mr_parser.h $(BOINC_BUILD)/sched/
	cd $(BOINC_BUILD); make
	cp $(BOINC_BUILD)/sched/sample_work_generator.o ./simple_work_generator.o
	cp $(BOINC_BUILD)/sched/sample_work_generator ./simple_work_generator

simple_assimilator: simple_assimilator.cpp mr_jobtracker.h mr_parser.h
	cp simple_assimilator.cpp $(BOINC_BUILD)/sched/sample_assimilator.cpp 
	cp mr_jobtracker.h mr_parser.h $(BOINC_BUILD)/sched/
	cd $(BOINC_BUILD); make
	cp $(BOINC_BUILD)/sched/sample_assimilator.o ./simple_assimilator.o
	cp $(BOINC_BUILD)/sched/sample_assimilator ./simple_assimilator

opencv_canny: opencv_canny.o
	g++ -o opencv_canny opencv_canny.o $(OPENCV_LIBS)

opencv_canny.o: opencv_canny.cpp
	g++ -c opencv_canny.cpp $(INCLUDES) $(FLAGS)

clean:
	rm *.o simple_app simple_work_generator simple_assimilator opencv_canny
