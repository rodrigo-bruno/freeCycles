// TODO - place open source banner

// Compilation flag to enable local testing.
//#define STANDALONE

#include "config.h"
#include <cstdio>
#include <cctype>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <csignal>
#include <unistd.h>
#include <wait.h>

#ifndef STANDALONE
#include "str_util.h"
#include "util.h"
#include "filesys.h"
#include "boinc_api.h"
#include "mfile.h"
#endif

#include "mr_tasktracker.h"
#include "data_handler.h"

/*
 * Usage: freeCycles-wrapper [-d D] [-u U] [-s S] [-t T] -map M -red R
 * Options:
 *  -d    Download rate limit (KBps)
 *  -u    Upload rate limit (KBps)
 *  -s    Location for the shared directory
 *  -t    Tracker to use for peer discovery
 *  -map  Number of mappers
 *  -red  Number of reducers
 */

// BOINC template names for input and output files.
#define BOINC_INPUT_FILENAME "in"
#define BOINC_OUTPUT_FILENAME "out"

/*  Several global variables and their default values. */
// Download limit: KBps
int download_rate=0;
// Upload limit: KBps
int upload_rate=0;
// Shared uploads.
std::string shared_dir = "/tmp/freeCycles-shared";
// Private downloads.
std::string working_dir;
// Tracker URL to use.
#ifdef STANDALONE
std::string tracker_url = "udp://localhost:6969";
#else
std::string tracker_url = "udp://boinc.rnl.ist.utl.pt:6969";
#endif
char buf[256];
// WU name
std::string wu_name;
// Number of mappers
int nmaps;
// Number of reducers
int nreds;

#ifdef STANDALONE
char* boinc_msg_prefix(char* buf, int size) {
	return strcpy(buf, "[STANDALONE]");
}
#endif

/*
 * Command line processing.
 * This reads the command line arguments and saves them in global variables.
 */
void process_cmd_args(int argc, char** argv) {
	for(int arg_index = 1; arg_index < argc; arg_index++) {
		if (!strcmp(argv[arg_index], "-d"))
		{ download_rate = atoi(argv[++arg_index]) * 1000; }
		else if (!strcmp(argv[arg_index], "-u"))
		{ upload_rate = atoi(argv[++arg_index]) * 1000; }
		else if (!strcmp(argv[arg_index], "-s"))
		{ shared_dir = argv[++arg_index]; }
		else if (!strcmp(argv[arg_index], "-t"))
		{ tracker_url = argv[++arg_index]; }
		else if (!strcmp(argv[arg_index], "-map"))
		{ nmaps = atoi(argv[++arg_index]); }
		else if (!strcmp(argv[arg_index], "-red"))
		{ nreds = atoi(argv[++arg_index]); }
		else {
	        fprintf(stderr,
	        		"%s [WRAPPER-process_cmd_args] unknown cmd arg %s\n",
	        		boinc_msg_prefix(buf, sizeof(buf)),
	        		argv[arg_index]);
	        exit(1);
		}
	}
}

int main(int argc, char **argv) {
	std::string input_path, output_path;
	BitTorrentHandler* bth = NULL;
	TaskTracker* tt = NULL;
#ifndef STANDALONE
    APP_INIT_DATA boinc_data;
    int retval;
#endif

    process_cmd_args(argc, argv);

#ifndef STANDALONE
    // Initialize BOINC.
    if ((retval = boinc_init())) {
        fprintf(stderr,
        		"%s [BOINC] boinc_init returned %d\n",
        		boinc_msg_prefix(buf, sizeof(buf)), retval);
        exit(retval);
    }
    // Resolve input and output files' logical name (.torrent files).
    boinc_resolve_filename_s(BOINC_INPUT_FILENAME, input_path);
    boinc_resolve_filename_s(BOINC_OUTPUT_FILENAME, output_path);
    // Resolve WU name.
    boinc_get_init_data(boinc_data);
    wu_name = std::string(boinc_data.wu_name);
#else
    input_path = "/tmp/boinc-slot/";
    output_path = "/tmp/boinc-slot/";
    wu_name = "freeCycles-wrapper_321_123";
#endif

    working_dir = std::string("/tmp/") + wu_name + "/";

    *bth = BitTorrentHandler(
    		input_path, output_path, working_dir, shared_dir, tracker_url);
    bth->init(download_rate, upload_rate);

    // map task
    if(wu_name.find("map") != std::string::npos) {
    	*tt = MapTracker(bth, wu_name+"-", nmaps, nreds);
        tt->map(wc_map);
        bth->stage_output(tt->getOutputs()->front());
    }
    // reduce task
    else {
    	*tt = ReduceTracker(bth, wu_name, nmaps, nreds);
        tt->reduce(wc_reduce);
        bth->stage_zipped_output(*(tt->getOutputs()));
    }

    // TODO - while:
	// 1- application is running
	// 2- we are waiting for the input files
	// 3- we are sleeping (in the end)
	// -> search for new .torrent files.
    // TODO - application should be run in a separate thread.
    sleep(60);

#ifndef STANDALONE
    boinc_fraction_done(1);
    boinc_finish(0);
#endif
}
