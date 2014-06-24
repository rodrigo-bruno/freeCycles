package freeCycles.model;

import java.util.HashMap;

/**
 * Class representing a data transfer.
 */
class DataTransfer extends DataOperation {
	
	/**
	 * Uploaders' constribution, i.e. how much every uploader uploaded for this
	 * particular data transfer.
	 */
	private HashMap<Integer, Float> uploaders_contribution;
		
	/**
	 * Constructor.
	 * @param data_id
	 * @param total_mbs
	 */
	public DataTransfer(Node node, int data_id, float total_mbs) {
		super(node, data_id, total_mbs);
		this.uploaders_contribution = new HashMap<Integer, Float>();		
	}
	
	/**
	 * Advance the data transfer. Register who is contributing.
	 * @param uploader
	 * @param mbs
	 */
	public void advance(int node_id, float mbs) {
		super.advance(mbs);
		// register uploader constribution
		if(!this.uploaders_contribution.containsKey(node_id)) {
			this.uploaders_contribution.put(node_id, 0f);
		}
		Float uploaded = this.uploaders_contribution.get(node_id);
		this.uploaders_contribution.put(node_id, uploaded.floatValue() + mbs);	
		
		Main.log(	"[Node " + this.node.getId() + " ( " + this + 
					")] - received " + mbs + " (left=" + this.left_mbs + 
					") from node " + node_id);
	}
	
	/**
	 * Return the number of MBs uploaded by a particular node.
	 * @param node
	 * @return
	 */
	public float getConstribution(Integer node_id) {
		return this.uploaders_contribution.containsKey(node_id) ? 
				this.uploaders_contribution.get(node_id).floatValue() : 0;
	}
}