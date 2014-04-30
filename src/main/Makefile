all: simple_app simple_work_generator

# FIXME - needed?
libs:
	ln -s `g++ -print-file-name=libstdc++.a`
#	ln -s /home/boincadm/boinc-master/lib/libboinc.a
	ln -s /usr/lib/libboinc.a
#	ln -s /home/boincadm/boinc-master/api/libboinc_api.a
	ln -s /usr/lib/libboinc_api.a

#BOINC_BUILD = /home/boincadm/boinc-master
BOINC_BUILD = /home/underscore/CloudPT/freeCycles-software/boinc-master

BOOST_MACROS = -DBOOST_ASIO_HASH_MAP_BUCKETS=1021 -DBOOST_EXCEPTION_DISABLE -DBOOST_ASIO_ENABLE_CANCELIO -DBOOST_ASIO_DYN_LINK
LIBTORRENT_MACROS = -DTORRENT_USE_OPENSSL -DWITH_SHIPPED_GEOIP_H -DTORRENT_LINKING_SHARED
MACROS = $(BOOST_MACROS) $(LIBTORRENT_MACROS)

BOINC_LIBS = /usr/lib/libboinc_api.a /usr/lib/libboinc.a -pthread
LIBTORRENT_LIBS = -L/usr/local/lib/ -L/usr/lib/ -ltorrent-rasterbar -lboost_system-mt

INCLUDES = -I/usr/include -I/usr/local/include -I/usr/local/include/libtorrent -I/usr/include/boinc

FLAGS = -Wall -g3 -pedantic -w -Wextra -Werror

simple_app: simple_app.o
	g++ simple_app.o -o simple_app $(BOINC_LIBS) $(LIBTORRENT_LIBS)
	
simple_app.o: simple_app.cpp mr_tasktracker.h data_handler.h
	g++ -c simple_app.cpp $(MACROS) $(INCLUDES) $(FLAGS)

simple_work_generator:
	cp simple_work_generator.cpp $(BOINC_BUILD)/sched/sample_work_generator.cpp
	cd $(BOINC_BUILD); make
	cp $(BOINC_BUILD)/sched/sample_work_generator.o ./simple_work_generator.o
	cp $(BOINC_BUILD)/sched/sample_work_generator ./simple_work_generator

clean:
	rm *.o simple_app simple_work_generator
