package freeCycles.model;

import java.util.HashMap;

public class Task {
	
	/**
	 * UID.
	 */
	public int data_id;
	
	/**
	 * Replication factor.
	 */
	public int replication;
	
	/**
	 * Mapping between nodes working on this task and the delivery timestamp.
	 */
	HashMap<Node, Integer> workers;
	
	/**
	 * Mapping between finished nodes and completion timestamp.
	 */
	HashMap<Node, Integer> results;
	
	/**
	 * Input size.
	 */
	int input_size;
	
	/**
	 * Output size.
	 */
	int output_size;
	
	/**
	 * Constructor.
	 * @param data_id
	 * @param replication
	 */
	public Task(int data_id, int replication, int input_size, int output_size) {
		this.data_id = data_id;
		this.replication = replication;
		this.input_size = input_size;
		this.output_size = output_size;
		this.workers = new HashMap<Node, Integer>();
		this.results = new HashMap<Node, Integer>();
	}
	
	/**
	 * New worker for this task. 
	 * Returns false if worker is not acceptet (might already be in the worker 
	 * list).
	 * @param node
	 * @return
	 */
	boolean newWorker(Node node) {
		if(this.workers.containsKey(node)) { return false; }
		else { this.workers.put(node, Main.getTime()); return true; }
	}
	
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
	
	public int getDataID() { return this.data_id; }
	public int getInputSize() { return this.input_size; }
	public int getOutputSize() { return this.output_size; }
}
