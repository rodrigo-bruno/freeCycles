#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include "boinc/lib/error_numbers.h"
#include "boinc/lib/parse.h"

#define JOBTRACKER_FILE "/tmp/jobtracker.xml"

/**
 * A MapReduce tasks is either a map or a reduce task.
 */
class MapReduceTask {

protected:
	/**
	 * State can be one of the following values:
	 * - "w" (if the task is waiting to be created)
	 * - "c" (if the task is already created)
	 * - "f" (if the task is finished)
	 */
	std::string state;
	/**
	 * This state offset is used when we need to change the state of the task.
	 * This offset is used to place the file pointer pointing to the offset
	 * byte.
	 */
	unsigned long stateOffset;
	/**
	 * Input file path.
	 */
	std::string input;
	/**
	 * Output file path.
	 */
	std::string output;

public:
	MapReduceTask(
			std::string state,
			int offset,
			std::string input,
			std::string output) :
		state(state), stateOffset(offset), input(input), output(output) {}
	std::string getState() { return this->state; }
	void setState(std::string state) { this->state = state; }
	unsigned long getStateOffset() { return this->stateOffset; }
	const std::string& getInputPath() { return this->input; }
	const std::string& getOutputPath() { return this->output; }
	/**
	 * Dumps the current task state.
	 */
	void dump(FILE* io) {
		fprintf(io, "\tstate=%s, input=%s, output=%s, offset=%lu\n",
				this->state.c_str(), this->input.c_str(), this->output.c_str(), this->stateOffset);
	}
};

/**
 * A MapReduce job comprehends a set of map tasks and a set of reduce tasks.
 */
class MapReduceJob {

protected:
	/**
	 * Map tasks.
	 */
	std::vector<MapReduceTask> maps;
	/**
	 * Reduce tasks.
	 */
	std::vector<MapReduceTask> reds;
	/**
	 * Unique identifier.
	 */
	int id;

public:
	MapReduceJob(int id) : id(id) {}
	std::vector<MapReduceTask>& getMapTasks() { return this->maps; }
	std::vector<MapReduceTask>& getReduceTasks() { return this->reds; }
	void addMapTask(const MapReduceTask& mrt) { this->maps.push_back(mrt); }
	void addReduceTask(const MapReduceTask& mrt) { this->reds.push_back(mrt); }
	/**
	 * Dumps the current job state.
	 */
	void dump(FILE* io) {
		fprintf(io,"MapReduceJob: id=%d\n", this->id);
		// print map tasks
		fprintf(io,"Map Tasks:\n");
		for(	std::vector<MapReduceTask>::iterator it = this->maps.begin();
				it != this->maps.end();
				++it) { it->dump(io); }
		// print reduce tasks
		fprintf(io,"Reduce Tasks:\n");
		for(	std::vector<MapReduceTask>::iterator it = this->reds.begin();
				it != this->reds.end();
				++it) { it->dump(io); }
	}
};

/**
 * TODO - doc.
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
        	// *ALERT* - magic number 10.
        	stateOffset = ftell(f) - 11;
        	parse_str(buf, "<status>", state);
        }
        else {
        	printf("erro=%s", buf);
        	return ERR_XML_PARSE; }
	}
	return 0;
}

/**
 * TODO - doc.
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
        else { return ERR_XML_PARSE; }
    }

	return 0;
}

/**
 * TODO - doc.
 * XML files are assumed to have only one tag per line.
 * I assume that all jobtracker XML files are well written.
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


int main(int argc, char** argv) {
    int retval;
    std::vector<MapReduceJob> jobs;
    std::vector<MapReduceJob>::iterator it;

	FILE* f = fopen(JOBTRACKER_FILE, "r+");
    if (!f){
        // TODO - print message
        return ERR_FOPEN;
    }

    retval = parse_jobtracker(f, jobs);
    if(!retval) {
    	// TODO - print message
    }

    // Dump state.
	for( it = jobs.begin(); it != jobs.end(); ++it) { it->dump(stdout); }

	// Put every map task in the finished state.
	for( it = jobs.begin(); it != jobs.end(); ++it) {
		std::vector<MapReduceTask>& maps = it->getMapTasks();
		std::vector<MapReduceTask>::iterator it2;
		for( it2 = maps.begin(); it2 != maps.end();	++it2) {
			fseek(f, it2->getStateOffset(),SEEK_SET);
			fwrite("f", 1, 1, f);
			it2->setState("f");
		}
	}


	// Get map tasks state.
	for( it = jobs.begin(); it != jobs.end(); ++it) {
		std::vector<MapReduceTask>& maps = it->getMapTasks();
		std::vector<MapReduceTask>::iterator it2;
		for( it2 = maps.begin(); it2 != maps.end();	++it2) {
			char state[2];
			fseek(f, it2->getStateOffset(), SEEK_SET);
			fread(&state, 1, 1, f);
			state[1] = '\0';
			it2->setState(std::string(state));
		}
	}
	// Dump state.
	for( it = jobs.begin(); it != jobs.end(); ++it) { it->dump(stdout); }
	sleep(2);


	fclose(f);

	return 0;
}




