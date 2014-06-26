package freeCycles.model;

import java.util.HashMap;
import java.util.LinkedList;

/**
 * Class representing a replicated task.
 * In practise, a task will originate several workunits (replicas of 
 * computation).
 *
 */
public class Task {
	
	/**
	 * Task id.
	 */
	private int task_id;
	
	/**
	 * Input data ids.
	 */
	private LinkedList<Integer> idata_ids;
	
	/**
	 * Output data ids.
	 */
	private LinkedList<Integer> odata_ids;
	 
	/**
	 * Replication factor.
	 */
	private int replication;
	
	/**
	 * Mapping between nodes working on this task and the delivery timestamp.
	 */
	private HashMap<Node, Integer> workers;
	
	/**
	 * Mapping between finished nodes and completion timestamp.
	 */
	private HashMap<Node, Integer> results;
	
	/**
	 * Input size per id (total input size = idata_ids.size * input_size).
	 */
	private int input_size;
	
	/**
	 * Output size per id (total output size = odata_ids.size * output_size).
	 */
	private int output_size;
	
	/**
	 * Constructor.
	 * @param task_id
	 * @param idata_ids
	 * @param odata_ids
	 * @param replication
	 * @param input_size
	 * @param output_size
	 */
	public Task(
			int task_id,
			LinkedList<Integer> idata_ids,
			LinkedList<Integer> odata_ids,
			int replication, 
			int input_size, 
			int output_size) {
		this.task_id = task_id;
		this.idata_ids = idata_ids;
		this.odata_ids = odata_ids;
		this.replication = replication;
		this.input_size = input_size;
		this.output_size = output_size;
		this.workers = new HashMap<Node, Integer>();
		this.results = new HashMap<Node, Integer>();
	}
	
	/**
	 * New worker for this task. 
	 * @param node
	 * @return
	 */
	void newWorker(Node node) {	this.workers.put(node, Main.getTime());	}
	
	/**
	 * Returns true if the node is already registered as worker for this task.
	 * @param node
	 * @return
	 */
	boolean hasWorker(Node node) { return this.workers.containsKey(node); }
	
	/**
	 * New result from node 'node'. 
	 * Returns false if result is already on the list.
	 * @param node
	 * @return
	 */
	boolean newResult(Node node) {
		if(this.results.containsKey(node)) { return false; }
		else { this.results.put(node, Main.getTime()); return true; }
	}
	
	/**
	 * @return true if task is finished.
	 */
	boolean finished() {
		return this.results.size() >= this.replication;
	}
	
	/**
	 * @return false if the task still accepts new workers.
	 */
	boolean delivering() {
		return this.workers.size() < this.replication;
	}
	
	public int getTaskID() { return this.task_id; }
	public LinkedList<Integer> getInputDataIDs() { return this.idata_ids; }
	public LinkedList<Integer> getOutputDataIDs() { return this.odata_ids; }
	public int getInputSize() { return this.input_size; }
	public int getOutputSize() { return this.output_size; }
	public HashMap<Node, Integer> getWorkers() { return this.workers; }
}
