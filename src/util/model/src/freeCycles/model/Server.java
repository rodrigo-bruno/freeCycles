/**
 * 
 */
package freeCycles.model;

import java.util.HashMap;
import java.util.LinkedList;

/**
 * @author underscore
 * TODO - doc.
 */
public class Server extends Node {
	
	private HashMap<Integer, LinkedList<Node>> tracker;

	/**
	 * Constructor.
	 * @param upload_rate
	 */
	public Server(int upload_rate) { 
		super(upload_rate, null);
	}
	
	/**
	 * Submit a new MapReduce job.
	 * @param mrj
	 */
	public void submit(MapReduceJob mrj) {
		
	}
	
	/**
	 * Request for work.
	 * @return
	 */
	public WorkUnit requestTask() {
		return null;
		// receiver should create data transfer, and then create data processing.
	}
	
	public void taskFinished(int job_id, int task_id) {
		
	}
	
	/**
	 * Returns the list of uploaders for a particular data id.	
	 * @param data_id
	 * @return
	 */
	public LinkedList<Node> getUploaders(Integer data_id) {
		return this.tracker.containsKey(data_id) ? 
				this.tracker.get(data_id) :
				new LinkedList<Node>();
	}

}
