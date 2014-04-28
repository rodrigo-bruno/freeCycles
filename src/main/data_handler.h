#ifndef __DATA_HANDLER_H__
#define __DATA_HANDLER_H__

#include <stdio.h>

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
 * Moves one file from source to destination.
 */
int move_file(const string& source, const string& dest) {
  pid_t pid;

  // Split execution
  if((pid = fork()) == 0)
  { return execl("/bin/mv", "/bin/mv", source.c_str(), dest.c_str(), (char *)0); }
  else {
	  sprintf(dh_buf, "%s %s %s", "/bin/mv", source.c_str(), dest.c_str());
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
  // Three additional args: 1) executable path, 2) output zip file path, 3) NULL.
  char** args = new char* [files.size()+3];

  // Setup command
  args[0] = "/bin/zip";
  args[1] = output.c_str();
  for(int i = 0; i < files.size(); i++)
  { args[i+2] = files[i].c_str(); }
  args[files.size()+3 - 1] = NULL;

  // Split execution
  if((pid = fork()) == 0)
  { return execv(args[0], args); }
  else {
	  sprintf(
			  dh_buf,
			  "%s %s ... %s", "/bin/zip",
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
	char *line, *ptr;
	size_t len;
	// Split execution
	if((pid = fork()) == 0) {
		return execl(
			"/bin/unzip",
			"/bin/unzip",
			"-d", wdir.c_str(),
			input.c_str(),
			(char *)0);
	}
	else {
		sprintf(dh_buf, "%s %s", "/bin/unzip", input.c_str());
		if(!handle_child_proc(pid, dh_buf)) {
			// TODO - handle failure
		}
		// Read hidden file (which contains list of unzipped files)
		if(!(f = fopen((wdir+".files").c_str(), "r"))) {
			// TODO - handle failure
		}
		// Read all files and fill the vector.
		while (getline(&line, &len, f) != -1) {
			ptr = strtok(line," ");
			while(ptr != NULL) {
				files.push_back(wdir + ptr);
				strtok(NULL," ");
			}
		}
		free(line);
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
 * TODO - doc.
 */
bool torrent_done_asd (const libtorrent::torrent_handle& t)
{ return t.status().is_seeding; }

/**
 * TODO - doc.
 * Do not include files and folders whose name starts with a "." (dot).
 */
bool file_filter(string const& f) {
	if (libtorrent::filename(f)[0] == '.') return false;
	fprintf(stderr, "%s\n", f.c_str());
	return true;
}

/**
 * TODO - comment.
 */
class BitTorrentHandler : public DataHandler {
private:
	libtorrent::session_settings bt_settings;
	libtorrent::session bt_session;
	libtorrent::error_code bt_ec;
	string shared_dir;
	string tracker_url;

public:

	/**
	 * TODO - put a comment on shared_dir and tracker_url.
	 */
	BitTorrentHandler(
			string input,
			string output,
			string working_dir,
			string shared_dir,
			string tracker_url) :
				shared_dir(shared_dir),
				tracker_url(tracker_url),
				DataHandler(input, output, working_dir) {
		init_dir(shared_dir);
	}
	BitTorrentHandler() = delete;
	BitTorrentHandler(const BitTorrentHandler& bt) = delete;
	~BitTorrentHandler() {}

	/**
	 * I am assuming that the BOINC input path preserves the original file name
	 * (the one given by the work generator).
	 */
	void get_input(string& input) {
		list<libtorrent::torrent_handle> handles;

		// Add input torrent (input path) with shared_dir as shared_dir.
		handles.push_back(this->add_torrent(this->input_path, this->shared_dir));
		// Wait until its done.
		this->wait_torrent(handles);
		// Copy input torrent to shared dir.
		copy_file(this->input_path, this->shared_dir);
		// Return the file path.
		input = handles.front().get_torrent_info().file_at(0).path;
	}

	void get_zipped_input(vector<string>& inputs) {
		list<libtorrent::torrent_handle> handles;
		vector<string>::iterator vit;
		list<libtorrent::torrent_handle>::iterator lit;

		// Extract all .torrent files to working directory.
		unzip_files(this->working_dir, this->input_path, inputs);
		// For every .torrent file, add torrent and save handle.
		for(vit = inputs.begin(); vit != inputs.end(); vit++)
		{ handles.push_back(this->add_torrent(*vit, this->shared_dir)); }
		// Wait until all torrents are done.
		this->wait_torrent(handles);
		// Copy .torrent files to shared directory.
		copy_file(this->working_dir + "*.torrent", this->shared_dir);
		// Return the file paths.
		inputs.clear();
		for(lit = handles.begin(); lit != handles.end(); lit++)
		{ inputs.push_back(lit->get_torrent_info().file_at(0).path); }
	}

	/**
	 * Note: this output will most likely point somewhere inside a working dir.
	 */
	void stage_output(string& output) {
		// Create .torrent (according to BOINC output path).
		make_torrent(output, this->output_path);
		// Move output to shared directory.
		move_file(output, this->shared_dir);
		// Copy .torrent to shared directory.
		copy_file(this->output_path, this->shared_dir);
	}

	void stage_zipped_output(vector<string>& outputs) {
		vector<string> torrents = vector<string>(outputs.size());
		vector<string>::iterator vit;

		// For every output file, create .torrent.
		for(vit = outputs.begin(); vit != outputs.end(); vit++) {
			torrents.push_back(*vit+".torrent");
			make_torrent(*vit, *vit+".torrent");
		}
		// Zip .torrent files and place zip into BOINC output path.
		zip_files(this->output_path, torrents);
		// Move all files from working directory (output files and .torrents) to
		// the shared directory. WARNING: this could be dangerous.
		move_file(this->working_dir+"/*", this->shared_dir);
	}

	/**
	 * TODO - doc.
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
	 * TODO - doc.
	 * Blocking function that holds execution while input is not ready.
	 */
	void wait_torrent(list<libtorrent::torrent_handle> torrents)
	{ while(!torrents.size()) { torrents.remove_if(torrent_done_asd); } }

	// Creates a torrent file ("output_torrent") representing the contents of
	// "output_file". The tracker "tracker_url" is added. See documentation in
	// http://www.rasterbar.com/products/libtorrent/make_torrent.html
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

		// create the torrent and print it to stdout
		std::vector<char> torrent;
		bencode(back_inserter(torrent), t.generate());

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
