/**
 * 
 */
package freeCycles.model;

import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;

/**
 * @author underscore
 *
 */
public class Volunteer extends Node {
	
	/**
	 * Node processing power (MB/s). This represents  how many MBs can this
	 * node process per second.
	 */
	private int prossessing_rate;
	
	/**
	 * Map of on going computations (data processings).
	 */
	private HashMap<Integer, DataProcessing> active_processing;
	
	/**
	 * List of on going tasks.
	 */
	private LinkedList<WorkUnit> active_tasks;
	
	/**
	 * The almighty server. 
	 */
	private Server server;

	/**
	 * Constructor.
	 * @param upload_rate
	 * @param processing_rate
	 * @param server
	 */
	public Volunteer(int upload_rate, int processing_rate, Server server) {
		super(upload_rate, server);
		this.prossessing_rate = processing_rate;
		this.active_processing = new HashMap<Integer, DataProcessing>();
		this.active_tasks = new LinkedList<WorkUnit>();
		this.server = server;
	}
	
	/**
	 * Update current tasks' state. 
	 * Each task has one piece of data to process. The task id is the same as
	 * its piece of data.
	 */
	void updateTasks() {
		// if no task,
		if(this.active_tasks.size() == 0) { 
			WorkUnit wu = this.server.requestWork();
			
			if (wu == null) { return; }
			
			int data_id = wu.getDataId();
			int input_size = wu.getInputSize();
			DataTransfer dt = new DataTransfer(data_id, input_size, input_size);
			this.active_tasks.add(wu);
			this.downloads.put(data_id,  dt);
			for(Node node : this.tracker.getUploaders(data_id))	{ 
				node.requestDataTransfer(dt); 
			}
			return;
		}
		// if there is at least one ongoing task,
		Iterator<WorkUnit> it = this.active_tasks.listIterator();
		while(it.hasNext()) {
			WorkUnit wu = it.next();
			int data_id = wu.getDataId();
			DataTransfer dt = this.downloads.get(data_id);
			DataProcessing dp = this.active_processing.get(data_id);
			// if data transfer is done and no active processing, 
			if(dt.done() && dp == null) {
				this.active_processing.put(
						data_id, 
						new DataProcessing(
								data_id,
								wu.getInputSize(), 
								wu.getInputSize(), 
								wu.getStakeholder()));
				return;
			}
			// if processing is done, 
			if(dp.done()) {
				dp.getStakeholder().workFinished(this ,wu.getDataId());
				it.remove();
				this.active_processing.remove(data_id);
			}
			// otherwise (if there is still more work to do),
			else {
				dp.advance(prossessing_rate / this.active_processing.size());
			}
		}
	}
	
	/**
	 * See base doc.
	 */
	@Override
	public void deactivate() {
		this.active_tasks.clear();
		this.active_processing.clear();
		super.deactivate();
	}
	
	/**
	 * See base doc.
	 */
	@Override
	void update() {
		// if failed, ignore
		if(this.failed) { return; }
		this.updateTasks();
		super.update();
	}

}
