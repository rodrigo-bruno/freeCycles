package freeCycles.model;

/**
 * Class representing a unit of work.
 */
public class WorkUnit {

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
	 * Server responsible for the computation.
	 */
	private Server stakeholder;
	
	/**
	 * Constructor.
	 * @param data_id
	 * @param uploaders
	 * @param stakeholder
	 */
	public WorkUnit(  
			int data_id, 
			int input_size,
			int output_size, 
			Server stakeholder) {
		this.data_id = data_id;
		this.input_size = input_size;
		this.output_size = output_size;
		this.stakeholder = stakeholder;
	}
	
	public int getDataId() { return this.data_id; }
	public int getInputSize() { return this.input_size; }
	public int getOutputSize() { return this.output_size; }
	public Server getStakeholder() { return this.stakeholder; }

}
