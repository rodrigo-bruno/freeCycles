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
#include "libtorrent/create_torrent.hpp"

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
// Input torrent $(shared_dir)/$(wu_name).input.torrent
std::string input_torrent;
// Input folder $(shared_dir)/$(wu_name).input
std::string input_dir;
// Output torrent $(shared_dir)/$(wu_name).output.torrent
std::string output_torrent;
// Output folder $(shared_dir)/$(wu_name).output
std::string output_dir;
// WU name
std::string wu_name;
// Tracker URL to use.
std::string tracker_url = "udp://boinc.rnl.ist.utl.pt:6969";

// Adds a new torrent to the current session.
libtorrent::torrent_handle add_torrent(const char* torrent) {
	libtorrent::add_torrent_params p;
	p.save_path = shared_dir;
	p.ti = new libtorrent::torrent_info(torrent, bt_ec);
	return bt_session.add_torrent(p, bt_ec);
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
      		  "%s [WRAPPER-copy_file] can't fork to perform cp %s %s\n",
      		  boinc_msg_prefix(buf, sizeof(buf)),
      		  source.c_str(),
      		  dest.c_str());
    return -1;
  }
  else {
    pid_t ws = waitpid( pid, &childExitStatus, 0);
    if (ws == -1) {
        fprintf(stderr,
        		  "%s [WRAPPER-copy_file] cannot wait for child process (cp %s %s)\n",
        		  boinc_msg_prefix(buf, sizeof(buf)),
        		  source.c_str(),
        		  dest.c_str());
      return -1;
    }
    if (WIFSIGNALED(childExitStatus)) { /* killed */
        fprintf(stderr,
        		  "%s [WRAPPER-copy_file] signaled child process (cp %s %s)\n",
        		  boinc_msg_prefix(buf, sizeof(buf)),
        		  source.c_str(),
        		  dest.c_str());
      return -1;
    }
    else if (WIFSTOPPED(childExitStatus)) { /* stopped */
        fprintf(stderr,
        		  "%s [WRAPPER-copy_file] stopped child process (cp %s %s)\n",
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
        		  "%s [WRAPPER-init_dir] failed to create dir %s. Permission denied.\n",
        		  boinc_msg_prefix(buf, sizeof(buf)),
        		  dir.c_str());
	}
	else {
        fprintf(stderr,
        		  "%s [WRAPPER-init_dir] failed to create dir %s. Error code %d.\n",
        		  boinc_msg_prefix(buf, sizeof(buf)),
        		  dir.c_str(),
        		  status);
	}
	return status;
}

/**
 * This is application specific code. It should be isolated as possible.
 * Only one convention remains: all input and output files must be read and
 * written inside input_dir and output_dir respectively.
 */
int upper_case(std::string input_dir, std::string output_dir) {
	std::string input_file = input_dir + "/infile";
	std::string output_file = output_dir + "/outfile";
    MFILE out;
    FILE* in;
    int retval;
	char c;

    in = boinc_fopen(input_file.c_str(), "r");
    if (!in) {
        fprintf(stderr,
            "%s [APP] Couldn't find input file, resolved name %s.\n",
            boinc_msg_prefix(buf, sizeof(buf)), input_file.c_str());
        return -1;
    }

    retval = out.open(output_file.c_str(), "wb");
    if (retval) {
    	fprintf(stderr, "%s [APP]: upper_case output open failed:\n",
    			boinc_msg_prefix(buf, sizeof(buf)));
    	fprintf(stderr, "%s resolved name %s, retval %d\n",
    			boinc_msg_prefix(buf, sizeof(buf)),
    			output_file.c_str(),
    			retval);
    	perror("open");
    	return -1;
    }

    for (int i=0; ; i++) {
        c = fgetc(in);
        if (c == EOF) break;
        c = toupper(c);
        out._putchar(c);
    }

    retval = out.flush();
    if (retval) {
        fprintf(stderr,
        		"%s [APP]: upper_case flush failed %d\n",
        		boinc_msg_prefix(buf, sizeof(buf)),
        		retval);
        return -1;
    }
    return 0;
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
	        		"%s [WRAPPER-process_cmd_args] unknown cmd arg %s\n",
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

// Do not include files and folders whose name starts with a "." (dot).
bool file_filter(std::string const& f) {
	if (libtorrent::filename(f)[0] == '.') return false;
	fprintf(stderr, "%s\n", f.c_str());
	return true;
}

// Creates a torrent file ("output_torrent") representing the contents of
// "output_dir". The tracker "tracker_url" is added. See documentation in
// http://www.rasterbar.com/products/libtorrent/make_torrent.html
int make_torrent(
		std::string output_dir,
		std::string output_torrent,
		std::string tracker_url) {
	std::string creator_str = "freeCycles-wrapper (using libtorrent)";
	libtorrent::file_storage fs;
	libtorrent::error_code ec;
	int flags = 0;
	int piece_size = 0;
	int pad_file_limit = -1;
	std::string full_path = libtorrent::complete(output_dir);

	add_files(fs, full_path, file_filter, flags);
	if (fs.num_files() == 0) {
        fprintf(stderr,
        		"%s [WRAPPER-make_torrent] no files found in %s\n",
        		boinc_msg_prefix(buf, sizeof(buf)),
        		output_dir.c_str());
		return -1;
	}

	libtorrent::create_torrent t(fs, piece_size, pad_file_limit, flags);
	t.add_tracker(tracker_url);

	libtorrent::set_piece_hashes(t,libtorrent::parent_path(full_path),ec);
	if (ec)	{
        fprintf(stderr,
        		"%s [WRAPPER-make_torrent] %s\n",
        		boinc_msg_prefix(buf, sizeof(buf)),
        		ec.message().c_str());
		return -1;
	}

	t.set_creator(creator_str.c_str());

	// create the torrent and print it to stdout
	std::vector<char> torrent;
	bencode(back_inserter(torrent), t.generate());

	FILE* output = fopen(output_torrent.c_str(), "wb+");
	if (output == NULL)	{
        fprintf(stderr,
        		"%s [WRAPPER-make_torrent] failed to open file %s: (%d) %s\n",
        		boinc_msg_prefix(buf, sizeof(buf)),
        		output_torrent.c_str(),
        		errno,
        		strerror(errno));
		return -1;
	}
	fwrite(&torrent[0], 1, torrent.size(), output);
	fclose(output);
	return 0;
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
    input_torrent = shared_dir + "/" + wu_name + ".input.torrent";
    input_dir = shared_dir + "/" + wu_name + ".input";
    output_torrent = shared_dir + "/" + wu_name + ".output.torrent";
    output_dir = shared_dir + "/" + wu_name + ".output";

    // Initialize needed directories.
    init_dir(shared_dir);

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

    // TODO - while:
	// 1- application is running
	// 2- we are waiting for the input files
	// -> search for new .torrent files.

    // Add torrent and wait until completion
    wait_torrent(add_torrent(input_path));
    // Copy input .torrent file to shared dir.
    copy_file(input_path, input_torrent);
    // TODO - application should be run in a separate process.
    // Call application.
    upper_case(input_dir, output_dir);
    // Create .torrent file for output directory.
    make_torrent(output_dir, output_torrent, tracker_url);
    // Copy output .torrent file to the right place (BOINC expected location).
    copy_file(output_torrent, output_path);


    boinc_fraction_done(1);
    boinc_finish(0);
}
