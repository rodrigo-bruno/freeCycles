/**
 * 
 */
package freeCycles.model;

/**
 * @author underscore
 * TODO - doc.
 */
public class Server extends Node {

	/**
	 * Constructor.
	 * @param upload_rate
	 */
	public Server(int upload_rate) { super(upload_rate); }
	
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
	

}
