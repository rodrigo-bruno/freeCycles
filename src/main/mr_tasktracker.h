#ifndef __MR_TASKTRACKER_H__
#define __MR_TASKTRACKER_H__

/**
 * This file contains a simple and straightforward implementation of MapReduce.
 * Input data is read from a file (see run at MapTracker for details).
 * Intermediate data is placed inside maps (these are also used for sorting).
 * All intermediate date is stored as std::strings (this is just a to facilitate
 * this initial implementation).
 * Output data is written to a file.
 */

#include <stdio.h>
#include <stdlib.h>

#include <sstream>
#include <map>
#include <vector>
#include <string>


#include "data_handler.h"

using std::vector;
using std::map;
using std::string;

/**
 * Simple implementation for the word count map task.
 * The function receives:
 *  - key (current file pointer)
 *  - line (read from input file (it may include a '\n' at the end))
 *  - imap (where output data should be placed).
 */
void wc_map(
		int k, string v, vector<std::map<string, vector<string> > >* imap) {
	string token;
	int red = 0;

	// remove trailing new line if exists.
	if(*(--v.end()) == '\n') { v.erase(--v.end()); }
	std::istringstream iss(v);
	// For every word in line, apply the modulo to the first character and
	// insert some place holder inside a map.
	while(getline(iss, token, ' '))
	{ (*imap)[*token.c_str() % imap->size()][token].push_back("1"); }
}

/**
 * Implementation for the word count map task.
 * The function receives:
 *  - key
 *  - values
 *  - omap (where output data is stored until it is written to file).
 */
void wc_reduce(
		string k, vector<string> v, std::map<string, vector<string> >* omap) {
	std::stringstream ss;
	ss << v.size();
	(*omap)[k] = vector<string>(1, ss.str());
}

/**
 * This class implements the task execution functionality inherent to a
 * MapReduce job.
 * This is a base class and therefore, one of the subclasses (MapTracker or
 * ReduceTracker) should be used.
 * In order to use this implementation, one should only provide a map or reduce
 * function and the number of mappers and reducers in the current job.
 */
class TaskTracker {

protected:
	/**
	 * Vector of input file paths.
	 */
	vector<string> inputs;
	/**
	 * Vector of output file paths.
	 */
	vector<string> outputs;
	/**
	 * Number of mappers in the current job.
	 */
	int nmaps;
	/**
	 * Number of reducers in the current job.
	 */
	int nreds;

public:
	TaskTracker(int nmaps, int nreds) : nmaps(nmaps), nreds(nreds)  {
		inputs = vector<string>();
		outputs = vector<string>();
	}
	/**
	 * Method that implements the map stage in a MapReduce workflow.
	 * It receives a function that will be used to process each <K,V> read from
	 * the input file.
	 */
	int map(void (*map_func) (
			int k, string v, vector<std::map<string, vector<string> > >* io)) {
        char* line = NULL;
        size_t len = 0;
        std::map<string, vector<string> >::iterator mit;
        vector<string>::iterator vit;
        vector<std::map<string, vector<string> > > io =
        		vector<std::map<string, vector<string> > >(this->nreds);

        // Open file (map tasks are assumed to have only one input file).
		FILE* f = fopen(this->inputs.front().c_str(), "r");
        if (f == NULL) {
            fprintf(stderr,
            		"[MR-map] failed to open file %s.\n",
            		this->inputs.front().c_str());
            return 1;
        }

        // For every line, call map function.
        while (getline(&line, &len, f) != -1)
        { map_func(ftell(f), std::string(line), &io); }
        // Cleanup.
        free(line);
        fclose(f);
        // For every std::map, write intermediate data to file.
        for(int i = 0; i < io.size(); i++)
        { writeData(this->outputs[i], &(io.at(i))); }
        return 0;
	}
	/**
	 * Method that implements the reduce stage in a MapReduce workflow.
	 * It receives a function that will be used to process each intermediate
	 * <K,V>.
	 */
	int reduce(void (*reduce_func) (
			string k, vector<string> v,	std::map<string, vector<string> >* o)) {
		vector<string>::iterator vit;
		std::map<string, vector<string> >::iterator mit;
		std::map<string, vector<string> > imap;
		std::map<string, vector<string> > omap;

		// For every input path, open file and load key,values.
		for(vit = this->inputs.begin(); vit != this->inputs.end(); vit++)
		{ this->readData(*vit, &imap); }
		// For every <K,V> pair, call reduce function.
		for(mit = imap.begin(); mit != imap.end(); mit++)
		{ reduce_func(mit->first, mit->second, &omap); }
		this->writeData(this->outputs.front(), &omap);
		return 0;
	}

	virtual ~TaskTracker() { }
	std::vector<std::string>* getInputs() { return &this->inputs; }
	std::vector<std::string>* getOutputs() { return &this->outputs; }

	/**
	 * Auxiliary method that reads an input file with <K,V> pairs and loads
	 * them into a map structure.
	 */
	int readData(string path, std::map<string, vector<string> >* data) {
        char *line = NULL, *ptr=NULL;
        FILE* f = NULL;
        size_t len = 0, start = 0, end = 0;
		string key, buf;

		if(!(f = fopen(path.c_str(), "r"))) {
            fprintf(stderr,
            		"[MR-readData] failed to open file %s.\n",
            		path.c_str());
            return 1;
		}

		// For every line, extract <K (string), V (string vector)>
		while(getline(&line, &len, f) != -1) {
			vector<string> values = vector<string>();
			buf = string(line);
			end = buf.find("=", (start = 0));
			key = buf.substr(start, end - start);
			end = buf.find(";", (start = end + 1));
			while(end != string::npos) {
				values.push_back(buf.substr(start, end - start));
				end = buf.find(";", (start = end + 1));
			}
			if(data->find(key) != data->end()) {	// TODO - fix
				(*data)[key].insert(
						(*data)[key].end(), values.begin(), values.end());
			}
			else
			{ (*data)[key] = values; }
		}
		fclose(f);
		free(line);
		return 0;
	}

	/**
	 * Auxiliary method that writes <K,V> pairs into an output file.
	 */
	int writeData(string path, std::map<string, vector<string> >* data) {
		std::map<string, vector<string> >::iterator mit;
		vector<string>::iterator vit;
		FILE* f = NULL;

		if(!(f = fopen(path.c_str(), "w"))) {
            fprintf(stderr,
            		"[MR-readData] failed to open file %s.\n",
            		path.c_str());
		}

    	// For every <K,V>, write it to file
    	for(mit = (*data).begin(); mit != (*data).end(); mit++) {
    		fprintf(f, "%s=",mit->first.c_str());
    		// For every V corresponding to the same K
    		for(vit = mit->second.begin(); vit != mit->second.end(); vit++) {
    			fprintf(f, "%s;", vit->c_str());
    		}
    		fprintf(f, "\n");
    	}
    	fclose(f);
    	return 0;
	}
};

/**
 * Class responsible for running Map tasks.
 */
class MapTracker : public TaskTracker {

public:
	/**
	 * This constructor is responsible for initializing both input and output
	 * vectors.
	 */
	MapTracker(DataHandler* dh, string output_prefix, int nmaps, int nreds) :
			TaskTracker(nmaps, nreds) {
		char buf[64];
		this->inputs.push_back(string());
		dh->get_input(this->inputs[0]);
		for(int i = 0; i < nreds; i++) {
			sprintf(buf,"%d",i);
			this->outputs.push_back(output_prefix + std::string(buf));
		}
	}
};

/**
 * Class responsible for running Reduce tasks.
 */
class ReduceTracker : public TaskTracker {

public:
	/**
	 * This constructor is responsible for initializing both input and output
	 * vectors.
	 */
	ReduceTracker(DataHandler* dh, string output, int nmaps, int nreds) :
			TaskTracker(nmaps, nreds) {
		dh->get_zipped_input(this->inputs);
		this->outputs.push_back(output);
	}
};


#endif /* MR_TASKTRACKER_H_ */
