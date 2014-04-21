all: parser_test uc2

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

parser_test: parser_test.o
	g++ parser_test.o -g -o parser_test $(BOINC_LIBS) $(LIBTORRENT_LIBS)
	
parser_test.o: parser_test.cpp
	g++ -c parser_test.cpp $(MACROS) $(INCLUDES) -I../../main $(FLAGS)

uc2: uc2.o
	g++ uc2.o -o uc2 $(BOINC_LIBS)

uc2.o: uc2.cpp
	g++ -c uc2.cpp $(INCLUDES)

clean:
	rm *.o parser_test uc2
