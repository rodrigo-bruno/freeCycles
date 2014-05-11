#ifndef __DATA_HANDLER_H__
#define __DATA_HANDLER_H__

#include <stdio.h>
#include <wait.h>

#include <vector>
#include <list>
#include <string>

#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/session.hpp"
#include "libtorrent/create_torrent.hpp"

using std::vector;
using std::list;
using std::string;

// Auxiliary buffer for some C I/O operations.
char dh_buf[1024];

/**
 * Auxiliary function that handles child processes. It waits until child process
 * completion and checks for errors.
 */
int handle_child_proc(pid_t pid, char* cmd) {
	int childExitStatus;
	pid_t ws;
	char* tag = "[DH-handle_child_proc]";
	if (pid < 0) {
		fprintf(stderr,"%s can't fork to perform: %s\n", tag, cmd);
	    return pid;
	}
	ws = waitpid( pid, &childExitStatus, 0);
	if (ws == -1) {
		fprintf(stderr, "%s cannot wait for child process: %s\n", tag, cmd);
		return pid;
	}
	if (WIFSIGNALED(childExitStatus)) { /* killed */
		fprintf(stderr,	"%s signaled child process: %s\n", tag, cmd);
		return pid;
	}
	else if (WIFSTOPPED(childExitStatus)) { /* stopped */
		fprintf(stderr, "%s child process was stopped: %s\n", tag, cmd);
		return pid;
	}
	return 0;
}

/**
 * Copies one file from source to destination.
 */
int copy_file(const string& source, const string& dest) {
  pid_t pid;

  // Split execution
  if((pid = fork()) == 0)
  { return execl("/bin/cp", "/bin/cp", source.c_str(), dest.c_str(), (char *)0); }
  else {
	  sprintf(dh_buf, "%s %s %s", "/bin/cp", source.c_str(), dest.c_str());
	  return handle_child_proc(pid, dh_buf);
  }
}

/**
 * Zips a set of files into an output (zipped) file.
 * Note: All files are PATHs.
 */
int zip_files(const string& output, vector<string>& files) {
  pid_t pid;
  int i, ret;
  // Four additional args (see below).
  const char** const args = new const char*[files.size()+4]();

  // Setup command
  args[0] = "/usr/bin/zip"; 		// 1) executable path,
  args[1] = "-j0"; 					// 2) -j (junk paths), 0 (only store)
  args[2] = output.c_str(); 		// 3) output zip file path,
  for(int i = 0; i < files.size(); i++)
  { args[i+3] = files[i].c_str(); }
  args[files.size()+4 - 1] = NULL; 	// 4) NULL.

  // Split execution
  if((pid = fork()) == 0)
  // Lets all hope this is OK (cast from const char** const to char** const).
  { return execv("/usr/bin/zip", (char** const)args); }
  else {
	  sprintf(
			  dh_buf,
			  "%s %s ... %s",
			  "/usr/bin/zip",
			  files.front().c_str(),
			  files.back().c_str());
	  ret = handle_child_proc(pid, dh_buf);
	  // Just to be safe, free args after child is done.
	  free(args);
	  return ret;
  }
}

/**
 * Unzips a given input file and returns the vector of extracted files.
 * Note: All files are PATHs.
 * Note: Zipped input files have a special file (".files") that contains the
 * name (NOT PATH) of all the extracted files (except for it self).
 */
int unzip_files(const string& wdir, const string& input, vector<string>& files) {
	pid_t pid;
	FILE* f;
	char buf[128];
	// Split execution
	if((pid = fork()) == 0) {
		return execl(
			"/usr/bin/unzip",
			"/usr/bin/unzip",
			"-u",
			"-d", wdir.c_str(),
			input.c_str(),
			(char *)0);
	}
	else {
		sprintf(dh_buf, "%s %s", "/usr/bin/unzip", input.c_str());
		handle_child_proc(pid, dh_buf);
		// Read hidden file (which contains list of unzipped files)
		if(!(f = fopen((wdir+".files").c_str(), "r"))) {
	        fprintf(stderr,
	        		"[DH-unzip] failed to open file %s.\n",
	        		(wdir+".files").c_str());
		}
		// Read all files and fill the vector.
		while (fscanf(f, "%s", buf) != EOF) { files.push_back(wdir + buf); }
		fclose(f);
		return 0;
	}
}

/**
 * Initializes (creates) a directory.
 */
int init_dir(string dir) {
	int status = mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	if(status == 0 || status == EEXIST) { return 0; }
	if(status == EACCES) {
        fprintf(stderr,
        		"[DH-init_dir] failed to create dir %s. Permission denied.\n",
        		dir.c_str());
	}
	else {
        fprintf(stderr,
        		"[DH-init_dir] failed to create dir %s. Error code %d.\n",
        		dir.c_str(),
        		status);
	}
	return status;
}

/**
 * Auxiliary function that is used as a predicate to check if a torrent has
 * finished downloading or not.
 */
bool torrent_done (const libtorrent::torrent_handle& t)
{ return t.status().is_seeding; }

/**
 * Auxiliary function that is used as a predicato to check if a file is
 * accessible or not.
 */
bool file_ready (const string& s) {
	struct stat buffer;
	return (stat(s.c_str(), &buffer) == 0);
}

/**
 * Auxiliary function that is used as a predicate and detected hidden files.
 */
bool file_filter(string const& f) {
	if (libtorrent::filename(f)[0] == '.') return false;
	fprintf(stderr, "%s\n", f.c_str());
	return true;
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

protected:
	string input_path;
	string output_path;
	string working_dir;

public:
	/**
	 * Both input and output paths are paths given by BOINC. They are used
	 * to transfer files using BOINC protocol (HTTP). These paths are
	 * configured using job templates.
	 * The working directory is used whenever we need to have temporary files
	 * (eg: unzipped files).
	 */
	DataHandler(string input_path, string output_path, string working_dir) :
			input_path(input_path),
			output_path(output_path),
			working_dir(working_dir) {
		init_dir(working_dir);
	}
	virtual ~DataHandler() {}

	/**
	 * This method returns the path to the real input. This is the path that
	 * should be used to open the file.
	 */
	virtual void get_input(string& input) { input = this->input_path; }

	/**
	 * Method similar to 'get_input'. This one has additional functionality
	 * since it manages a zipped input (that holds several input files).
	 */
	virtual void get_zipped_input(vector<string>& inputs)
	{ unzip_files(this->working_dir, this->input_path, inputs); }

	/**
	 * This method receives an output file path and stages it at its
	 * expected location (BOINC path).
	 */
	virtual void stage_output(string& output)
	{ copy_file(output, this->output_path); }

	/**
 	 * Method similar to 'stage_output'. This one has additional 
 	 * functionality to handle multiple output files (they are zipped into
 	 * one). 
 	 */	 
	virtual void stage_zipped_output(vector<string>& outputs)
	{ zip_files(this->output_path, outputs); }
};


/**
 * BitTorrentHandler is another implementation but it uses the BitTorrent
 * protocol to move data. The BOINC protocol is still used only to carry
 * .torrent files.
 */
class BitTorrentHandler : public DataHandler {
private:
	libtorrent::session_settings bt_settings;
	libtorrent::session bt_session;
	libtorrent::error_code bt_ec;
	string shared_dir;
	string tracker_url;
	string wu_name;

public:

	/**
	 * Regarding the fields not already described in the superclass (shared_dir
	 * and tracker_url): the first one is a path to the locatio where multiple
	 * clients share their input; the second one is the tracker url (that goes
	 * in the .torrent file).
	 */
	BitTorrentHandler(
			string input,
			string output,
			string working_dir,
			string shared_dir,
			string tracker_url,
			string wu_name) :
				shared_dir(shared_dir),
				tracker_url(tracker_url),
				wu_name(wu_name),
				DataHandler(input, output, working_dir) {
		init_dir(shared_dir);
	}
	BitTorrentHandler() = delete;
	BitTorrentHandler(const BitTorrentHandler& bt) = delete;
	~BitTorrentHandler() {}

	void get_input(string& input) {
		list<libtorrent::torrent_handle> handles;
		list<string> files;

		// Add input torrent (input path) with shared_dir as shared_dir.
		handles.push_back(this->add_torrent(this->input_path, this->shared_dir));
#if DEBUG
    	debug_log("[DH-get_input]", "Torrent added:.", this->input_path.c_str());
#endif
		// Wait until its done.
		this->wait_torrent(handles);
#if DEBUG
    	debug_log("[DH-get_input]", "download done.", "");
#endif
		// Return the file PATH.
		input = this->shared_dir + handles.front().name();
		// Copy input torrent to shared dir (the name has to be faked since
		// boinc does not preserve the original file name).
		copy_file(this->input_path, input + ".torrent");
		// Wait until the file is accessible.
		files.push_back(input);
#if DEBUG
    	debug_log("[DH-get_input]", "waiting for file:", input.c_str());
#endif
		this->wait_files(files);
	}

	void get_zipped_input(vector<string>& inputs) {
		list<libtorrent::torrent_handle> handles;
		list<string> files;
		vector<string>::iterator vit;
		list<libtorrent::torrent_handle>::iterator lit;

		// Extract all .torrent files to working directory.
		unzip_files(this->working_dir, this->input_path, inputs);
		// For every .torrent file, add torrent and save handle.
		for(vit = inputs.begin(); vit != inputs.end(); vit++)
		{ handles.push_back(this->add_torrent(*vit, this->shared_dir)); }
		// Wait until all files are downloaded.
		this->wait_torrent(handles);
		inputs.clear();
		for(lit = handles.begin(); lit != handles.end(); lit++)	{
			// Prepare output (list of input file paths).
			inputs.push_back(this->shared_dir + lit->name());
			// Prepare list to use in wait_files
			files.push_back(inputs.back());
			// Move .torrent files from working to shared directory.
			rename(	(this->working_dir+lit->name()+".torrent").c_str(),
					(this->shared_dir+lit->name()+".torrent").c_str());
		}
		// Wait until the file is accessible.
		this->wait_files(files);
	}

	/**
	 * Note: this output will most likely point somewhere inside a working dir.
	 */
	void stage_output(string& output) {
		// Create .torrent (according to BOINC output path).
		make_torrent(output, this->output_path);
		// Move output to shared directory.
		rename(output.c_str(), this->shared_dir.c_str());
		// Copy .torrent to shared directory.
		copy_file(this->output_path, this->shared_dir + wu_name + ".torrent");
	}

	void stage_zipped_output(vector<string>& outputs) {
		vector<string> torrents = vector<string>();
		vector<string>::iterator vit;

		// For every output file, create .torrent.
		for(vit = outputs.begin(); vit != outputs.end(); vit++) {
			torrents.push_back(*vit+".torrent");
			// These .torrent files go directly to the shared directory.
			make_torrent(*vit, *vit+".torrent");
		}
		// Zip .torrent files and place zip into BOINC output path.
		zip_files(this->working_dir + "output", torrents);
		copy_file((this->working_dir + "output.zip").c_str(),
				   this->output_path.c_str());
	}

	/**
	 * Initialization function, starts BitTorrent client after some
	 * configuration (some configurations are user definable).
	 */
	int init(int download_rate,	int upload_rate) {
		// Setup BitTorrent settings
	    this->bt_settings = libtorrent::high_performance_seed();
	    this->bt_settings.allow_multiple_connections_per_ip = true;
	    this->bt_settings.active_downloads = -1; // unlimited
	    this->bt_settings.download_rate_limit = download_rate;
	    this->bt_settings.upload_rate_limit = upload_rate;
	    this->bt_session.set_settings(bt_settings);
	    this->bt_session.listen_on(std::make_pair(6500, 7000), bt_ec);
	    if (bt_ec)	{
	        fprintf(stderr,
	                "[DH] Failed to open listen socket: %s\n",
	                bt_ec.message().c_str());
			return 1;
		}
	    this->check_shared_torrents();
	    return 0;
	}

	/**
	 * Adds a new torrent to the current session.
	 * Note: Save path is the directory path where the downloaded file will be.
	 */
	libtorrent::torrent_handle add_torrent(string torrent, string save_path) {
		libtorrent::add_torrent_params p;
		p.save_path = this->shared_dir;
		p.ti = new libtorrent::torrent_info(torrent, this->bt_ec);
		return this->bt_session.add_torrent(p, this->bt_ec);
	}

	/**
	 * Blocking function that holds execution while input is not ready.
	 */
	void wait_torrent(list<libtorrent::torrent_handle> torrents) {
		torrents.remove_if(torrent_done);
		while(torrents.size()) {
			torrents.remove_if(torrent_done);
			sleep(1);
		}
	}

	/**
	 * Blocking function that holds execution while input is not ready.
	 */
	void wait_files(list<string> files) {
		files.remove_if(file_ready);
		while(files.size()) {
			files.remove_if(file_ready);
			sleep(1);
		}
	}

	/**
	 * Searches for torrent files inside the shared directory.
	 * Found torrents will be added to the given session.
	 */
	void check_shared_torrents() {
		DIR* dir;

		// try to open dir. If opendir is unsuccessful, return.
		if((dir = opendir(this->shared_dir.c_str())) == NULL) { return; }
		// look for all files and try to add them as new torrents.
		for(struct dirent* dp = readdir(dir); dp != NULL; dp = readdir(dir)) {

			if(dp->d_type == DT_DIR) { continue; }
			if(! ends_with(dp->d_name, ".torrent")) { continue; }

			add_torrent(this->shared_dir+dp->d_name, this->shared_dir);
		}
	}

	/**
	 * Creates a torrent file ("output_torrent") representing the contents of
	 * "output_file". The tracker "tracker_url" is added. See documentation in
	 * http://www.rasterbar.com/products/libtorrent/make_torrent.html
	 */
	int make_torrent(string output_file, string output_torrent) {
		libtorrent::file_storage fs;
		libtorrent::error_code ec;
		int flags = 0, piece_size = 0, pad_file_limit = -1;
		string full_path = libtorrent::complete(output_file);

		add_files(fs, full_path, file_filter, flags);
		if (fs.num_files() == 0) {
	        fprintf(stderr,
	        		"[DH-make_torrent] failed to add file %s\n",
	        		output_file.c_str());
			return 1;
		}

		libtorrent::create_torrent t(fs, piece_size, pad_file_limit, flags);
		t.add_tracker(this->tracker_url);

		libtorrent::set_piece_hashes(t,libtorrent::parent_path(full_path),ec);
		if (ec)	{
	        fprintf(stderr, "[DH-make_torrent] %s\n", ec.message().c_str());
			return 1;
		}

		t.set_creator("freeCycles-wrapper (using libtorrent)");

		// create the torrent
		libtorrent::entry e = t.generate();
		// hack to change creation date (needs to be the same for all .torrents files)
		e.dict()["creation date"] = 0;

		std::vector<char> torrent;
		bencode(back_inserter(torrent), e);

		FILE* output = fopen(output_torrent.c_str(), "wb+");
		if (output == NULL)	{
	        fprintf(stderr,
	        		"[DH-make_torrent] failed to open file %s: (%d) %s\n",
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
