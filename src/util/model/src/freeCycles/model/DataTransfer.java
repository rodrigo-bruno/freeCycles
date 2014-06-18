package freeCycles.model;

/**
 * 
 * @author underscore
 *
 */
class DataTransfer extends DataOperation {
	
	/**
	 * Constructor.
	 * @param data_id
	 * @param total_mbs
	 * @param left_mbs
	 */
	public DataTransfer(int data_id, int total_mbs, int left_mbs) {
		super(data_id, total_mbs, left_mbs);
	}
}