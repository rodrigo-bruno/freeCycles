// TODO - place open source banner

#include "config.h"
#include <cstdio>
#include <cctype>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <csignal>
#include <unistd.h>

#include "str_util.h"
#include "util.h"
#include "filesys.h"
#include "boinc_api.h"
#include "mfile.h"

#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/session.hpp"

// TODO - usage
//        "Usage: %s [OPTION]...\n"
//        "Options:\n"
//        "  [ -d X ]   Download rate limit (KBps)\n"
//        "  [ -u Y ]   Upload rate limit (KBps)\n"
//        "  [ -s   ]   FIXME \n",
//        "  [ -p   ]   FIXME.\n",


// TODO - use std::string OR char*
// TODO - test if it is okay to remove this. I like explicit stuff.
using std::string;

// BOINC template names for input and output files.
#define BOINC_INPUT_FILENAME "in"
#define BOINC_OUTPUT_FILENAME "out"

#define BT_SEARCH_INTERVAL 1
#define BT_FILE_SUFFIX ".torrent"

// Several global variables. Just to facilitate method calling.
libtorrent::session_settings bt_settings;
libtorrent::session bt_session;
libtorrent::error_code bt_ec;

int retval;
char buf[256];

// Default values for user definable variables.
//
// Private downloads
std::string private_dir = "./";
// Shared uploads
std::string shared_dir = "./"; // FIXME - I will probably have to create this dir.


/**
 * Adds a new torrent to the current session. Possible errors are reported
 * through the error_code object.
 */
libtorrent::torrent_handle add_torrent(const char* torrent) {

	libtorrent::add_torrent_params p;
	libtorrent::torrent_handle th;

	p.save_path = private_dir;
	p.ti = new libtorrent::torrent_info(torrent, bt_ec);
	if (bt_ec)	{ return th; }
	th = bt_session.add_torrent(p, bt_ec);
	if (bt_ec)	{ return th; }

	return th;
}


/**
 * Reads char, converts to upper case, writes char.
 */
void upper_case(FILE* in, MFILE out) {
	char c;
    for (int i=0; ; i++) {
        c = fgetc(in);
        if (c == EOF) break;
        c = toupper(c);
        out._putchar(c);
    }
}

void process_cmd_args(int argc, char** argv) {

	/* Setup bt settings */
    bt_settings = libtorrent::high_performance_seed();
    bt_settings.allow_multiple_connections_per_ip = true;
    bt_settings.active_downloads = -1; // unlimited

	/* Command line processing */
	for(int arg_index = 1; arg_index < argc; arg_index++) {
		if (!strcmp(argv[arg_index], "-d"))	{
			/* download limit: KBps */
			bt_settings.download_rate_limit = atoi(argv[++arg_index]) * 1000;
		}
		else if (!strcmp(argv[arg_index], "-u"))	{
			/* upload limit: KBps */
			bt_settings.upload_rate_limit = atoi(argv[++arg_index]) * 1000;
		}
		else if (!strcmp(argv[arg_index], "-s"))	{
			shared_dir = argv[++arg_index];
		}
		else if (!strcmp(argv[arg_index], "-p"))	{
			private_dir = argv[++arg_index];
		}
		else {
	        fprintf(stderr,
	        		"%s [APP] unknown cmd arg %s\n",
	        		boinc_msg_prefix(buf, sizeof(buf)),
	        		argv[arg_index]);
	        exit(1);
		}
	}
}

void wait_for_input() {}

/**
 * TODO:
 * 2 - get input file, load it in libtorrent
 * 3 - one its all done, perform computation
 * 4 - create .torrent for the output
 * 5 - mv input and output files, and copy .torrent files (input and output) to a shared dir (with other WUs). 
 */
int main(int argc, char **argv) {
    MFILE out;
    FILE* in;
    char input_path[512], output_path[512];

    process_cmd_args(argc, argv);

    // init boinc
    //
    retval = boinc_init();
    if (retval) {
        fprintf(stderr,
        		"%s [BOINC] boinc_init returned %d\n",
        		boinc_msg_prefix(buf, sizeof(buf)), retval);
        exit(retval);
    }

	/* init bt session */
    bt_session.set_settings(bt_settings);
    bt_session.listen_on(std::make_pair(6500, 7000), bt_ec);
	if (bt_ec)	{
        fprintf(stderr,
                "%s [BT] Failed to open listen socket: %s\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                input_path, bt_ec.message().c_str());
		return 1;
	}

    // resolve input file (.torrent file) logical name
    boinc_resolve_filename(BOINC_INPUT_FILENAME, input_path, sizeof(input_path));

    add_torrent(input_path);
    // TODO once all files are downloaded
    // TODO issue (fork) cp downloaded+.torrent to shared
    upper_case(in, out);
    // TODO create .torrent file for output files
    // TODO state .torrent file
    // TODO mv output+.torrent to shared


    // open the output file (resolve logical name first)
    // FIXME
    boinc_resolve_filename(BOINC_OUTPUT_FILENAME, output_path, sizeof(output_path));
    retval = out.open(output_path, "wb");
    if (retval) {
        fprintf(stderr, "%s APP: upper_case output open failed:\n",
            boinc_msg_prefix(buf, sizeof(buf))
        );
        fprintf(stderr, "%s resolved name %s, retval %d\n",
            boinc_msg_prefix(buf, sizeof(buf)), output_path, retval
        );
        perror("open");
        exit(1);
    }

    // flush output file.
    //
    retval = out.flush();
    if (retval) {
        fprintf(stderr,
        		"%s APP: upper_case flush failed %d\n",
                boinc_msg_prefix(buf, sizeof(buf)), retval);
        exit(1);
    }

    boinc_fraction_done(1);
    boinc_finish(0);
}
