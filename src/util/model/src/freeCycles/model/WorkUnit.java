package freeCycles.model;

import java.util.LinkedList;

/**
 * Class representing a unit of work.
 */
public class WorkUnit {

	/**
	 * ID belonging to the task that delivered this work unit.
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
	 * Input size per id (total input size = idata_ids.size * input_size).
	 */
	private int input_size;
	
	/**
	 * Output size per id (total output size = odata_ids.size * output_size).
	 */
	private int output_size;
	
	/**
	 * Server responsible for the computation.
	 */
	private Server stakeholder;
	
	/**
	 * Constructor.
	 * @param task_id
	 * @param idata_ids
	 * @param odata_ids
	 * @param input_size
	 * @param output_size
	 * @param stakeholder
	 */
	public WorkUnit(  
			int task_id, 
			LinkedList<Integer> idata_ids,
			LinkedList<Integer> odata_ids,
			int input_size,
			int output_size, 
			Server stakeholder) {
		this.task_id = task_id;
		this.idata_ids = idata_ids;
		this.odata_ids = odata_ids;
		this.input_size = input_size;
		this.output_size = output_size;
		this.stakeholder = stakeholder;
	}
	
	public int getTaskID() { return this.task_id; }
	public LinkedList<Integer> getInputDataIDs() { return this.idata_ids; }
	public LinkedList<Integer> getOutputDataIDs() { return this.odata_ids; }
	public int getInputSize() { return this.input_size; }
	public int getOutputSize() { return this.output_size; }
	public Server getStakeholder() { return this.stakeholder; }

}
