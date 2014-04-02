all: simple_client simple_app client_test make_torrent dump_torrent uc2

libs:
#	ln -s `g++ -print-file-name=libstdc++.a`
	ln -s /home/boincadm/boinc-master/lib/libboinc.a
#	ln -s /usr/lib/libboinc.a
	ln -s /home/boincadm/boinc-master/api/libboinc_api.a
#	ln -s /usr/lib/libboinc_api.a


simple_client: simple_client.o
	g++ simple_client.o -o simple_client -L/usr/local/lib/ -L/usr/lib/ -ltorrent-rasterbar -lboost_system-mt

simple_client.o: simple_client.cpp
	g++ -c simple_client.cpp -DTORRENT_USE_OPENSSL -DWITH_SHIPPED_GEOIP_H -DBOOST_ASIO_HASH_MAP_BUCKETS=1021 -DBOOST_EXCEPTION_DISABLE -DBOOST_ASIO_ENABLE_CANCELIO -DBOOST_ASIO_DYN_LINK -DTORRENT_LINKING_SHARED -I/usr/local/include -I/usr/local/include/libtorrent

simple_app: libs simple_app.o
	g++ simple_app.o -o simple_app libboinc_api.a libboinc.a -pthread

simple_app.o: simple_app.cpp
	g++ -c simple_app.cpp -I/usr/include

uc2: libs uc2.o
	g++ uc2.o -o uc2 libboinc_api.a libboinc.a -pthread

uc2.o: uc2.cpp
	g++ -c uc2.cpp -I/usr/include

client_test: client_test.o
	g++ client_test.o -o client_test -L/usr/local/lib/ -L/usr/lib/ -ltorrent-rasterbar -lboost_system-mt

client_test.o: client_test.cpp
	g++ -c client_test.cpp -DTORRENT_USE_OPENSSL -DWITH_SHIPPED_GEOIP_H -DBOOST_ASIO_HASH_MAP_BUCKETS=1021 -DBOOST_EXCEPTION_DISABLE -DBOOST_ASIO_ENABLE_CANCELIO -DBOOST_ASIO_DYN_LINK -DTORRENT_LINKING_SHARED -I/usr/local/include -I/usr/local/include/libtorrent

make_torrent: make_torrent.o
	g++ make_torrent.o -o make_torrent -L/usr/local/lib/ -L/usr/lib/ -ltorrent-rasterbar -lboost_system-mt

make_torrent.o: make_torrent.cpp
	g++ -c make_torrent.cpp -DTORRENT_USE_OPENSSL -DWITH_SHIPPED_GEOIP_H -DBOOST_ASIO_HASH_MAP_BUCKETS=1021 -DBOOST_EXCEPTION_DISABLE -DBOOST_ASIO_ENABLE_CANCELIO -DBOOST_ASIO_DYN_LINK -DTORRENT_LINKING_SHARED -I/usr/local/include -I/usr/local/include/libtorrent

dump_torrent: dump_torrent.o
	g++ dump_torrent.o -o dump_torrent -L/usr/local/lib/ -L/usr/lib/ -ltorrent-rasterbar -lboost_system-mt

dump_torrent.o: dump_torrent.cpp
	g++ -c dump_torrent.cpp -DTORRENT_USE_OPENSSL -DWITH_SHIPPED_GEOIP_H -DBOOST_ASIO_HASH_MAP_BUCKETS=1021 -DBOOST_EXCEPTION_DISABLE -DBOOST_ASIO_ENABLE_CANCELIO -DBOOST_ASIO_DYN_LINK -DTORRENT_LINKING_SHARED -I/usr/local/include -I/usr/local/include/libtorrent

clean:
	rm *.o *.a simple_client client_test make_torrent dump_torrent simple_app uc2
