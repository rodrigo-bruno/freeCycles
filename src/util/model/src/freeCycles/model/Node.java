package freeCycles.model;

import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Map.Entry;

/**
 * TODO - comment on this.
 * @author underscore
 *
 */
public class Node {
	
	/**
	 * Node uid.
	 */
	protected int node_id;
		
	/**
	 * Node upload rate (MB/s).
	 */
	protected float upload_rate;
	
	/**
	 * Tracker node, used to update uploaders list (for downloads).
	 */
	protected Server tracker;
	
	/**
	 * List of ongoing uploads.
	 */
	protected LinkedList<DataTransfer> active_uploads;
	
	/**
	 * Map of downloads (some might be ongoing, others might be done).
	 * Map<data id, data transfer>
	 */
	protected HashMap<Integer, DataTransfer> downloads;
	
	/**
	 * Map of buffered uploads. This is used to commit/flush uploads when 
	 * more appropriate.
	 * Map<data transfer, mbs to upload>  
	 */
	protected HashMap<DataTransfer, Float> buffered_uploads;
	
	/**
	 * Failed?
	 */
	protected boolean failed;
	
	/**
	 * Constructor.
	 * @param upload_rate
	 * @param tracker
	 */
	public Node(int node_id, float upload_rate, Server tracker) {
		this(node_id, upload_rate);
		this.tracker = tracker;
	}
	
	/**
	 * Constructor.
	 * @param upload_rate
	 */
	public Node(int node_id, float upload_rate) {
		this.node_id = node_id;
		this.upload_rate = upload_rate;
		this.tracker = null;
		this.active_uploads = new LinkedList<DataTransfer>();
		this.downloads = new HashMap<Integer, DataTransfer>();
		this.buffered_uploads = new HashMap<DataTransfer, Float>();
		this.failed = false;
		Main.log("[Node " + node_id + "] - ready to serve." );
	}
	
	/**
	 * Request data transfer. Nodes wanting some data will call this method on
	 * nodes they think that they have it.
	 * @param node
	 * @param data_transfer
	 */
	public void requestDataTransfer(Node node, DataTransfer data_transfer) {
		if(!this.failed && !this.active_uploads.contains(data_transfer)) {
			Main.log(	"[Node " + this.getId() + "] - node " + node.getId() + 
						" requested data " + data_transfer.getId());
			this.active_uploads.add(data_transfer);
		}
	}
	
	/**
	 * Deactivate node.
	 */
	public void deactivate(){ 
		this.failed = true;
		this.active_uploads.clear();
		// for all downloads, abort
		this.downloads.clear();
		Main.log("[Node " + this.node_id + "] - deactivated.");
	}
	
	/**
	 * Activate node.
	 */
	public void activate() {
		this.failed = false;
		Main.log("[Node " + this.node_id + "] - activated.");
	}
	
	/**
	 * Method that goes through a set of data transfers to verify if all of 
	 * them are finished.
	 * @param data_transfer
	 * @return
	 */
	protected boolean finishedDataTransfers(
			HashMap<Integer, DataTransfer> data_transfer) {
		Iterator<Entry<Integer, DataTransfer>> it = 
				data_transfer.entrySet().iterator();
		while(it.hasNext()) { 
			if(!it.next().getValue().done()) { return false; } 
		}
		return true;				
	}
	
	/**
	 * Update upload transfers.
	 */
	public void updateUploads() {
		// if no uploads, ignore,
		if(this.active_uploads.size() == 0) { return; }
		
		float share = this.upload_rate / this.active_uploads.size();
		
		Iterator<DataTransfer> it = this.active_uploads.listIterator();
		while(it.hasNext()) {
			DataTransfer dt = it.next();
			int destination_id = dt.getNodeID();
			float total_uploaded = dt.getConstribution(this.node_id);
			DataTransfer local_data = this.downloads.get(dt.getId());	
			
			// if transfer is done or aborted
			if(dt.done() || dt.aborted()) {
				Main.log(	"[Node "+ this.getId() + "] - finished uploading data " 
							+ dt.getId());
				it.remove();
				continue;
			}
			
			// if local data is inexistent,
			if(local_data == null) { continue; }
			// unsent_mbs = every thing that I have less: 1) what you gave me, 2) what I gave you 
			//float unsent_mbs = 
			//		local_data.getFinishedMBs() -  
			//		local_data.getConstribution(destination_id) -
			//		total_uploaded;
			float unsent_mbs = local_data.getFinishedMBs() - dt.getFinishedMBs();
			// if there is still data to transfer,
			if(unsent_mbs > 0) {
				this.bufferUploads(dt, unsent_mbs < share ? unsent_mbs : share);
			}
		} 
	}
	
	/**
	 * Update download transfers.
	 */
	public void updateDownloads() {
		Iterator<Entry<Integer, DataTransfer>> it =
				this.downloads.entrySet().iterator();
		while(it.hasNext()) {
			Entry<Integer, DataTransfer> entry = it.next();
			DataTransfer dt = entry.getValue();
			// register node as uploader,
			this.tracker.registerUploader(this, dt.getId());
			// if transfer is done or aborted
			if(dt.done()) {	continue; }
			// for all known uploader, request
			for(Node uploader : this.tracker.getUploaders(entry.getKey())) {
				// cheating =)
				if(uploader.getId() == this.getId()) { continue; }
				uploader.requestDataTransfer(this, dt);
			}
		}
	}
	
	/**
	 * Register data to upload.
	 * @param dt
	 * @param mbs
	 */
	private void bufferUploads(DataTransfer dt, float mbs) {
		Float current = 0f;
		if(this.buffered_uploads.containsKey(dt)) {
			current = this.buffered_uploads.get(dt);
		}
		this.buffered_uploads.put(dt, current + mbs);
	}
	
	/**
	 * Float buffered uploads.
	 */
	public void flushUploads() {
		Iterator<Entry<DataTransfer, Float>> it =
				this.buffered_uploads.entrySet().iterator();
		while(it.hasNext()) { 
			Entry<DataTransfer, Float> entry = it.next();
			entry.getKey().advance(this.node_id, entry.getValue());
			it.remove();
		}
	}
	
	/**
	 * Update node state.
	 */
	void update() { 
		// if failed, ignore
		if(this.failed) { return; }
		this.updateDownloads();
		this.updateUploads();
	}
	
	int getId() { return this.node_id; }
	
	DataTransfer getDownload(int data_id) { 
		return this.downloads.get(data_id); 
	}
	
	DataTransfer getUpload(int data_id) {
		for(DataTransfer dt : this.active_uploads) { 
			if(dt.getId() == data_id) { return dt; } 
		}
		return null;
	}
	
	DataTransfer getDataTransfer(int data_id) {
		DataTransfer dt = this.getDownload(data_id);
		return dt == null ? this.getUpload(data_id) : dt; 
	}

}
