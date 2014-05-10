// TODO - place open source banner

#include "control.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#if not STADALONE
#include "config.h"
#include "boinc_api.h"
#include "util.h"
#endif

#include "mr_tasktracker.h"
#include "data_handler.h"
#include "benchmarks.h"

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
// Download limit: KBps (default = 0, unlimited)
int download_rate=0;
// Upload limit: KBps (default = 0, unlimited)
int upload_rate=0;
// Shared uploads.
std::string shared_dir = "/tmp/freeCycles-shared/";
// Private downloads.
std::string working_dir;
// Tracker URL to use.
#if STANDALONE
std::string tracker_url = "udp://localhost:6969";
#else
std::string tracker_url = "udp://boinc.rnl.ist.utl.pt:6969";
#endif
char buf[256];
// WU name
std::string wu_name;
// Number of mappers
int nmaps = 0;
// Number of reducers
int nreds = 0;

/*
 * Command line processing.
 * This reads the command line arguments and saves them in global variables.
 */
int process_cmd_args(int argc, char** argv) {
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
	        return 1;
		}
	}
	// Check for mandatory args.
    if(!nmaps || !nreds) {
        fprintf(stderr,
        		"%s [WRAPPER-process_cmd_args] nreds or nmaps set to zero\n",
        		boinc_msg_prefix(buf, sizeof(buf)));
        return 1;
    }
    return 0;
}

int main(int argc, char **argv) {
	std::string input_path, output_path;
#if BITTORRENT
	BitTorrentHandler* dh = NULL;
#else
	DataHandler* dh = NULL;
#endif
	TaskTracker* tt = NULL;
	int retval = 0;
	pid_t pid;
#if not STANDALONE
    APP_INIT_DATA boinc_data;
#endif

#if BITTORRENT
    // Ignore SIGTERM (I use this to keep a child process running for a bit
    // longer after the parent calls boinc_finish).
    if (signal(SIGTERM, SIG_IGN) == SIG_ERR) {
    	fprintf(stderr,
    			"%s [WRAPPER-main] failed to setup SIGTERM handler.\n",
    	        boinc_msg_prefix(buf, sizeof(buf)));
    }
#endif

    if((retval = process_cmd_args(argc, argv))) { goto fail; }

#if not STANDALONE
    // Initialize BOINC.
    if ((retval = boinc_init())) {
        fprintf(stderr,
        		"%s [WRAPPER-main] boinc_init returned %d\n",
        		boinc_msg_prefix(buf, sizeof(buf)), retval);
        goto fail;
    }
    // Resolve input and output files' logical name (.torrent files).
    boinc_resolve_filename_s(BOINC_INPUT_FILENAME, input_path);
    boinc_resolve_filename_s(BOINC_OUTPUT_FILENAME, output_path);
    // Resolve WU name.
    boinc_get_init_data(boinc_data);
    wu_name = std::string(boinc_data.wu_name);
#else
    init_dir("/tmp/freeCycles-boinc-slot/");


/** Test 1
    // BT
    //input_path = "/tmp/freeCycles-boinc-slot/freeCycles-map-0.torrent";
    // BOINC
    input_path = "/tmp/freeCycles-boinc-slot/freeCycles-map-0";
    output_path = "/tmp/freeCycles-boinc-slot/freeCycles-map-0.zip";
    wu_name = "freeCycles-map-0";
*/

/** Teste 2
    input_path = "/tmp/freeCycles-boinc-slot/freeCycles-reduce-0.zip";
    // BT
    //output_path = "/tmp/freeCycles-boinc-slot/freeCycles-reduce-0.torrent";
    // BOINC
    output_path = "/tmp/freeCycles-boinc-slot/freeCycles-reduce-0";
    wu_name = "freeCycles-reduce-0";
*/
#endif

    working_dir = std::string("/tmp/") + wu_name + "/";

#if BITTORRENT
    dh = new BitTorrentHandler(
    		input_path,	output_path,
    		working_dir, shared_dir,
    		tracker_url, wu_name);
    dh->init(download_rate, upload_rate);
#else
    dh = new DataHandler(input_path, output_path, working_dir);
#endif

#if DEBUG
    	debug_log("[WRAPPER-main]", "running task:", wu_name.c_str());
    	debug_log("[WRAPPER-main]", "input to download:", input_path.c_str());
    	debug_log("[WRAPPER-main]", "output to upload:", output_path.c_str());
#endif

    // map task
    if(wu_name.find("map") != std::string::npos) {
#if BITTORRENT
    	tt = new MapTracker(dh, shared_dir + wu_name+"-", nmaps, nreds);
#else
    	tt = new MapTracker(dh, working_dir + wu_name+"-", nmaps, nreds);
#endif
#if DEBUG
    	debug_log("[WRAPPER-main]", "input downloaded.", "");
#endif
    	retval = 32768;
        tt->map(wc_map, &retval);
        retval = 0;
#if DEBUG
    	debug_log("[WRAPPER-main]", "map done.", "");
#endif
    	dh->stage_zipped_output(*(tt->getOutputs()));
    }
    // reduce task
    else if (wu_name.find("reduce") != std::string::npos){
#if BITTORRENT
    	tt = new ReduceTracker(dh, shared_dir + wu_name, nmaps, nreds);
#else
    	tt = new ReduceTracker(dh, working_dir + wu_name, nmaps, nreds);
#endif
        tt->reduce(wc_reduce);
        dh->stage_output(tt->getOutputs()->front());
    }
    // unknown task
    else {
        fprintf(stderr,
        		"%s [WRAPPER-main] task is not map nor reduce: %s\n",
        		boinc_msg_prefix(buf, sizeof(buf)),
        		wu_name.c_str());
        retval = 1;
        goto fail;
    }

    // TODO - while:
	// 1- application is running
	// 2- we are waiting for the input files
	// 3- we are sleeping (in the end)
	// -> search for new .torrent files.

	delete tt;
    delete dh;

    // Hack: process is cloned. Parent calls boinc_finish and the child keeps
    // running a BitTorrent client for some time.
#if BITTORRENT
    if(!(pid = fork())) {

    	// Child.
    	dh = new BitTorrentHandler(
        		input_path, output_path,
        		working_dir, shared_dir,
        		tracker_url, wu_name);
    	dh->init(download_rate, upload_rate);
    	boinc_sleep(120);
    	delete dh;
    	exit(retval);
    }
#else
    if(0) {}
#endif
    else {
    	// Parent.
#if not STANDALONE
        boinc_fraction_done(1);
        boinc_finish(retval);
#else
        exit(retval);
#endif
    }
    // This piece of code is never reached by normal execution (only gotos are
    // able to jump here.
fail:
	delete tt;
	delete dh;
	fprintf(stderr,
			"%s [WRAPPER-main] Failing.\n",
			boinc_msg_prefix(buf, sizeof(buf)));
	exit(retval);
}
