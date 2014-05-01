// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// sample_work_generator.cpp: an example BOINC work generator.
// This work generator has the following properties
// (you may need to change some or all of these):
//
// - Runs as a daemon, and creates an unbounded supply of work.
//   It attempts to maintain a "cushion" of 100 unsent job instances.
//   (your app may not work this way; e.g. you might create work in batches)
// - Creates work for the application "example_app".
// - Creates a new input file for each job;
//   the file (and the workunit names) contain a timestamp
//   and sequence number, so they're unique.

#include <sys/param.h>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>

#include "boinc_db.h"
#include "error_numbers.h"
#include "backend_lib.h"
#include "parse.h"
#include "util.h"
#include "svn_version.h"

#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"
#include "str_util.h"

#include "mr_parser.h"
#include "mr_jobtracker.h"

#define CUSHION 10
    // maintain at least this many unsent results
#define REPLICATION_FACTOR  1

// FIXME - this file should be named freeCycles_work_generator.
// It will use the mr-wrapper and, consequently, the bt-wrapper.

// TODO - put some decent names.
const char* app_name = "example_app";
const char* in_template_file = "example_app_in";
const char* out_template_file = "example_app_out";
const char* jobtracker_file_path = "/home/boincadm/projects/test4vm/mr/jobtracker.xml";
const char* shuffle_script = "/home/boincadm/git/freeCycles/src/scripts/shuffle.sh";

char* in_template;
DB_APP app;
std::vector<MapReduceJob> jobs;
FILE* jobtracker_file = NULL;

/**
 * Helper function that copies file(s).
 * It uses the cp (unix program) do perform the copy.
 */
int copy_file(const char* source, const char* dest) {
  pid_t pid;
  int childExitStatus;
  pid = fork();
  
  if(pid == 0) {
    return execl("/bin/cp", "/bin/cp", source, dest, (char *)0);
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

/**
 * Helper function that shuffles the intermediate data.
 * It uses an external script to perform the shuffle.
 */
int shuffle(const char* job_id, const char* nreds) {
  pid_t pid;
  int childExitStatus;
  pid = fork();

  if(pid == 0) {
    return execl(shuffle_script, shuffle_script, job_id, nreds, (char*)0);
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

/**
 * Helper function that writes to the jobtracker state file.
 * It is called when a task is sent.
 */
void write_task_state(MapReduceTask* mrt) {
	fseek(jobtracker_file, mrt->getStateOffset(),SEEK_SET);
	fwrite(TASK_CREATED, 1, 1, jobtracker_file);
	fflush(jobtracker_file);
	mrt->setState(TASK_CREATED);
}

/**
 * Helper function that writes to the jobtracker state file.
 * It is called when the shuffle is performed.
 */
void write_shuffled_state(MapReduceJob* mrj) {
	fseek(jobtracker_file, mrj->getShuffledOffset(),SEEK_SET);
	fwrite("1", 1, 1, jobtracker_file);
	fflush(jobtracker_file);
	mrj->setShuffled(true);
}

/**
 * This function re-reads the jobtracker state file to update the map task
 * status (this is important to start the reduce phase).
 */
void read_maps_state(MapReduceJob& mrj) {
	std::vector<MapReduceTask>& maps = mrj.getMapTasks();
	std::vector<MapReduceTask>::iterator it;
	for( it = maps.begin(); it != maps.end();	++it) {
		char state[2];
		fseek(jobtracker_file, it->getStateOffset(), SEEK_SET);
		fread(&state, 1, 1, jobtracker_file);
		state[1] = '\0';
		it->setState(std::string(state));
	}
}

/**
 * This function tries to find a task to sent to a volunteer. The algorithm
 * proceeds as follows:
 * - for each job
 * 	- tries to find a map task
 * 	- tries to find a reduce task (if all map tasks are finished)
 */
MapReduceTask* get_MapReduce_task(std::vector<MapReduceJob>& jobs_ref) {
	std::vector<MapReduceJob>::iterator it;
	MapReduceTask* mrt = NULL;
	char buf[128];
	for( it = jobs_ref.begin(); it != jobs_ref.end(); ++it) {
		// if this job is already deployed (maps and reduces), nothing to do.
		if(!it->hasUnsentTasks()) { continue; }
		mrt = it->getNextMap();
		// if all map tasks were already delivered.
		if(mrt == NULL) {
			read_maps_state(*it);
			mrt = it->getNextReduce();
			// this means that we might be waiting for map results.
			if(mrt == NULL) { continue; }
			else {
				// before returning a reduce task, make sure that the input is
				// shuffled already.
				if(it->needShuffle()) {
					sprintf(buf, "%lu", it->getReduceTasks().size());
					shuffle(it->getID().c_str(), buf);
					write_shuffled_state(&(*it));
				}
				return mrt;
			}
		}
		else { return mrt; }
	}
	// if no job has more tasks to deliver.
	return NULL;
}

/**
 * Creates a new job.
 */
int make_job(MapReduceTask* mrt) {
    DB_WORKUNIT wu;
    char path[MAXPATHLEN];
    const char* infiles[1];
    int retval;

    // Put the input file at the right place in the download dir hierarchy.s
    retval = config.download_path(mrt->getName().c_str(), path);
    if (retval) return retval;


    log_messages.printf(MSG_NORMAL, "Making workunit %s\n", path);

    retval = copy_file(mrt->getInputPath().c_str(), path);
    if (retval) return retval;

    // Fill in the job parameters
    //
    wu.clear();
    wu.appid = app.id;
    strcpy(wu.name, mrt->getName().c_str());
    wu.rsc_fpops_est = 1e12;
    wu.rsc_fpops_bound = 1e14;
    wu.rsc_memory_bound = 1e8;
    wu.rsc_disk_bound = 1e8;
    wu.delay_bound = 86400;
    wu.min_quorum = REPLICATION_FACTOR;      /* TODO <- 3 for both map and reduce */
    wu.target_nresults = REPLICATION_FACTOR; /* TODO <- 5 if map, 3 if reduce */
    wu.max_error_results = REPLICATION_FACTOR*4;
    wu.max_total_results = REPLICATION_FACTOR*8;
    wu.max_success_results = REPLICATION_FACTOR*4;

    // Extracts the file name from path.
    infiles[0] = mrt->getInputPath().substr(
    		mrt->getInputPath().find_last_of("\\")+1).c_str();
    log_messages.printf(MSG_NORMAL, "In File %s", infiles[0]);

    // Updates the MapReduceTask state and updates the jobtracker state file.
    write_task_state(mrt);

    // Register the job with BOINC.
    sprintf(path, "templates/%s", out_template_file);
    return create_work(
        wu,
        in_template,
        path,
        config.project_path(path),
        infiles,
        1,
        config
    );
}

void main_loop() {
    int retval;
    MapReduceTask* mrt = NULL;
    while (1) {
        check_stop_daemons();
        int n;
        retval = count_unsent_results(n, 0);
        if (retval) {
            log_messages.printf(
            		MSG_CRITICAL,
            		"count_unsent_jobs() failed: %s\n",
            		boincerror(retval));
            exit(retval);
        }
        if (n > CUSHION) { daemon_sleep(10); }
        else {
            int njobs = (CUSHION-n)/REPLICATION_FACTOR;;
            for (int i=0; i<njobs; i++) {
            	// get MapReduce task if available.
            	mrt = get_MapReduce_task(jobs);
            	if(mrt == NULL) { break; }
                retval = make_job(mrt);
                if (retval) {
                    log_messages.printf(
                    		MSG_CRITICAL,
                    		"can't make job: %s\n",
                    		boincerror(retval));
                    exit(retval);
                }
            }
            // Now sleep for a few seconds to let the transitioner
            // create instances for the jobs we just created.
            // Otherwise we could end up creating an excess of jobs.
            daemon_sleep(5);
        }
    }
}

void usage(char *name) {
    fprintf(stderr, "This is an example BOINC work generator.\n"
        "This work generator has the following properties\n"
        "(you may need to change some or all of these):\n"
        "  It attempts to maintain a \"cushion\" of 100 unsent job instances.\n"
        "  (your app may not work this way; e.g. you might create work in batches)\n"
        "- Creates work for the application \"example_app\".\n"
        "- Creates a new input file for each job;\n"
        "  the file (and the workunit names) contain a timestamp\n"
        "  and sequence number, so that they're unique.\n\n"
        "Usage: %s [OPTION]...\n\n"
        "Options:\n"
        "  [ --app X                Application name (default: example_app)\n"
        "  [ --in_template_file     Input template (default: example_app_in)\n"
        "  [ --out_template_file    Output template (default: example_app_out)\n"
    	"  [ --jobtracker_file    	MapReduce state file (default: $PROJECT_HOME/mr/jobtracker.xml)\n"
        "  [ -d X ]                 Sets debug level to X.\n"
        "  [ -h | --help ]          Shows this help text.\n"
        "  [ -v | --version ]       Shows version information.\n",
        name
    );
}

int main(int argc, char** argv) {
    int i, retval;
    char buf[256];

    for (i=1; i<argc; i++) {
    	// TODO - uniformizar teste.
        if (is_arg(argv[i], "d")) {
            if (!argv[++i]) {
                log_messages.printf(
                		MSG_CRITICAL,
                		"%s requires an argument\n\n",
                		argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            int dl = atoi(argv[i]);
            log_messages.set_debug_level(dl);
            if (dl == 4) g_print_queries = true;
        } else if (!strcmp(argv[i], "--app")) {
            app_name = argv[++i];
        } else if (!strcmp(argv[i], "--in_template_file")) {
            in_template_file = argv[++i];
        } else if (!strcmp(argv[i], "--out_template_file")) {
            out_template_file = argv[++i];
        } else if (!strcmp(argv[i], "--jobtracker_file")) {
            jobtracker_file_path = argv[++i];
        } else if (is_arg(argv[i], "h") || is_arg(argv[i], "help")) {
            usage(argv[0]);
            exit(0);
        } else if (is_arg(argv[i], "v") || is_arg(argv[i], "version")) {
            printf("%s\n", SVN_VERSION);
            exit(0);
        }
        else {
            log_messages.printf(
            		MSG_CRITICAL,
            		"unknown command line argument: %s\n\n",
            		argv[i]);
            usage(argv[0]);
            exit(1);
        }
    }

    retval = config.parse_file();
    if (retval) {
        log_messages.printf(
        		MSG_CRITICAL,
        		"Can't parse config.xml: %s\n",
        		boincerror(retval));
        exit(1);
    }

    // Open jobtracker state file.
	jobtracker_file = fopen(jobtracker_file_path, "r+");
    if (!jobtracker_file){
    	log_messages.printf(
    			MSG_CRITICAL,
    			"Error opening jobtracker file (%s).\n",
    			jobtracker_file_path);
    	exit(ERR_FOPEN);
    }

    // Parse jobtracker state file.
    retval = parse_jobtracker(jobtracker_file, jobs);
    if(retval) {
    	log_messages.printf(MSG_CRITICAL, "Error parsing jobtracker file\n");
    	exit(ERR_XML_PARSE);
    }

    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "can't open db\n");
        exit(1);
    }

    sprintf(buf, "where name='%s'", app_name);
    if (app.lookup(buf)) {
        log_messages.printf(MSG_CRITICAL, "can't find app %s\n", app_name);
        exit(1);
    }

    sprintf(buf, "templates/%s", in_template_file);
    if (read_file_malloc(config.project_path(buf), in_template)) {
        log_messages.printf(MSG_CRITICAL, "can't read input template %s\n", buf);
        exit(1);
    }

    log_messages.printf(MSG_NORMAL, "Starting\n");

    main_loop();
}
