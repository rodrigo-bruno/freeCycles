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
	protected int upload_rate;
	
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
	 */
	protected HashMap<Integer, DataTransfer> downloads;
	
	/**
	 * Failed?
	 */
	boolean failed;
	
	/**
	 * Constructor.
	 * @param upload_rate
	 * @param tracker
	 */
	public Node(int node_id, int upload_rate, Server tracker) {
		this(node_id, upload_rate);
		this.tracker = tracker;
	}
	
	/**
	 * Constructor.
	 * @param upload_rate
	 */
	public Node(int node_id, int upload_rate) {
		this.node_id = node_id;
		this.upload_rate = upload_rate;
		this.tracker = null;
		this.active_uploads = new LinkedList<DataTransfer>();
		this.downloads = new HashMap<Integer, DataTransfer>();
		this.failed = false;
		Main.log("[Node " + node_id + "] - ready to serve." );
	}
	
	/**
	 * Request data transfer. Nodes wanting some data will call this method on
	 * nodes they think that they have it.
	 * @param data_transfer
	 */
	public void requestDataTransfer(DataTransfer data_transfer) {
		if(!this.failed && !this.active_uploads.contains(data_transfer)) {
			this.active_uploads.add(data_transfer);
		}
	}
	
	/**
	 * Deactivate node.
	 */
	void deactivate(){ 
		this.failed = true;
		this.active_uploads.clear();
		// for all downloads, abort
		this.downloads.clear();
	}
	
	/**
	 * Activate node.
	 */
	void activate() {
		this.failed = false;
	}

	
	/**
	 * Update upload transfers.
	 */
	void updateUploads() {
		// if no uploads, ignore,
		if(this.active_uploads.size() == 0) { return; }
		int share = this.upload_rate / this.active_uploads.size();
		Iterator<DataTransfer> it = this.active_uploads.listIterator();
		while(it.hasNext()) {
			DataTransfer dt = it.next();
			DataTransfer local_data = this.downloads.get(dt.getId());
			int total_uploaded = dt.getConstribution(this);	
			// if transfer is done or aborted
			if(dt.done() || dt.aborted()) {
				it.remove();
				continue;
			}
			// if local data is inexistent,
			if(local_data == null) { continue; }
			int unsent_mbs = local_data.getFinishedMBs() - total_uploaded;
			// if there is still data to transfer,
			if(unsent_mbs > 0) {
				dt.advance(this, unsent_mbs < share ? unsent_mbs : share);
			}
		} 
	}
	
	/**
	 * Update download transfers.
	 */
	void updateDownloads() {
		Iterator<Entry<Integer, DataTransfer>> it =
				this.downloads.entrySet().iterator();
		while(it.hasNext()) {
			Entry<Integer, DataTransfer> entry = it.next();
			DataTransfer dt = entry.getValue();
			// if transfer is done or aborted
			if(dt.done() || dt.aborted()) {	continue; }
			// for all known uploader, request
			for(Node uploader : this.tracker.getUploaders(entry.getKey())) {
				uploader.requestDataTransfer(dt);
			}
		}
	}
	
	/**
	 * Update node state.
	 */
	void update() { 
		// if failed, ignore
		if(this.failed) { return; }
		this.updateUploads();
		this.updateDownloads();
	}

}
