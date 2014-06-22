package freeCycles.model;

import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Map.Entry;

/**
 * Special node implementation. 
 * A server is a node that delivers MapReduce tasks.
 * It does not perform computation (i.e. does not contribute for the MapReduce
 * tasks).
 */
public class Server extends Node {
	
	/**
	 * Global current new data id.
	 */
	private static int DATA_ID = 0;
	
	/**
	 * Global current new task id.
	 */
	private static int TASK_ID = 0;
	
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
	public Server(int node_id, float upload_rate, MapReduceJob mrj) { 
		super(node_id, upload_rate);
		this.tracker = this;
		this.tracker_table = new HashMap<Integer, LinkedList<Node>>();
	
		// prepare data IDs
		int[] input_ids = this.prepareInputIDs(mrj);
		int[][] interm_ids = this.prepareIntermDataIDs(mrj);
		int[] output_ids = this.prepareOutputIDs(mrj);
		
		// create map tasks
		this.map_tasks = new HashMap<Integer, Task>();
		for(int i = 0; i < mrj.getMapTasks(); i++) {
			int task_id = TASK_ID++;
			int input_id = input_ids[i];
			
			// add new task to map of map tasks.
			this.map_tasks.put(
					task_id, 
					this.prepareMapTask(task_id, input_id, interm_ids, mrj));
			// tell tracker that this node (server) has data for this task.
			this.registerUploader(this, input_id);
			// register input data on the server (and advance it to completion).
			this.downloads.put(
					input_id, new DataTransfer(this.node_id, input_ids[i], mrj.getInputSize()));
			this.downloads.get(input_id).advance(this.node_id, mrj.getInputSize());
			
			Main.log("[Node 0] - new map task with id " + task_id);
			
		}
		// create reduce tasks
		this.reduce_tasks = new HashMap<Integer, Task>();
		for(int i = 0; i < mrj.getReduceTasks(); i++) {
			int task_id = TASK_ID++;
			int output_id = output_ids[i];
			// add new task to map of reduce tasks.
			this.reduce_tasks.put(
					task_id, 
					this.prepareReduceTask(task_id, output_id, interm_ids, mrj));
			Main.log("[Node 0] - new reduce task with id " + task_id);
		}		
	} 
	
	/**
	 * TODO - doc.
	 * @param task_id
	 * @param input_id
	 * @param interm_ids
	 * @param mrj
	 * @return
	 */
	Task prepareMapTask(
			int task_id, int input_id, int[][] interm_ids, MapReduceJob mrj) {
		LinkedList<Integer> idata_ids = new LinkedList<Integer>();
		LinkedList<Integer> odata_ids = new LinkedList<Integer>();
		
		idata_ids.add(input_id);
		for(int id : interm_ids[task_id]) {	odata_ids.add(id); }
		
		return new Task(
				task_id, 
				idata_ids, 
				odata_ids, 
				mrj.getMapReplFactor(), 
				mrj.getInputSize(), 
				mrj.getIntermSize());
	}
	
	/**
	 * TODO - doc.
	 * @param task_id
	 * @param output_id
	 * @param interm_ids
	 * @param mrj
	 * @return
	 */
	Task prepareReduceTask(
			int task_id, int output_id, int[][] interm_ids, MapReduceJob mrj) {
		LinkedList<Integer> idata_ids = new LinkedList<Integer>();
		LinkedList<Integer> odata_ids = new LinkedList<Integer>();
		
		odata_ids.add(output_id);
		for(int i = 0; i < mrj.getMapTasks(); i++) {
			// FIXME - this task_id - number of map tasks is a hack.
			idata_ids.add(interm_ids[i][task_id - mrj.getMapTasks()]);
		}
		
		return new Task(
				task_id, 
				idata_ids, 
				odata_ids, 
				mrj.getRedReplFactor(), 
				mrj.getIntermSize(), 
				mrj.getOutputSize());
	}
	
	/**
	 * Auxiliary method to prepare data ids.
	 * @param number_ids
	 * @return
	 */
	int[] prepareIDs(int number_ids) {
		int[] array = new int[number_ids];
		for(int i = 0; i < number_ids; i++) { array[i] = DATA_ID++; }
		return array;
	}
	
	/**
	 * TODO - doc.
	 * @param mrj
	 * @return
	 */
	int[] prepareInputIDs(MapReduceJob mrj) {
		return this.prepareIDs(mrj.getMapTasks());
	} 

	/**
	 * TODO - doc.
	 * @param mrj
	 * @return
	 */
	int[] prepareOutputIDs(MapReduceJob mrj) {
		return this.prepareIDs(mrj.getReduceTasks());
	} 
	
	/**
	 * Method that prepares data ids for map and reduce tasks.
	 * The returned matrix helps to shuffle intermediate data ids. 
	 * The returned matrix should be something like this:
	 * [map 1 output 1, reduce 1 input 1] ... [map N output 1, reduce 1 input N]
	 * ...
	 * [map 1 output N, reduce N input 1] ... [map N output N, reduce N input N]
	 * @param mrj
	 * @return
	 */
	int[][] prepareIntermDataIDs(MapReduceJob mrj) {
		int[][] ids = new int[mrj.getMapTasks()][mrj.getReduceTasks()];
		for(int i = 0; i < mrj.getMapTasks(); i++) {
			for(int j = 0; j < mrj.getReduceTasks(); j++) {
				ids[i][j] = DATA_ID++;
			}
		}
		return ids;
		
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
			// if task is finished
			if(task.finished()) {
				// for all outputs
				for(int output_id : task.getOutputDataIDs()) {
					// if we are not downloading it already,
					if(!this.downloads.containsKey(output_id)) {
						DataTransfer dt = 
								new DataTransfer(this.node_id, output_id, task.getOutputSize());
						this.downloads.put(output_id,  dt);
						// for every uploader of this output data,
						for(Node uploader : this.tracker.getUploaders(output_id)) {
							uploader.requestDataTransfer(this, dt);
						}
			
					}
				}
			}
			
		}
	}
	
	/**
	 * See base doc.
	 */
	@Override
	public void update() {
		super.update();
		this.updateReduceTasks();
		// if all reduce tasks are finished
		if(this.finishedTasks(this.reduce_tasks) && 
		   this.finishedDataTransfers(this.downloads)) {
			throw new DoneException();			
		}
		// TODO - check if there is the new to replicate more tasks.
	}
	
	/**
	 * Creates an association between a node and a data id. This simply means 
	 * that the node 'node' contains the data identified by 'data_id'. 
	 */
	public void registerUploader(Node node, int data_id) {
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
	 * Searches for a task with unsent work units.
	 * Null returned if all work units have already been delivered.
	 * @param map
	 * @param tasks
	 * @return
	 */
	private Task searchDeliveringTask(HashMap<Integer, Task> tasks, Node node) {
		Iterator<Entry<Integer, Task>> it = tasks.entrySet().iterator();
		while(it.hasNext()) {
			Task task = it.next().getValue();
			if(task.delivering() && !task.hasWorker(node)) { return task; }
		}		
		return null;
	}
	
	/**
	 * Method that goes through a set of tasks to verify if all of them are 
	 * finished.
	 * @param tasks
	 * @return true if all tasks are finished.
	 */
	protected boolean finishedTasks(HashMap<Integer, Task> tasks) {
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
	public WorkUnit requestWork(Node node) {
		Task task = null;
		Task map_task = this.searchDeliveringTask(this.map_tasks, node);
		Task red_task = this.searchDeliveringTask(this.reduce_tasks, node);
		
		// if there is a map work to deliver,
		if(map_task != null) { task = map_task; task.newWorker(node); }
		
		// if we are still waiting for map results,
		else if(!this.finishedTasks(this.map_tasks)) { return null;	}
		
		// if we are in the reduce phase,
		else if(red_task != null )	{ task = red_task; task.newWorker(node); }
		
		// if no more reduce tasks to deliver,
		else { return null;	}
		
		Main.log("[Node 0] - node " + node.getId() + " requested work. Got task " + task.getTaskID());
		return new WorkUnit(
				task.getTaskID(),
				task.getInputDataIDs(),
				task.getOutputDataIDs(),
				task.getInputSize(), 
				task.getOutputSize(), 
				this);
	}
	
	/**
	 * Register a new finished task.
	 * @param node
	 * @param data_id
	 */
	public void workFinished(Node node, int task_id) {
		Task task = null;
		
		// check if it is a map task,
		if(this.map_tasks.containsKey(task_id)) { 
			task = this.map_tasks.get(task_id); 
		}
		// otherwise, it is a reduce task,
		else { 
			task = this.reduce_tasks.get(task_id); 
		}
		
		task.newResult(node);
		// for all outputs, register node as uploader.
		for(int output_id : task.getOutputDataIDs()) {
			this.registerUploader(node, output_id);	
		}
		
		Main.log("[Node 0] - computation " + task_id + " finished by node "+ node.getId());
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
