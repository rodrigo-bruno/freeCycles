package freeCycles.model;

import java.util.ArrayList;

/**
 * TODO - doc.
 * @author underscore
 *
 */
public class WorkUnit {
	
	/**
	 * Job id.
	 */
	private int job_id;
	
	/**
	 * Task id.
	 */
	private int task_id;
	
	/**
	 * Data id.
	 */
	private int data_id;
	
	/**
	 * Input size;
	 */
	private int input_size;
	
	/**
	 * Output size;
	 */
	private int output_size;
	
	/**
	 * List of possible data uploaders.
	 */
	private ArrayList<Node> uploaders;
	
	/**
	 * Server responsible for the computation.
	 */
	private Server stakeholder;
	
	/**
	 * Constructor.
	 * @param job_id
	 * @param task_id
	 * @param data_id
	 * @param uploaders
	 * @param stakeholder
	 */
	public WorkUnit(
			int job_id, 
			int task_id, 
			int data_id, 
			int input_size,
			int output_size,
			ArrayList<Node> uploaders, 
			Server stakeholder) {
		this.job_id = job_id;
		this.task_id = task_id;
		this.data_id = data_id;
		this.input_size = input_size;
		this.output_size = output_size;
		this.uploaders = uploaders;
		this.stakeholder = stakeholder;
	}
	
	public int getDataId() { return this.data_id; }
	public ArrayList<Node> getUploaders() { return this.uploaders; }
	public int getInputSize() { return this.input_size; }
	public int getOutputSize() { return this.output_size; }
	public Server getStakeholder() { return this.stakeholder; }
	public int getTaskId() { return this.task_id; }
	public int getJobId() { return this.job_id; }

}
