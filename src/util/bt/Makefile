all: simple_client make_torrent client_test dump_torrent

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

simple_client: simple_client.o
	g++ simple_client.o -o simple_client $(LIBTORRENT_LIBS)

simple_client.o: simple_client.cpp
	g++ -c simple_client.cpp $(MACROS) $(INCLUDES) $(FLAGS)

client_test: client_test.o
	g++ client_test.o -o client_test $(LIBTORRENT_LIBS)

client_test.o: client_test.cpp
	g++ -c client_test.cpp $(MACROS) $(INCLUDES) $(FLAGS)

make_torrent: make_torrent.o
	g++ make_torrent.o -o make_torrent $(LIBTORRENT_LIBS)

make_torrent.o: make_torrent.cpp
	g++ -c make_torrent.cpp $(MACROS) $(INCLUDES)  $(FLAGS)

dump_torrent: dump_torrent.o
	g++ dump_torrent.o -o dump_torrent $(LIBTORRENT_LIBS) 

dump_torrent.o: dump_torrent.cpp
	g++ -c dump_torrent.cpp $(MACROS) $(INCLUDES) $(FLAGS)

clean:
	rm *.o simple_client client_test make_torrent dump_torrent
