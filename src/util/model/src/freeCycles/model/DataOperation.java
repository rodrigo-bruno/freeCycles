package freeCycles.model;

/**
 * Class representing a generic operation over data.
 */
abstract class DataOperation {
	
	/**
	 * Responsible node for this operation.
	 */
	protected Node node;
	
	/**
	 * Operation id.
	 */
	protected int id;
	
	/**
	 * Size of the data to transfer.
	 */
	protected float total_mbs;
	
	/**
	 * Number of MBs left to transfer;
	 */
	protected float left_mbs;
	
	/**
	 * Aborted?
	 */
	protected boolean aborted;
	
	/**
	 * Constructor.
	 * @param node
	 * @param total_mbs
	 * @param left_mbs
	 */
	public DataOperation(Node node, int id, float total_mbs) {
		this.node = node;
		this.id = id;
		this.total_mbs = total_mbs;
		this.left_mbs = total_mbs;
		this.aborted = false;
	}

	/**
	 * @return the total_mbs
	 */
	public float getTotalMBs() {
		return total_mbs;
	}

	/**
	 * @return the left_mbs
	 */
	public float getLeftMBs() {
		return left_mbs;
	}
	
	/**
	 * @return number of processed/downloaded/whatever MBs.
	 */
	public float getFinishedMBs() {
		return this.total_mbs - this.left_mbs;
	}
	
	/**
	 * @return true if data operation is done.
	 */
	public boolean done() {
		return this.left_mbs <= 0;
	}
	
	/**
	 * @return true if operation was aborted.
	 */
	public boolean aborted() {
		return this.aborted;
	}
		
	/**
	 * 
	 * @return the id.
	 */
	public int getId() {
		return this.id;
	}
	
	/**
	 * 
	 * @return responsible node.
	 */
	public Node getNode() {
		return this.node;
	}
	
	/**
	 * Advance this operation by mbs MBs.
	 * @param mbs - number of MBs to advance.
	 */
	public void advance(float mbs) {
		this.left_mbs = (this.left_mbs <= mbs) ? 0 : this.left_mbs - mbs;
	}
	
	/**
	 * Abort operation.
	 */
	public void abort() {
		this.aborted = true;
	}
}