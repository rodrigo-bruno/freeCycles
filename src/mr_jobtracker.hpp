#ifndef __MR_JOBTRACKER_H__
#define __MR_JOBTRACKER_H__

#include <vector>
#include <stdio.h>

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
		fprintf(io,
				"\tstate=%s, input=%s, output=%s, offset=%lu\n",
				this->state.c_str(),
				this->input.c_str(),
				this->output.c_str(),
				this->stateOffset);
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

#endif
