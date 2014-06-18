package freeCycles.model;

/**
 * Class representing a generic operation over data.
 */
abstract class DataOperation {
	
	/**
	 * Operation id.
	 */
	private int id;
	
	/**
	 * Size of the data to transfer.
	 */
	private int total_mbs;
	
	/**
	 * Number of MBs left to transfer;
	 */
	private int left_mbs;
	
	/**
	 * Constructor.
	 * @param total_mbs
	 * @param left_mbs
	 */
	public DataOperation(int id, int total_mbs, int left_mbs) {
		this.id = id;
		this.total_mbs = total_mbs;
		this.left_mbs = left_mbs;
	}

	/**
	 * @return the total_mbs
	 */
	public int getTotalMbs() {
		return total_mbs;
	}

	/**
	 * @return the left_mbs
	 */
	public int getLeftMbs() {
		return left_mbs;
	}
	
	/**
	 * 
	 * @return the id.
	 */
	public int getId() {
		return this.id;
	}
	
	/**
	 * Advance this operation by mbs MBs.
	 * @param mbs - number of MBs to advance.
	 */
	public void advance(int mbs) {
		this.left_mbs = (this.left_mbs <= mbs) ? 0 : this.left_mbs - mbs;
	}
}