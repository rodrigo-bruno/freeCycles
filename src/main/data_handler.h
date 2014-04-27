#ifndef DATA_HANDLER_H_
#define DATA_HANDLER_H_

#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/session.hpp"
#include "libtorrent/create_torrent.hpp"

// BOINC template names for input and output files.
#define BOINC_INPUT_FILENAME "in"
#define BOINC_OUTPUT_FILENAME "out"

int handle_child_proc(pid_t pid) {
	return 0;
}

// Copies one file from source to destination.
int copy_file(const std::string& source, const std::string& dest) {
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
    return 1;
  }
  else {
    pid_t ws = waitpid( pid, &childExitStatus, 0);
    if (ws == -1) {
        fprintf(stderr,
        		  "%s [WRAPPER-copy_file] cannot wait for child process (cp %s %s)\n",
        		  boinc_msg_prefix(buf, sizeof(buf)),
        		  source.c_str(),
        		  dest.c_str());
      return 1;
    }
    if (WIFSIGNALED(childExitStatus)) { /* killed */
        fprintf(stderr,
        		  "%s [WRAPPER-copy_file] signaled child process (cp %s %s)\n",
        		  boinc_msg_prefix(buf, sizeof(buf)),
        		  source.c_str(),
        		  dest.c_str());
      return 1;
    }
    else if (WIFSTOPPED(childExitStatus)) { /* stopped */
        fprintf(stderr,
        		  "%s [WRAPPER-copy_file] stopped child process (cp %s %s)\n",
        		  boinc_msg_prefix(buf, sizeof(buf)),
        		  source.c_str(),
        		  dest.c_str());
      return 1;
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
        		  "%s [DH-init_dir] failed to create dir %s. Permission denied.\n",
        		  boinc_msg_prefix(buf, sizeof(buf)),
        		  dir.c_str());
	}
	else {
        fprintf(stderr,
        		  "%s [DH-init_dir] failed to create dir %s. Error code %d.\n",
        		  boinc_msg_prefix(buf, sizeof(buf)),
        		  dir.c_str(),
        		  status);
	}
	return status;
}

/**
 * This implementation assumes that there is only one input file and only one
 * output file. The file paths can be resolved using BOINC API since they have
 * constant identifiers (specified at server templates).
 * If you want multiple files for input or output, we do support zipped files.
 *
 * DataHandler is the default implementation (it uses BOINC file transfers).
 */
class DataHandler {

private:
	std::string input_path;
	std::string output_path;

public:
	DataHandler(std::string input_path, std::string output_path) :
		input_path(input_path), output_path(output_path) {	}
	virtual ~DataHandler() {}

	/**
	 * This method returns the path to the real input. This is the path that
	 * should be used to open the file.
	 */
	virtual std::string get_input() { return std::string(input_path); }

	/**
	 * Method similar to 'get_input'. This one has additional functionality
	 * since it manages a zipped input (that holds several input files).
	 * Note: zipped input files have a special file (".files") that contains
	 * the name of all the extracted files (except for it self).
	 */
	virtual std::vector<std::string> get_zipped_input() {
		// unzip input_path .
		// read ".files"
		return std::vector<std::string>();
	}

	virtual void stage_output(std::string output) {
		// mv output output_path
	}

	virtual void stage_zipped_output(std::vector<std::string> outputs) {
		// zip outputs output_path
	}
};

class BitTorrentHandler : public DataHandler {
private:
	libtorrent::session_settings bt_settings;
	libtorrent::session bt_session;
	libtorrent::error_code bt_ec;
	std::string shared_dir;
	std::string tracker_url;

public:

	BitTorrentHandler(std::string input, std::string output) :
		DataHandler(input, output) {}

	std::string get_input() {
		// copy input_path to shared dir
		// add torrent
		// wait until its done and return the file name.
		return std::string();
	}

	std::vector<std::string> get_zipped_input() {
		return std::vector<std::string>();
	}

	void stage_output(std::string output) {
		// create .torrent for each one and place them at the shared dir.
		// call base with .torrent file names
	}

	void stage_zipped_output(std::vector<std::string> outputs) {
		// create .torrent for each one and place them at the shared dir.
		// call base with .torrent file names
	}

	int init(
			int download_rate,
			int upload_rate,
			std::string shared_dir,
			std::string tracker_url) {
	    // Initialize needed directories. -> TODO - move to data_handler?
	    init_dir(shared_dir);
		// Setup BitTorrent settings
	    bt_settings = libtorrent::high_performance_seed();
	    bt_settings.allow_multiple_connections_per_ip = true;
	    bt_settings.active_downloads = -1; // unlimited
	    bt_settings.download_rate_limit = download_rate;
	    bt_settings.upload_rate_limit = upload_rate;
	    this->shared_dir = shared_dir;
	    this->tracker_url = tracker_url;
	    bt_session.set_settings(bt_settings);
	    bt_session.listen_on(std::make_pair(6500, 7000), bt_ec);
		// FIXME - error handling
	    if (bt_ec)	{
	        fprintf(stderr,
	                "%s [BT] Failed to open listen socket: %s\n",
	                boinc_msg_prefix(buf, sizeof(buf)),
	                bt_ec.message().c_str());
			return 1;
		}
	    return 0;
	}

	// Adds a new torrent to the current session.
	libtorrent::torrent_handle add_torrent(const char* torrent) {
		libtorrent::add_torrent_params p;
		p.save_path = shared_dir;
		p.ti = new libtorrent::torrent_info(torrent, this->bt_ec);
		return bt_session.add_torrent(p, this->bt_ec);
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
			return 1;
		}

		libtorrent::create_torrent t(fs, piece_size, pad_file_limit, flags);
		t.add_tracker(tracker_url);

		libtorrent::set_piece_hashes(t,libtorrent::parent_path(full_path),ec);
		if (ec)	{
	        fprintf(stderr,
	        		"%s [WRAPPER-make_torrent] %s\n",
	        		boinc_msg_prefix(buf, sizeof(buf)),
	        		ec.message().c_str());
			return 1;
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
			return 1;
		}
		fwrite(&torrent[0], 1, torrent.size(), output);
		fclose(output);
		return 0;
	}

};

#endif /* DATA_HANDLER_H_ */
