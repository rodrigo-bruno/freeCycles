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
	 * Aborted?
	 */
	private boolean aborted;
	
	/**
	 * Constructor.
	 * @param total_mbs
	 * @param left_mbs
	 */
	public DataOperation(int id, int total_mbs, int left_mbs) {
		this.id = id;
		this.total_mbs = total_mbs;
		this.left_mbs = left_mbs;
		this.aborted = false;
	}

	/**
	 * @return the total_mbs
	 */
	public int getTotalMBs() {
		return total_mbs;
	}

	/**
	 * @return the left_mbs
	 */
	public int getLeftMBs() {
		return left_mbs;
	}
	
	/**
	 * @return true if data operation is done.
	 */
	public boolean done() {
		return this.left_mbs == 0;
	}
	
	/**
	 * @return true if operation was aborted.
	 */
	public boolean aborted() {
		return this.aborted;
	}
	
	/**
	 * @return number of processed/downloaded/whatever MBs.
	 */
	public int getFinishedMBs() {
		return this.total_mbs - this.left_mbs;
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
	
	/**
	 * Abort operation.
	 */
	public void abort() {
		this.aborted = true;
	}
}