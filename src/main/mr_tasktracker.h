#ifndef __MR_TASKTRACKER_H__
#define __MR_TASKTRACKER_H__

#include <stdio.h>

/**
 * TODO - doc.
 */
class TaskTracker {

protected:
	/**
	 * TODO - doc.
	 */
	std::vector<std::string> inputs;
	std::vector<std::string> outputs;
	int nreds;
	int nmaps;
public:

	TaskTracker(int nmaps, int nreds) : nmaps(nmaps), nreds(nreds)  {
		inputs = std::vector<std::string>();
		outputs = std::vector<std::string>();
	}
	virtual int run() = 0;
	virtual ~TaskTracker() { }
};

/**
 * TODO - doc.
 */
class MapTracker : public TaskTracker {

public:

	/**
	 * TODO - doc.
	 */
	MapTracker(
			std::string input,
			std::string output_prefix,
			int nmaps,
			int nreds) : TaskTracker(nmaps, nreds) {
		this->inputs.push_back(input);
		for(int i = 0; i < nreds; i++)
		{ this->outputs.push_back(output_prefix + i); }
	}

	/**
	 * TODO - doc
	 */
	template<typename K, typename V>
	int run(
			void (*map) (
				// Warning: input dependent.
				std::pair<int, std::string>,
				std::multimap<K, V>)) {
		// for every input path
		//  - open file
		//  - for every line
		//  - - call method and store output pair
		// for every key in mod(key nred) -> multimap
		//  - store it in one of the output files.
		return 0;
	}
};

class ReduceTracker : public TaskTracker {

	/**
	 * TODO - doc.
	 */
	ReduceTracker(
			// FIXME - check this.
			std::vector<std::string> inputs,
			std::string output,
			int nmaps,
			int nreds) : TaskTracker(nmaps, nreds), inputs(inputs)
	{ this->outputs.push_back(output); }

	/**
	 * TODO - doc
	 */
	template<typename K, typename V>
	int run(
			void (*reduce)(
					std::multimap<K, V>::iterator it,
					// Warning: output dependent.
					std::multimap<std::string, std::string>)) {
		// for every input path
		//  - open file
		//  - put k,v into a multimap
		// for every key in multimap
		//  - call method and write output pair to output file.
		return 0;
	}
};

void wc_map(
		std::pair<int, std::string> line,
		std::multimap<std::string, int> interm_map) {
	// TODO - break line and count words
}

void wc_reduce(
		std::multimap<std::string, int>::iterator it,
		std::multimap<std::string, std::string> output_map) {
	// TODO - sum the value given by it and add entry to output_map
}



#endif /* MR_TASKTRACKER_H_ */
