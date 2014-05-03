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

// A simple assimilator that:
// 1) if success, copy the output file(s) to a directory
// 2) if failure, append a message to an error log

#include <vector>
#include <string>
#include <cstdlib>

#include "boinc_db.h"
#include "error_numbers.h"
#include "filesys.h"
#include "sched_msgs.h"
#include "validate_util.h"
#include "sched_config.h"

#include "mr_jobtracker.h"
#include "mr_parser.h"

const char* jobtracker_file_path = "/home/boincadm/projects/test4vm/mr/jobtracker.xml";
FILE* jobtracker_file = NULL;
std::vector<MapReduceJob> jobs;

int write_error(char* p) {
    static FILE* f = 0;
    if (!f) {
        f = fopen(config.project_path("sample_results/errors"), "a");
        if (!f) return ERR_FOPEN;
    }
    fprintf(f, "%s", p);
    fflush(f);
    return 0;
}

void debug(const char* what, const char* with) {
	char buf[1024];
	sprintf(buf, "%s %s\n", what, with);
	write_error(buf);
}

/**
 * This helper function searches for a MapReduce task which is identified by
 * the given 'wu_name'.
 * Note: Task names follow the format: id-[map|reduce]-seq.number
 */
MapReduceTask* get_task_by_name(std::vector<MapReduceJob>& jobs_ref, char* wu_name) {
	std::string id = std::string(wu_name, strchr(wu_name, '-')  - wu_name);
	std::vector<MapReduceJob>::iterator it;
	std::vector<MapReduceTask>::iterator it2;

	debug("get_task_by_name", wu_name);

	for(it = jobs_ref.begin(); it != jobs_ref.end(); ++it) {
		debug("job", id.c_str());
		if (it->getID().compare(id)) { continue; }
		// Search wu_name among map tasks
		for(	it2 = it->getMapTasks().begin();
				it2 != it->getMapTasks().end();
				++it2)
			{
			debug("map task", it2->getName().c_str());
			if(!strcmp(wu_name, it2->getName().c_str())) { return &(*it2); }
			}
		// Search wu_name among reduce tasks
		for(	it2 = it->getReduceTasks().begin();
				it2 != it->getReduceTasks().end();
				++it2)
			{
			debug("reduce task", it2->getName().c_str());
			if(!strcmp(wu_name, it2->getName().c_str())) { return &(*it2); }
			}
	}
	return NULL;
}

/**
 * Helper function that writes to the jobtracker state file.
 * It is called when a task is assimilated.
 */
void write_task_state(MapReduceTask* mrt) {
	debug("write_task_state", mrt->getName().c_str());
	fseek(jobtracker_file, mrt->getStateOffset(),SEEK_SET);
	fwrite(TASK_FINISHED, 1, 1, jobtracker_file);
	fflush(jobtracker_file);
	mrt->setState(TASK_FINISHED);
}

int assimilate_handler(
		WORKUNIT& wu,
		std::vector<RESULT>& /*results*/,
		RESULT& canonical_result) {
    int retval;
    char buf[1024];
    unsigned int i;
    MapReduceTask* mrt = NULL;

    // First time initialization (loads jobtracker state).
    if(jobtracker_file == NULL) {
		// Open jobtracker state file.
		jobtracker_file = fopen(jobtracker_file_path, "r+");
		if (!jobtracker_file){
			sprintf(buf, "Can't jobtracker file (%s).\n", jobtracker_file_path);
			return write_error(buf);
		}
		// Parse jobtracker state file.
		retval = parse_jobtracker(jobtracker_file, jobs);
		if(retval) {
			sprintf(buf, "Error parsing jobtracker file\n");
			return write_error(buf);
		}
    }

    retval = boinc_mkdir(config.project_path("sample_results"));
    if (retval) return retval;

    if (wu.canonical_resultid) {
        std::vector<OUTPUT_FILE_INFO> output_files;
        const char *copy_path;
        get_output_file_infos(canonical_result, output_files);
        unsigned int n = output_files.size();
        bool file_copied = false;

        // Get the task identified by the work unit name.
        mrt = get_task_by_name(jobs, wu.name);
        if (mrt == NULL) {
			sprintf(buf, "Can't find MapRedureTask %s\n", wu.name);
			//return write_error(buf);
			write_error(buf);
        }
        // Update: 1)state file and 2) in memory structure (jobs)
		if (mrt != NULL) { write_task_state(mrt); }

    	// TODO - both map and reduces will only have one output.
        for (i=0; i<n; i++) {
            OUTPUT_FILE_INFO& fi = output_files[i];
            if (n==1) {
            	// FIXME - maybe we should use the value writen on the mr xml file.
            	// FIXME - if wu.name contains reduce, also copy to bt new.
                copy_path = config.project_path("sample_results/%s", wu.name);
            } else {
                copy_path = config.project_path("sample_results/%s_%d", wu.name, i);
            }

            retval = boinc_copy(fi.path.c_str() , copy_path);
            if (!retval) { file_copied = true; }

        }
        // TODO - delete?
        if (!file_copied) {
            copy_path = config.project_path("sample_results/%s_%s", wu.name, "no_output_files");
            FILE* f = fopen(copy_path, "w");
            fclose(f);
        }
    } else {
        sprintf(buf, "%s: 0x%x\n", wu.name, wu.error_mask);
        return write_error(buf);
    }
    return 0;
}
