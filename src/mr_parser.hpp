#ifndef __MR_PARSER_H__
#define __MR_PARSER_H__

#include <stdio.h>
#include <vector>
#include "boinc/lib/error_numbers.h"
#include "boinc/lib/parse.h"
#include "mr_jobtracker.h"


/**
 * Function that parses a MapReduce tasks (either a map or a reduce task).
 * It receives the file to read from and the job to which the task belong.
 */
int parse_task(FILE* f, MapReduceJob& mpr) {
	char buf[512];
	std::string state;
	unsigned long stateOffset;
	std::string input;
	std::string output;
	while (fgets(buf, 512, f)) {
        if (match_tag(buf, "</map>")) {
        	mpr.addMapTask(MapReduceTask(state, stateOffset, input, output));
        	break;
        }
        else if (match_tag(buf, "</reduce>")) {
        	mpr.addReduceTask(MapReduceTask(state, stateOffset, input, output));
        	break;
        }
        else if (match_tag(buf, "<input>")) { parse_str(buf, "<input>", input); }
        else if (match_tag(buf, "<output>")) { parse_str(buf, "<output>", output); }
        else if (match_tag(buf, "<status>")) {
        	// *ALERT* - magic number 11.
        	stateOffset = ftell(f) - 11;
        	parse_str(buf, "<status>", state);
        }
        else {
        	// TODO - print decent message
        	printf("erro=%s", buf);
        	return ERR_XML_PARSE; }
	}
	return 0;
}

/**
 * Function that parses a MapReduce job (set of map and reduce tasks).
 * It receives the file to read from and the vector of jobs where the new job
 * must be added.
 */
int parse_job(FILE* f, std::vector<MapReduceJob>& jobs) {
	char buf[512];
	int id;

	while (fgets(buf, 512, f)) {
        if (match_tag(buf, "<id>")) {
        	parse_int(buf, "<id>", id);
        	jobs.push_back(MapReduceJob(id));
        }
        else if (match_tag(buf, "<map>")) { parse_task(f, jobs.back()); }
        else if (match_tag(buf, "<reduce>")) { parse_task(f, jobs.back()); }
        else if (match_tag(buf, "</mr>")) { break; }
        else if (match_tag(buf, "</id>")) { continue; }
        else {
        	// TODO - print decent message
        	printf("erro=%s", buf);
        	return ERR_XML_PARSE;
        }
    }

	return 0;
}

/**
 * Function that parses jobtracker state file.
 * It receives a file to read from and a vector of jobs to where jobs should be
 * added.
 * Note: XML files are assumed to have only one open tag per line.
 * Note: XML files are assumed to be well written.
 */
int parse_jobtracker(FILE* f, std::vector<MapReduceJob>& jobs) {
    char buf[512];

    while (fgets(buf, 512, f)) {
        if (match_tag(buf, "<mr>")) {
        	parse_job(f, jobs);
        }
    }

    return 0;
}

#endif
