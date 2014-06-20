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
 * Special node implementation. 
 * A server is a node that delivers MapReduce tasks.
 * It does not perform computation (i.e. does not contribute for the MapReduce
 * tasks).
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
	public Server(int node_id, int upload_rate, MapReduceJob mrj) { 
		super(node_id, upload_rate);
		this.tracker = this;
		this.tracker_table = new HashMap<Integer, LinkedList<Node>>();
		
		// create map tasks
		this.map_tasks = new HashMap<Integer, Task>();
		for(int i = 0; i < mrj.getMapTasks(); i++) {
			int new_id = DATA_ID++;
			this.map_tasks.put(new_id, new Task(
					new_id,	
					mrj.getMapReplFactor(),	
					mrj.getInputSize(), 
					mrj.getIntermSize()));
			// tell tracker that this node (server) has data for this task.
			this.registerData(this, new_id);
		}
		// create reduce tasks
		this.reduce_tasks = new HashMap<Integer, Task>();
		for(int i = 0; i < mrj.getReduceTasks(); i++) {
			int new_id = DATA_ID++;
			this.reduce_tasks.put(new_id, new Task(
					new_id, 
					mrj.getRedReplFactor(), 							
					mrj.getIntermSize(), 
					mrj.getOutputSize()));
		}		
	}
	
	/**
	 * Reduce tasks need special attention because the server needs to fetch
	 * output data from volunteers as soon as a reduce task is done.
	 */
	private void updateReduceTasks() {
		Iterator<Entry<Integer, Task>> it = 
				this.reduce_tasks.entrySet().iterator();
		// for every reduce task,
		while(it.hasNext()) {
			Task task = it.next().getValue();
			// if task is finished and not downloading yet
			if(task.finished() && !this.downloads.containsKey(task.getDataID())) {
				DataTransfer dt = 
						new DataTransfer(task.getDataID(), task.getOutputSize());
				this.downloads.put(task.getDataID(),  dt);
				for(Node uploader : this.tracker.getUploaders(task.getDataID())) {
					uploader.requestDataTransfer(dt);
				}
			}
			
		}
	}
	
	/**
	 * See base doc.
	 */
	@Override
	public void update() {
		this.updateReduceTasks();
		super.update();
		// if all reduce tasks are finished
		if(this.finished(this.reduce_tasks)) {
			
			Iterator<Entry<Integer, DataTransfer>> it =
					this.downloads.entrySet().iterator();
			while(it.hasNext()) {
				// check if any output download is still going,
				if(!it.next().getValue().done()) { break; }
			}
			throw new DoneException();			
		}
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
	 * Searches for an unsent work unit.
	 * Null returned if all work units have already been delivered.
	 * @param map
	 * @return
	 */
	private WorkUnit searchAvailableWork(HashMap<Integer, Task> tasks) {
		Iterator<Entry<Integer, Task>> it = tasks.entrySet().iterator();
		while(it.hasNext()) {
			Task task = it.next().getValue();
			if(task.delivering()) {
				return new WorkUnit(
						task.getDataID(), 
						task.getInputSize(), 
						task.getOutputSize(), 
						this);
			}
		}		
		return null;
	}
	
	/**
	 * Method that goes through a set of tasks to verify if all of them are 
	 * finished.
	 * @param tasks
	 * @return true if all tasks are finished.
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
