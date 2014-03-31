/*

Copyright (c) 2003, Arvid Norberg
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the distribution.
    * Neither the name of the author nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*/

#include <stdlib.h>
#include <unistd.h>
#include <queue>
#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/session.hpp"

#define UPDATE_INTERVAL 5
#define TORRENT_FILE_SUFFIX ".torrent"


/**
 * Tests if base ends with suffix.
 */
bool ends_with (const char* base, const char* suffix) {
    int blen = strlen(base);
    int slen = strlen(suffix);
    return (blen >= slen) && (!strcmp(base + blen - slen, suffix));
}

/**
 * Adds a new torrent to the current session. Possible errors are reported
 * through the error_code object.
 */
libtorrent::torrent_handle add_torrent(
		libtorrent::session &s,
		const char* torrent,
		libtorrent::error_code &ec,
		const char* work_dir) {

	libtorrent::add_torrent_params p;
	libtorrent::torrent_handle th;

	p.save_path = work_dir;
	p.ti = new libtorrent::torrent_info(torrent, ec);
	if (ec)	{ return th; }
	th = s.add_torrent(p, ec);
	if (ec)	{ return th; }

	return th;
}

/**
 * Searches for torrent files inside a particular directory.
 * Found torrents will be added to the given session and will be moved to
 * the work directory.
 */
void check_new_torrent(
		libtorrent::session &s,
		libtorrent::error_code &ec,
		const std::string search_dir,
		const std::string work_dir) {
	std::string torrent;
	std::queue<std::string*> delete_queue;
	DIR* dir;

	// try to open dir. If opendir is unsuccessful, return.
	if((dir = opendir(search_dir.c_str())) == NULL) { return; }
	// look for all files and try to add them as new torrents.
	for(struct dirent* dp = readdir(dir); dp != NULL; dp = readdir(dir)) {

		if(dp->d_type == DT_DIR) { continue; }
		if(ends_with(dp->d_name, TORRENT_FILE_SUFFIX)) { continue; }

		add_torrent(s, (search_dir+dp->d_name).c_str(), ec, work_dir.c_str());

		if (ec)	{ fprintf(stderr, "%s\n", ec.message().c_str()); }
		else {
			fprintf(stderr, "Torrent added: %s\n", dp->d_name);
			delete_queue.push(new std::string(dp->d_name));
		}
	}

	// move all successfully added torrents to work directory.
	while(!delete_queue.empty()) {
		std::string name = *delete_queue.front();
		std::string work_name = work_dir + name;
		std::string search_name = search_dir + name;
		rename(search_name.c_str(), work_name.c_str());
		delete_queue.pop();
	}

	closedir(dir);
}

void usage(char *name) {
    fprintf(stderr, "This is a simple command line BitTorrent client.\n"
        "Usage: %s [OPTION]...\n"
        "Options:\n"
        "  [ -d X ]   Download rate limit (KBps)\n"
        "  [ -u Y ]   Upload rate limit (KBps)\n"
        "  [ -h   ]   Shows this help text.\n",
        "  [ -s   ]   Directory to search periodically for .torrents\n",
        "  [ -w   ]   Directory used to save .torrents and downloaded files.\n",
        name
    );
}

/**
 * TODO - add way to launch processes once a specific file is downloaded.
 */

int main(int argc, char* argv[]) {
	using namespace libtorrent;

	session s;
	error_code ec;
	session_settings settings;
	std::string search_dir = "./";
        std::string work_dir = "./";

	/* Init session */
	s.listen_on(std::make_pair(6500, 7000), ec);
	if (ec)	{
		fprintf(stderr, "failed to open listen socket: %s\n", ec.message().c_str());
		return 1;
	}

	/* Settings */
	settings = high_performance_seed();
	settings.allow_multiple_connections_per_ip = true;
	settings.active_downloads = -1; // unlimited

	/* Command line processing */
	for(int arg_index = 1; arg_index < argc; arg_index++) {
		if (!strcmp(argv[arg_index], "-d"))	{
			/* download limit: KBps */
			settings.download_rate_limit = atoi(argv[++arg_index]) * 1000;
			fprintf(stderr, "max download rate = %dKBps\n", settings.download_rate_limit);
		}
		else if (!strcmp(argv[arg_index], "-u"))	{
			/* upload limit: KBps */
			settings.upload_rate_limit = atoi(argv[++arg_index]) * 1000;
			fprintf(stderr, "max upload rate = %dKBps\n", settings.upload_rate_limit);
		}
		else if (!strcmp(argv[arg_index], "-h"))	{
			usage(argv[0]);
			exit(0);
		}
		else if (!strcmp(argv[arg_index], "-s"))	{
			search_dir = argv[++arg_index];
			fprintf(stderr, "search dir = %s\n", search_dir.c_str());
		}
		else if (!strcmp(argv[arg_index], "-w"))	{
			work_dir = argv[++arg_index];
			fprintf(stderr, "work dir = %s\n", work_dir.c_str());
		}
		else {
			usage(argv[0]);
			exit(1);
		}
	}

	s.set_settings(settings);

	/* Periodically check for new torrents */
	unsigned int time_left;
	fprintf(stderr, "Just started simple bt client.\n");
        check_new_torrent(s, ec, work_dir, work_dir);
	while(true) {
		time_left = UPDATE_INTERVAL;
		check_new_torrent(s, ec, search_dir, work_dir);
		while(time_left) { time_left = sleep(time_left); }
	}
	return 0;
}

