package freeCycles.model;

/**
 * 
 * @author underscore
 *
 */
class DataProcessing extends DataOperation {

	/**
	 * The stakeholder for this computation.
	 */
	private Server stakeholder;
	
	/**
	 * Constructor.
	 * @param total_mbs
	 * @param left_mbs
	 * @param stakeholder
	 */
	public DataProcessing(
			int data_id, int total_mbs, Server stakeholder) {
		super(data_id, total_mbs);
		this.stakeholder = stakeholder;
	}
	
	public Server getStakeholder() { return this.stakeholder; }
}
