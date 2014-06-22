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
	 * @param node_id
	 * @param total_mbs
	 * @param left_mbs
	 * @param stakeholder
	 */
	public DataProcessing(
			int node_id, int data_id, int total_mbs, Server stakeholder) {
		super(node_id, data_id, total_mbs);
		this.stakeholder = stakeholder;
	}
	
	public Server getStakeholder() { return this.stakeholder; }
}
