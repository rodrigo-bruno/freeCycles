#include <stdlib.h>
#include <unistd.h>
#include "mr_parser.h"
#include "mr_jobtracker.h"


#define JOBTRACKER_FILE "/tmp/jobtracker.xml"

int main(int argc, char** argv) {
    int retval;
    std::vector<MapReduceJob> jobs;
    std::vector<MapReduceJob>::iterator it;

	FILE* f = fopen(JOBTRACKER_FILE, "r+");
    if (!f){
    	printf("Error openning file.\n");
    	return ERR_FOPEN;
    }

    retval = parse_jobtracker(f, jobs);
    if(retval) {
    	printf("Error parsing file.\n");
    	return ERR_XML_PARSE;
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

