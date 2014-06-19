package freeCycles.model;

/**
 * Class describing a MapReduce job with several parameters.
 * @author underscore
 *
 */
public class MapReduceJob {

	/**
	 * Number of map tasks (before replication).
	 */
	public int map_tasks;
	
	/**
	 * Number of reduce tasks (before replication).
	 */
	public int reduce_tasks;
	
	/**
	 * Size of input data (MBs).
	 */
	public int input_size;
	
	/**
	 * Size of intermediate data (MBs).
	 */
	public int interm_size;
	
	/**
	 * Size of output data (MBs).
	 */
	public int output_size;
	
	/**
	 * Replication factor for map tasks.
	 */
	public int map_repl_factor;
	
	/**
	 * Replication factor for reduce tasks.
	 */
	public int red_repl_factor;
	
	public MapReduceJob(
			int map_tasks,
			int reduce_tasks,
			int input_size,
			int interm_size,
			int output_size,
			int map_repl_factor,
			int red_repl_factor) {
		this.map_tasks = map_tasks;
		this.reduce_tasks = reduce_tasks;
		this.input_size = input_size;
		this.interm_size = interm_size;
		this.output_size = output_size;
		this.map_repl_factor = map_repl_factor;
		this.red_repl_factor = red_repl_factor;
	}

	/**
	 * Getters' party.
	 */
	public int getMapTasks() { return this.map_tasks; }
	public int getReduceTasks() { return this.reduce_tasks; }
	public int getInputSize() { return this.input_size; }
	public int getIntermSize() { return this.interm_size; }
	public int getOutputSize() { return this.output_size; }
	public int getMapReplFactor() { return this.map_repl_factor; }
	public int getRedReplFactor() { return this.red_repl_factor; }
	
}
