/**
 * 
 */
package freeCycles.model;

import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Map.Entry;

/**
 * @author underscore
 * TODO - doc.
 */
public class Server extends Node {
	
	/**
	 * Global current new id. Used to get ids for data (and tasks).
	 */
	private static int DATA_ID = 0;
	
	/**
	 * Map that contains the correspondence between data and nodes.
	 */
	private HashMap<Integer, LinkedList<Node>> tracker_table;
	
	/**
	 * Map of map tasks.
	 */
	private HashMap<Integer, Task> map_tasks;
	
	/**
	 * Map of reduce tasks.
	 */
	private HashMap<Integer, Task> reduce_tasks;

	/**
	 * Constructor. Sets up all tasks.
	 * @param upload_rate
	 */
	public Server(int upload_rate, MapReduceJob mrj) { 
		super(upload_rate);
		this.tracker = this;
		
		// create map tasks
		this.map_tasks = new HashMap<Integer, Task>();
		for(int i = 0; i < mrj.getMapTasks(); i++) {
			int new_id = DATA_ID++;
			this.map_tasks.put(
					new_id, new Task(new_id, mrj.getMapReplFactor()));
			// tell tracker that this node (server) has data for this task.
			this.registerData(this, new_id);
		}
		// create reduce tasks
		this.reduce_tasks = new HashMap<Integer, Task>();
		for(int i = 0; i < mrj.getReduceTasks(); i++) {
			int new_id = DATA_ID++;
			this.reduce_tasks.put(
					new_id, new Task(new_id, mrj.getRedReplFactor()));
		}		
	}
	
	/**
	 * See base doc.
	 */
	@Override
	public void update() {
		// TODO - check if we should ask for reduce data.
		// TODO - check if there is the new to replicate more tasks.
	}
	
	/**
	 * Creates an association between a node and a data id. This simply means 
	 * that the node 'node' contains the data identified by 'data_id'. 
	 */
	private void registerData(Node node, int data_id) {
		if(this.tracker_table.containsKey(data_id)) {
			LinkedList<Node> uploaders = this.tracker_table.get(data_id);
			if(!uploaders.contains(node)) {	uploaders.add(node); }
		} else {
			LinkedList<Node> uploaders = new LinkedList<Node>();
			uploaders.add(node);
			this.tracker_table.put(data_id, uploaders);
		}
	}
	
	/**
	 * TODO - implement!
	 * Null if returned if: all work units have been delivered.
	 * @param map
	 * @return
	 */
	private WorkUnit searchAvailableWork(HashMap<Integer, Task> tasks) {
		Iterator<Entry<Integer, Task>> it = tasks.entrySet().iterator();
		while(it.hasNext()) {
			Task task = it.next().getValue();
			if(task.delivering()) {
				return new WorkUnit(); // TODO - complete, Task impl is missing the input and output sizes.
			}
		}		
		return null;
	}
	
	/**
	 * TODO - doc!
	 * @param tasks
	 * @return
	 */
	private boolean finished(HashMap<Integer, Task> tasks) {
		Iterator<Entry<Integer, Task>> it = tasks.entrySet().iterator();
		while(it.hasNext()) {
			if(!it.next().getValue().finished()) { return false; }
		}
		return true;
	}
	
	/**
	 * Request for work. Returns 'null' if there is no available task.
	 * @return
	 */
	public WorkUnit requestWork() {
		WorkUnit wu = this.searchAvailableWork(this.map_tasks);
		
		// if there is a map work to deliver,
		if(wu != null) { return wu;	}
		
		// if we are still waiting for map results,
		if(!this.finished(this.map_tasks)) { return null; }
		
		// if we are in the reduce phase,
		return this.searchAvailableWork(this.reduce_tasks);
	}
	
	/**
	 * Register a new finished task.
	 * @param node
	 * @param data_id
	 */
	public void workFinished(Node node, int data_id) {
		// check if it is a map task,
		if(this.map_tasks.containsKey(data_id)) {
			this.map_tasks.get(data_id).newResult(node);
		}
		// otherwise, it is a reduce task,
		else {
			this.reduce_tasks.get(data_id).newResult(node);
		}
		this.registerData(node, data_id);
	}
	
	/**
	 * Returns the list of uploaders for a particular data id.	
	 * @param data_id
	 * @return
	 */
	public LinkedList<Node> getUploaders(Integer data_id) {
		return this.tracker_table.containsKey(data_id) ? 
				this.tracker_table.get(data_id) :
				new LinkedList<Node>();
	}

}
