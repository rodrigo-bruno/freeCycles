package freeCycles.model;

import java.util.HashMap;

/**
 * 
 * @author underscore
 *
 */
class DataTransfer extends DataOperation {
	
	/**
	 * Uploaders' constribution, i.e. how much every uploader uploaded for this
	 * particular data transfer.
	 */
	private HashMap<Node, Integer> uploaders_contribution;
	
	/**
	 * Constructor.
	 * @param data_id
	 * @param total_mbs
	 * @param left_mbs
	 */
	public DataTransfer(int data_id, int total_mbs, int left_mbs) {
		super(data_id, total_mbs, left_mbs);
		this.uploaders_contribution = new HashMap<Node, Integer>();
	}
	
	/**
	 * Advance the data transfer. Register who is contributing.
	 * @param uploader
	 * @param mbs
	 */
	public void advance(Node uploader, int mbs) {
		super.advance(mbs);
		if(!this.uploaders_contribution.containsKey(uploader)) {
			this.uploaders_contribution.put(uploader, 0);
		}
		Integer uploaded = this.uploaders_contribution.get(uploader);
		uploaded = uploaded.intValue() + mbs;		
	}
	
	/**
	 * Return the number of MBs uploaded by a particular node.
	 * @param node
	 * @return
	 */
	public int getConstribution(Node node) {
		return this.uploaders_contribution.containsKey(node) ? 
				this.uploaders_contribution.get(node).intValue() : 0;
	}
}