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
 * Usage: freeCycles-wrapper [-d D] [-u U] [-s S] [-t T] --app app_name [app arguments]
 * Options:
 *  -d    Download rate limit (KBps)
 *  -u    Upload rate limit (KBps)
 *  -s    Location for the shared directory
 *  -t    Tracker to use for peer discovery
 *  --app From here on, the app_name is read and all the remaining arguments
 *  are sent in the "app_name" call.
 *
 */

// BOINC template names for input and output files.
#define BOINC_INPUT_FILENAME "in"
#define BOINC_OUTPUT_FILENAME "out"

// Several global variables. Just to facilitate method calling.
libtorrent::session_settings bt_settings;
libtorrent::session bt_session;
libtorrent::error_code bt_ec;
char buf[256];

// Shared uploads
std::string shared_dir = "/tmp/freeCycles-wrapper";
// WU name
std::string wu_name;
// Tracker url to use.
std::string tracker_url = "udp://boinc.rnl.ist.utl.pt:6969";

// Adds a new torrent to the current session.
libtorrent::torrent_handle add_torrent(const char* torrent) {
	libtorrent::add_torrent_params p;
	p.save_path = shared_dir;
	p.ti = new libtorrent::torrent_info(torrent, bt_ec);
	return bt_session.add_torrent(p, bt_ec);
}

// Returns all the file names, within a specified location "search_dir",
// which are prefixed by "prefix".
int search_prefix(
		const std::string& search_dir,
		const std::string& prefix,
		std::vector<std::string>* results) {
	DIR* dir;

	// Try to open search_dir. If opendir is unsuccessful, return.
	if((dir = opendir(search_dir.c_str())) == NULL) {
		fprintf(stderr,
				"%s [APP] cannot open dir %s\n",
	      		boinc_msg_prefix(buf, sizeof(buf)),
	      		search_dir.c_str());
		return -1;
	}

	// Look for all files and and check if prefix applies.
	for(struct dirent* dp = readdir(dir); dp != NULL; dp = readdir(dir)) {

		if(dp->d_type == DT_DIR) { continue; }

		std::string file_name = std::string(dp->d_name);
		if(starts_with(file_name, prefix)) {
			results->push_back(file_name);
		}
	}
	return 0;
}

// Copies one file from source to destination.
int copy_file(const std::string& source, const std::string& dest) {
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
      fprintf(stderr,
      		  "%s [APP] can't fork to perform cp %s %s\n",
      		  boinc_msg_prefix(buf, sizeof(buf)),
      		  source.c_str(),
      		  dest.c_str());
    return -1;
  }
  else {
    pid_t ws = waitpid( pid, &childExitStatus, 0);
    if (ws == -1) {
        fprintf(stderr,
        		  "%s [APP] cannot wait for child process (cp %s %s)\n",
        		  boinc_msg_prefix(buf, sizeof(buf)),
        		  source.c_str(),
        		  dest.c_str());
      return -1;
    }
    if (WIFSIGNALED(childExitStatus)) { /* killed */
        fprintf(stderr,
        		  "%s [APP] signaled child process (cp %s %s)\n",
        		  boinc_msg_prefix(buf, sizeof(buf)),
        		  source.c_str(),
        		  dest.c_str());
      return -1;
    }
    else if (WIFSTOPPED(childExitStatus)) { /* stopped */
        fprintf(stderr,
        		  "%s [APP] stopped child process (cp %s %s)\n",
        		  boinc_msg_prefix(buf, sizeof(buf)),
        		  source.c_str(),
        		  dest.c_str());
      return -1;
    }
    return 0;
  }
}

// Initializes (creates) a directory.
int init_dir(std::string dir) {
	int status = mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	if(status == 0 || status == EEXIST) { return 0; }
	if(status == EACCES) {
        fprintf(stderr,
        		  "%s [APP] failed to create dir %s. Permission denied.\n",
        		  boinc_msg_prefix(buf, sizeof(buf)),
        		  dir.c_str());
	}
	else {
        fprintf(stderr,
        		  "%s [APP] failed to create dir %s. Error code %d.\n",
        		  boinc_msg_prefix(buf, sizeof(buf)),
        		  dir.c_str(),
        		  status);
	}
	return status;
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

// Processes command line arguments.
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
		else if (!strcmp(argv[arg_index], "-t"))	{
			tracker_url = argv[++arg_index];
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

void make_torrent(
		const std::vector<std::string>& input_files,
		std::string output_file,
		std::string tracker_url) {
	std::string creator_str = "freeCycles-wrapper (using libtorrent)";
	libtorrent::file_storage fs;

	for(std::vector<std::string>::iterator it = input_files.begin(); it != input_files.end(); ++it) {
	    *it->assign(libtorrent::complete(*it));
	}


	// TODO - implement.
}

int main(int argc, char **argv) {
	int retval;
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
    init_dir(shared_dir);
    bt_session.set_settings(bt_settings);
    bt_session.listen_on(std::make_pair(6500, 7000), bt_ec);
	if (bt_ec)	{
        fprintf(stderr,
                "%s [BT] Failed to open listen socket: %s\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                input_path, bt_ec.message().c_str());
		return 1;
	}

    // TODO - while:
	// 1- application is running
	// 2- we are waiting for the input files
	// -> search for new .torrent files.

    // Add torrent and wait until completion
    wait_torrent(add_torrent(input_path));
    // Copy input .torrent file to shared dir.
    copy_file(input_path, shared_dir+"/"+wu_name+".input.torrent");
    // TODO - application should be run in a separate process.
    // Call application.
    upper_case(shared_dir.c_str(), wu_name.c_str());
    // Search for application output files.
    search_prefix(shared_dir, wu_name, &app_outputs);
    // Create .torrent file for output files.
    make_torrent(app_outputs, tracker_url, shared_dir+"/"+wu_name+".output.torrent");
    // Copy output .torrent file to the right place (BOINC expected location).
    copy_file(shared_dir+"/"+wu_name+".output.torrent", output_path);


    boinc_fraction_done(1);
    boinc_finish(0);
}
