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

/*  Several global variables and their default values. */
// Download limit: KBps
int download_rate=0;
// Upload limit: KBps
int upload_rate=0;
// Shared uploads
std::string shared_dir = "/tmp/freeCycles-wrapper";
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

/**
 * This is application specific code. It should be isolated as possible.
 * Only one convention remains: all input and output files must be read and
 * written inside input_dir and output_dir respectively.
 *
 * input_file and output_file are done accordingly to the work_generator.
 */
int upper_case(std::string input_dir, std::string output_dir) {
	std::string input_file = input_dir + "/input";
	std::string output_file = output_dir + "/output";
    FILE* out;
    FILE* in;
    char c;

    in = fopen(input_file.c_str(), "r");
    if (!in) {
        fprintf(stderr,
            "%s [APP] Couldn't find input file, resolved name %s.\n",
            boinc_msg_prefix(buf, sizeof(buf)), input_file.c_str());
        return 1;
    }

    out = fopen(output_file.c_str(), "wb");
    if (!out) {
    	fprintf(stderr, "%s [APP]: upper_case output open failed:\n",
    			boinc_msg_prefix(buf, sizeof(buf)));
    	fprintf(stderr, "%s resolved name %s\n",
    			boinc_msg_prefix(buf, sizeof(buf)),
    			output_file.c_str());
    	perror("open");
    	return 1;
    }

    for (int i=0; ; i++) {
        c = fgetc(in);
        if (c == EOF) break;
        fputc(toupper(c), out);
    }

    fclose(in);
    fclose(out);
    return 0;
}

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

// Blocking function that holds execution while input is not ready.
void wait_torrent(libtorrent::torrent_handle input)
{ while(!input.status().is_seeding) { sleep(1); } }

int main(int argc, char **argv) {
	char input_path[512], output_path[512];
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
    boinc_resolve_filename(BOINC_INPUT_FILENAME, input_path, sizeof(input_path));
    boinc_resolve_filename(BOINC_OUTPUT_FILENAME, output_path, sizeof(output_path));
    // Resolve WU name.
    boinc_get_init_data(boinc_data);
    wu_name = std::string(boinc_data.wu_name);
#else
    strcpy(input_path, "/tmp/boinc-slot/");
    strcpy(output_path, "/tmp/boinc-slot/");
    wu_name = "freeCycles-wrapper_321_123";
#endif

    *bth = BitTorrentHandler(std::string(input_path), std::string(output_path));
    bth->init(download_rate, upload_rate, shared_dir, tracker_url);

    // map task
    if(wu_name.find("map") != std::string::npos) {
    	*tt = MapTracker(bth->get_input(), wu_name+"-", nmaps, nreds);
        tt->map(wc_map);
        bth->stage_output(tt->getOutputs().front());
    }
    // reduce task
    else {
    	*tt = ReduceTracker(bth->get_zipped_input(), wu_name, nmaps, nreds);
        tt->reduce(wc_reduce);
        bth->stage_zipped_output(tt->getOutputs());
    }

    // TODO - while:
	// 1- application is running
	// 2- we are waiting for the input files
	// 3- we are sleeping (in the end)
	// -> search for new .torrent files.
    // TODO - application should be run in a separate thread.


    // Add torrent and wait until completion
    // TODO - do it inside get_input.
    // Copy input .torrent file to shared dir.
	// TODO - this must be done inside get_intput
    //copy_file(input_path, input_torrent);
    // Call application.
    // FIXME - application returns a string vector (output files).
    //if (upper_case(input_dir, output_dir)) { return 1; }
    // Depending on the type of task, call output (zipped or not).
    // FIXME - make_torrent will be called for every output -> done in stage_output
    // make_torrent(output_dir, output_torrent, tracker_url);
    // FIXME - Copy output .torrent file to the right place (BOINC expected location). -> done in stage_output
    //copy_file(output_torrent, output_path);
    // Start sharing output. -> done in stage output
    //add_torrent(output_path);

    sleep(60);

#ifndef STANDALONE
    boinc_fraction_done(1);
    boinc_finish(0);
#endif
}
