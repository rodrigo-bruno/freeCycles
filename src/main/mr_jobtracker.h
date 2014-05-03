#ifndef __MR_JOBTRACKER_H__
#define __MR_JOBTRACKER_H__

#include <vector>
#include <stdio.h>
#include <string>

#define TASK_WAITING "w"
#define TASK_CREATED "c"
#define TASK_FINISHED "f"

/**
 * A MapReduce tasks is either a map or a reduce task.
 */
class MapReduceTask {

protected:
	/**
	 * Unique task identifier.
	 */
	std::string name;
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
	unsigned long state_offset;
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
			std::string task_name,
			std::string task_state,
			int task_offset,
			std::string task_input,
			std::string task_output) :
		name(task_name),
		state(task_state),
		state_offset(task_offset),
		input(task_input),
		output(task_output) {}
	void setState(std::string task_state) { this->state = task_state; }
	unsigned long getStateOffset() { return this->state_offset; }
	const std::string& getState() { return this->state; }
	const std::string& getInputPath() { return this->input; }
	const std::string& getOutputPath() { return this->output; }
	const std::string& getName() { return this->name; }
	/**
	 * Dumps the current task state.
	 */
	void dump(FILE* io) {
		fprintf(io,
				"\tname=%s, state=%s, input=%s, output=%s, offset=%lu\n",
				this->name.c_str(),
				this->state.c_str(),
				this->input.c_str(),
				this->output.c_str(),
				this->state_offset);
	}
};

/**
 * A MapReduce job comprehends a set of map tasks and a set of reduce tasks.
 */
class MapReduceJob {

protected:
	/**
	 * Unique job identifier.
	 */
	std::string id;
	/**
	 * Map tasks.
	 */
	std::vector<MapReduceTask> maps;
	/**
	 * Reduce tasks.
	 */
	std::vector<MapReduceTask> reds;
	/**
	 * Index (within maps vector) of the next map task to send.
	 */
	unsigned int next_map;
	/**
	 * Index (within reds vector) of the next reduce task to send.
	 */
	unsigned int next_red;
	/**
	 * Number of finished map tasks.
	 */
	unsigned int finished_map;
	/**
	 * False if all the map and reduce tasks have already been sent.
	 */
	bool unsent_tasks;
	/**
	 * True if the job is already shuffled.
	 */
	bool shuffled;
	/**
	 * This shuffled offset is used when we need to change the state file.
	 * This offset is used to place the file pointer pointing to the offset
	 * byte.
	 */
	int shuffled_offset;

public:
	MapReduceJob(std::string job_id) :
		id(job_id),
		next_map(0),
		next_red(0),
		finished_map(0),
		unsent_tasks(true),
		shuffled(false),
		shuffled_offset(-1) {}
	std::vector<MapReduceTask>& getMapTasks() { return this->maps; }
	std::vector<MapReduceTask>& getReduceTasks() { return this->reds; }
	/**
	 * This method searches for an unsent map task. It returns an unsent map
	 * task or NULL if there isn't one (all map tasks are created or finished).
	 */
	MapReduceTask* getNextMap() {
		for(; this->next_map < this->maps.size(); this->next_map++) {
			if (!this->maps[this->next_map].getState().compare(TASK_WAITING))
			{ return &(this->maps[this->next_map]); }
		}
		return NULL;
	}
	/**
	 * This method searches for an unsent reduce task. It returns an unsent
	 * reduce task or NULL if there isn't one (all reduce tasks are created or
	 * finished) or if there are unfinished map tasks.
	 */
	MapReduceTask* getNextReduce() {
		for(; this->finished_map < this->maps.size(); this->finished_map++) {
			if (this->maps[this->finished_map].getState().compare(TASK_FINISHED))
			{ return NULL;}
		}
		for(; this->next_red < this->reds.size(); this->next_red++) {
			if (!this->reds[this->next_red].getState().compare(TASK_WAITING))
			{ return &(this->reds[this->next_red]); }
		}
		this->unsent_tasks = false;
		return NULL;
	}
	/**
	 * Method to test if the job needs to be shuffled.
	 */
	bool needShuffle() {
		return !shuffled ? this->finished_map == this->maps.size() : false;
	}
	void setShuffled(bool new_shuffled) { this->shuffled = new_shuffled; }
	void setShuffledOffset(int new_shuffled_offset)
	{ this->shuffled_offset = new_shuffled_offset; }
	int getShuffledOffset() { return this->shuffled_offset; }
	std::string getID() { return this->id; }
	void addMapTask(const MapReduceTask& mrt) { this->maps.push_back(mrt); }
	void addReduceTask(const MapReduceTask& mrt) { this->reds.push_back(mrt); }
	bool hasUnsentTasks() { return this->unsent_tasks; }
	/**
	 * Dumps the current job state.
	 */
	void dump(FILE* io) {
		fprintf(io,
				"MapReduceJob: id=%s, shuffled=%d\n",
				this->id.c_str(),
				this->shuffled);
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
