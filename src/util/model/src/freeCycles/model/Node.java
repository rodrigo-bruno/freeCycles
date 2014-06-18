package freeCycles.model;

import java.util.HashMap;
import java.util.LinkedList;

/**
 * TODO - comment on this. Data Transfers only
 * @author underscore
 *
 */
public class Node {
		
	/**
	 * Node upload rate (MB/s).
	 */
	private int upload_rate;
	
	/**
	 * List of ongoing uploads.
	 */
	protected LinkedList<DataTransfer> active_uploads;
	
	/**
	 * Map of downloads (some might be ongoing, others might be done).
	 */
	protected HashMap<Integer, DataTransfer> downloads;
	
	/**
	 * Constructor.
	 * @param upload_rate
	 */
	public Node(int upload_rate) {
		this.upload_rate = upload_rate;
		this.active_uploads = new LinkedList<DataTransfer>();
		this.downloads = new HashMap<Integer, DataTransfer>();
	}
	
	/**
	 * TODO - put some doc.
	 * @param data_transfer
	 */
	public void requestDataTransfer(DataTransfer data_transfer) {
		this.active_uploads.add(data_transfer);
	}
	
	/**
	 * TODO - complete implementation.
	 */
	void fail(){ this.active_uploads.clear(); }

	
	/**
	 * TODO - doc.
	 * FIXME - one can only send what she/he has...
	 */
	void updateDataTransfers() {
		for(DataTransfer dt : this.active_uploads) {
			DataTransfer local_data = this.downloads.get(dt.getId()); 
			// if local data is inexistent,
			if(local_data == null) { continue; }
			// server has to sent all 1/3 in each upload.
			// volunteers have to sent 1/3 in each upload.
			// CORRECTION - all guys should upload as much as they can.
			// There must be a away to refresh the list of uploaders.
			// Each guy cannot send more than he has.
		} 
	}
	
	/**
	 * TODO - doc.
	 */
	void update() { this.updateDataTransfers(); }

}
