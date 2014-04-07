// TODO - place open source banner

#include "config.h"
#include <cstdio>
#include <cctype>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <csignal>
#include <unistd.h>
#include <wait.h>

#include "str_util.h"
#include "util.h"
#include "filesys.h"
#include "boinc_api.h"
#include "mfile.h"

#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/session.hpp"

/*
 * Usage: app [-d X] [-u Y] [-s] [-p]
 * Options:
 *  -d Download rate limit (KBps)
 *  -u Upload rate limit (KBps)
 *  -s Location for the shared directory
 *
 */


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
// Shared uploads
std::string shared_dir = "./";
// WU name
std::string wu_name;

/**
 * Adds a new torrent to the current session.
 */
libtorrent::torrent_handle add_torrent(const char* torrent) {

	libtorrent::add_torrent_params p;

	p.save_path = shared_dir;
	p.ti = new libtorrent::torrent_info(torrent, bt_ec);
	return bt_session.add_torrent(p, bt_ec);
}

void search_prefix(const std::string& dir, const std::string& prefix, const std::vector<std::string>& vector) {
	// TODO
}

// Copies one file from source to destination.
//
int copy_file(const std::string source, const std::string dest) {
  // TODO - try to implement this without creating a new process.
  // It seems to me like an excessive overhead...
  // I will use this function to copy only a few thousands of bytes.
  // See util.h
  pid_t pid;
  int childExitStatus;
  pid = fork();

  if(pid == 0) {
    return execl("/bin/cp", "/bin/cp", source.c_str(), dest.c_str(), (char *)0);
  }
  else if (pid < 0) {
    log_messages.printf(MSG_CRITICAL, "can't fork to perform cp (input staging)\n");
    return -1;
  }
  else {
    pid_t ws = waitpid( pid, &childExitStatus, 0);
    if (ws == -1) {
      log_messages.printf(MSG_CRITICAL, "can't wait for child (input staging)\n");
      return -2;
    }
    if (WIFSIGNALED(childExitStatus)) { /* killed */
      log_messages.printf(MSG_CRITICAL, "signaled child (input staging)\n");
      return -3;
    }
    else if (WIFSTOPPED(childExitStatus)) { /* stopped */
      log_messages.printf(MSG_CRITICAL, "stopped child (input staging)\n");
      return -4;
    }
    return 0;
  }
}

void init_shared_dir() {
	// TODO - check if dir exists, create if not
}

/**
 * This is application specific code. It should be isolated as possible.
 * Only one convention remains: all input and output files must be read and
 * written inside work_dir and use names prefixed with wu_name.
 * Reads char, converts to upper case, writes char.
 */
void upper_case(const char* work_dir, const char* wu_name) {
    MFILE out; // FIXME - proper file opening and closing
    FILE* in;
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
		else {
	        fprintf(stderr,
	        		"%s [APP] unknown cmd arg %s\n",
	        		boinc_msg_prefix(buf, sizeof(buf)),
	        		argv[arg_index]);
	        exit(1);
		}
	}
}

// Blocking function that holds execution while input is not ready.
//
void wait_torrent(libtorrent::torrent_handle input) {
	while(!input.status().is_seeding) { sleep(1); }
}

int main(int argc, char **argv) {

    char input_path[512], output_path[512];
    APP_INIT_DATA boinc_data;
    std::vector<std::string> app_outputs;

    process_cmd_args(argc, argv);

    // Initialize BOINC.
    retval = boinc_init();
    if (retval) {
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

	// Initialize BitTorrent session.
    bt_session.set_settings(bt_settings);
    bt_session.listen_on(std::make_pair(6500, 7000), bt_ec);
	if (bt_ec)	{
        fprintf(stderr,
                "%s [BT] Failed to open listen socket: %s\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                input_path, bt_ec.message().c_str());
		return 1;
	}


    // Add torrent and wait until completion
    wait_torrent(add_torrent(input_path));
    // Copy input .torrent file to shared dir ($(shared_dir)/$(wu_name).input.torrent
    copy_file(std::string(input_path), shared_dir+"/"+wu_name+".input.torrent");
    // Call application.
    upper_case(shared_dir.c_str(), wu_name.c_str());
    // Search for application output files.
    search_prefix(shared_dir, wu_name, app_outputs);
    // TODO make_torrent $(app_outputs) -t udp://boinc.rnl.ist.utl.pt:6969 -o $(shared_dir)/$(wu_name).output.torrent
    // Copy output .torrent file to the right place (BOINC expected location).
    copy_file(shared_dir+"/"+wu_name+".output.torrent", std::string(output_path));


    boinc_fraction_done(1);
    boinc_finish(0);
}
