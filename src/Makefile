all: simple_client simple_app client_test make_torrent dump_torrent uc2 simple_work_generator

libs:
	ln -s `g++ -print-file-name=libstdc++.a`
	ln -s /home/boincadm/boinc-master/lib/libboinc.a
#	ln -s /usr/lib/libboinc.a
	ln -s /home/boincadm/boinc-master/api/libboinc_api.a
#	ln -s /usr/lib/libboinc_api.a

BOINC_BUILD = /home/boincadm/boinc-master

BOOST_MACROS = -DBOOST_ASIO_HASH_MAP_BUCKETS=1021 -DBOOST_EXCEPTION_DISABLE -DBOOST_ASIO_ENABLE_CANCELIO -DBOOST_ASIO_DYN_LINK
LIBTORRENT_MACROS = -DTORRENT_USE_OPENSSL -DWITH_SHIPPED_GEOIP_H -DTORRENT_LINKING_SHARED
MACROS = $(BOOST_MACROS) $(LIBTORRENT_MACROS)

INCLUDES = -I/usr/local/include -I/usr/local/include/libtorrent
LIBS = -L/usr/local/lib/ -L/usr/lib/ -ltorrent-rasterbar -lboost_system-mt

simple_client: simple_client.o
	g++ simple_client.o -o simple_client $(LIBS)

simple_client.o: simple_client.cpp
	g++ -c simple_client.cpp $(MACROS) $(INCLUDES)

simple_app: libs simple_app.o
	g++ simple_app.o -o simple_app libboinc_api.a libboinc.a -pthread

simple_app.o: simple_app.cpp
	g++ -c simple_app.cpp -I/usr/include -I/usr/include/boinc

simple_work_generator:
	cp simple_work_generator.cpp $(BOINC_BUILD)/sched/sample_work_generator.cpp
	cd $(BOINC_BUILD); make
	cp $(BOINC_BUILD)/sched/sample_work_generator.o ./simple_work_generator.o
	cp $(BOINC_BUILD)/sched/sample_work_generator ./simple_work_generator

uc2: libs uc2.o
	g++ uc2.o -o uc2 libboinc_api.a libboinc.a -pthread

uc2.o: uc2.cpp
	g++ -c uc2.cpp -I/usr/include

client_test: client_test.o
	g++ client_test.o -o client_test $(LIBS)

client_test.o: client_test.cpp
	g++ -c client_test.cpp $(MACROS) $(INCLUDES)

make_torrent: make_torrent.o
	g++ make_torrent.o -o make_torrent $(LIBS)

make_torrent.o: make_torrent.cpp
	g++ -c make_torrent.cpp $(MACROS) $(INCLUDES) 

dump_torrent: dump_torrent.o
	g++ dump_torrent.o -o dump_torrent $(LIBS) 

dump_torrent.o: dump_torrent.cpp
	g++ -c dump_torrent.cpp $(MACROS) $(INCLUDES) 

clean:
	rm *.o *.a simple_client client_test make_torrent dump_torrent simple_app uc2 simple_work_generator
