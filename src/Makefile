all: simple_client simple_app client_test make_torrent dump_torrent uc2

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

simple_client: simple_client.o
	g++ simple_client.o -o simple_client $(LIBTORRENT_LIBS)

simple_client.o: simple_client.cpp
	g++ -c simple_client.cpp $(MACROS) $(INCLUDES)

simple_app: simple_app.o
	g++ simple_app.o -o simple_app $(BOINC_LIBS) $(LIBTORRENT_LIBS)
	
simple_app.o: simple_app.cpp
	g++ -c simple_app.cpp $(MACROS) $(INCLUDES)

simple_work_generator:
	cp simple_work_generator.cpp $(BOINC_BUILD)/sched/sample_work_generator.cpp
	cd $(BOINC_BUILD); make
	cp $(BOINC_BUILD)/sched/sample_work_generator.o ./simple_work_generator.o
	cp $(BOINC_BUILD)/sched/sample_work_generator ./simple_work_generator

uc2: uc2.o
	g++ uc2.o -o uc2 $(BOINC_LIBS)

uc2.o: uc2.cpp
	g++ -c uc2.cpp $(INCLUDES)

client_test: client_test.o
	g++ client_test.o -o client_test $(LIBTORRENT_LIBS)

client_test.o: client_test.cpp
	g++ -c client_test.cpp $(MACROS) $(INCLUDES)

make_torrent: make_torrent.o
	g++ make_torrent.o -o make_torrent $(LIBTORRENT_LIBS)

make_torrent.o: make_torrent.cpp
	g++ -c make_torrent.cpp $(MACROS) $(INCLUDES) 

dump_torrent: dump_torrent.o
	g++ dump_torrent.o -o dump_torrent $(LIBTORRENT_LIBS) 

dump_torrent.o: dump_torrent.cpp
	g++ -c dump_torrent.cpp $(MACROS) $(INCLUDES) 

clean:
	rm *.o simple_client client_test make_torrent dump_torrent simple_app uc2 simple_work_generator
