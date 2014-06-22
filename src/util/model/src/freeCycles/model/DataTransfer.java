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
	private HashMap<Integer, Float> uploaders_contribution;
	
	/**
	 * TODO - doc.
	 */
	private HashMap<Integer, Float> advance_holder;
		
	/**
	 * Constructor.
	 * @param data_id
	 * @param total_mbs
	 */
	public DataTransfer(int node_id, int data_id, float total_mbs) {
		super(node_id, data_id, total_mbs);
		this.uploaders_contribution = new HashMap<Integer, Float>();
		this.advance_holder = new HashMap<Integer, Float>();
		
	}
	
	/**
	 * Advance the data transfer. Register who is contributing.
	 * @param uploader
	 * @param mbs
	 */
	public void advance(int node_id, float mbs) {
		this.holdAdvance(Main.getTime(), mbs);
		if(!this.uploaders_contribution.containsKey(node_id)) {
			this.uploaders_contribution.put(node_id, 0f);
		}
		Float uploaded = this.uploaders_contribution.get(node_id);
		this.uploaders_contribution.put(node_id, uploaded.floatValue() + mbs);		
	}
	
	/**
	 * TODO - doc.
	 * @param current_time
	 * @param mbs
	 */
	private void holdAdvance(Integer current_time, Float mbs) {
		Float f = 0f;
		if(this.advance_holder.containsKey(current_time)) {
			f = this.advance_holder.get(current_time);
		}
		this.advance_holder.put(current_time, f + mbs);
	}
	
	/**
	 * TODO - doc.
	 * @param time
	 */
	public void commitAdvances(Integer time) {
		Float f = 0f;
		if(this.advance_holder.containsKey(time)) { 
			f = this.advance_holder.get(time); 
		}
		super.advance(f);
		this.advance_holder.remove(time);
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