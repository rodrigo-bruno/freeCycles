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
	public Volunteer(
			int node_id, float upload_rate, int processing_rate, Server server) {
		super(node_id, upload_rate, server);
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
	private void updateTasks() {
		// if no task,
		if(this.active_tasks.size() == 0) { 
			WorkUnit wu = this.server.requestWork(this);
			
			// if no task was received,
			if (wu == null) { return; }
			
			this.active_tasks.add(wu);
			
			int input_size = wu.getInputSize();
			// for all input ids,
			for(int input_id : wu.getInputDataIDs()) {
				if(!this.downloads.containsKey(input_id)) {
					DataTransfer dt = new DataTransfer(this.node_id, input_id, input_size);
					this.downloads.put(input_id,  dt);	
				}
				// for all uploaders of this input id,
				for(Node node : this.tracker.getUploaders(input_id))	{ 
					node.requestDataTransfer(this, this.downloads.get(input_id)); 
				}
			}
			return;
		}
		// if there is at least one ongoing task,
		Iterator<WorkUnit> it = this.active_tasks.listIterator();
		while(it.hasNext()) {
			WorkUnit wu = it.next();
			int task_id = wu.getTaskID();
			DataProcessing dp = this.active_processing.get(task_id);
			boolean downloads_finished = true;

			// for every input id,
			for(int input_id : wu.getInputDataIDs()) {
				// if download is still going,
				if(!this.downloads.get(input_id).done()) {
					downloads_finished = false;
					break;
				}
			}
			
			// if we are still missing some data,
			if(!downloads_finished) { continue; }
			
			// if processing is not started yet, 
			if(dp == null) {
				this.active_processing.put(task_id,	new DataProcessing(
						this.node_id,
						task_id,
						wu.getInputSize() * wu.getInputDataIDs().size(),  
						wu.getStakeholder()));
				Main.log("[Node " + this.getId() + "] - starting computation " + task_id);
				continue;
			}
			
			// if processing is done, 
			if(dp.done()) {
				// for all partial outputs
				for(int output_id : wu.getOutputDataIDs()) {
					// share create output data of the computation.
					this.downloads.put(
							output_id, 
							new DataTransfer(this.node_id, output_id, wu.getOutputSize()));
					// advance output to completion. 
					this.downloads.get(
							output_id).advance(this.node_id, wu.getOutputSize());
				}
				dp.getStakeholder().workFinished(this , wu.getTaskID());
				it.remove();
				this.active_processing.remove(task_id);
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
		super.update();
		// if failed, ignore
		if(this.failed) { return; }
		this.updateTasks();
	}

}
