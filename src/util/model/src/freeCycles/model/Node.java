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
	 * List of ongoing uploads (data-id -> [ dt from node1, dt from nodeN]).
	 */
	protected HashMap<Integer, LinkedList<DataTransfer>> upload_requests;
	
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
		this.upload_requests = new HashMap<Integer, LinkedList<DataTransfer>>();
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
		int data_id = data_transfer.getId();
		if(this.failed) { return; }
		
		if(!this.upload_requests.containsKey(data_id)) {
			LinkedList<DataTransfer> list = new LinkedList<DataTransfer>();
			list.add(data_transfer);
			this.upload_requests.put(data_transfer.getId(), list);
		} 
		
		else if (this.upload_requests.get(data_id).contains(data_transfer)) {
			return;
		} 
		
		else { this.upload_requests.get(data_id).add(data_transfer); }
		
		Main.log(	"[Node " + this.getId() + "] - node " + node.getId() + 
				" requested data " + data_transfer.getId());
	}
	
	/**
	 * Deactivate node.
	 */
	public void deactivate(){ 
		this.failed = true;
		this.upload_requests.clear();
		
		// for all downloads, abort
		Iterator<Entry<Integer, DataTransfer>> it =	
				this.downloads.entrySet().iterator();
		while(it.hasNext()) { it.next().getValue().aborted(); }
		
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
		if(this.upload_requests.size() == 0) { return; }
		
		float dshare = this.getUnusedUpBandwidth() / this.upload_requests.size();
		
		Iterator<Entry<Integer, LinkedList<DataTransfer>>> mit = 
				this.upload_requests.entrySet().iterator();
		// for all data ids,
		while(mit.hasNext()) {
			Entry<Integer, LinkedList<DataTransfer>> entry = mit.next();
			int data_id = entry.getKey();
			LinkedList<DataTransfer> list = entry.getValue();
			
			DataTransfer local_data = this.downloads.get(data_id);
			// if local data is inexistent,
			if(local_data == null) { continue; }
			
			LinkedList<Node> swarm = new LinkedList<Node>();
			
			// for all data transfers of this data id,
			Iterator<DataTransfer> lit = list.iterator();
			while(lit.hasNext()) {
				DataTransfer dt = lit.next();
				// if transfer is done or aborted
				if(dt.done() || dt.aborted()) {
					Main.log(	"[Node "+ this.getId() + 
								"] - finished uploading data " + dt.getId() + 
								" to " + dt.getNode().getId());
					lit.remove();
					continue;
				}
				
				// if remote node has more data than me,
				if(dt.getFinishedMBs() >= local_data.getFinishedMBs()) {
					continue;
				}
				
				// TODO - comment.
				float nshare = 
						Math.min(	Math.abs(
										dt.getFinishedMBs() - 
										local_data.getFinishedMBs()), 
									dshare);
				this.bufferUploads(dt, nshare / list.size());
				swarm.add(dt.getNode());
			}
			
			// for each node that received some data,
			for(Node n1 : swarm) {
				// min(upload restante / size lis, share/size list)
				float nshare = Math.min(
							n1.getUnusedUpBandwidth() / list.size(), 
							this.getBufferedUpload(n1.getId(), data_id));
				// for every node that 
				for(Node n2 : swarm) {
					if(n1 != n2) { 
						n1.bufferUploads(n2.getId(), data_id, nshare); 
					}
				}
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
	public void bufferUploads(DataTransfer dt, float mbs) {
		Float current = 0f;
		if(this.upload_requests.containsKey(dt)) {
			current = this.buffered_uploads.get(dt);
		}
		this.buffered_uploads.put(dt, current + mbs);
	}
	
	/**
	 * TODO - doc!
	 * @param data_id
	 * @param mbs
	 */
	public void bufferUploads(Integer node_id, Integer data_id, float mbs) {
		// assert(this.upload_requests.get(data_id) != null)
		if(!this.upload_requests.containsKey(data_id)) { return; }
		for(DataTransfer dt : this.upload_requests.get(data_id)) {
			if(dt.getNode().getId() == node_id) { this.bufferUploads(dt, mbs); }
		}		
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
	
	/**
	 * 
	 * @return the current unused upload bandwidth.
	 */
	float getUnusedUpBandwidth() {
		Iterator<Entry<DataTransfer, Float>> it =
				this.buffered_uploads.entrySet().iterator();
		float sum = 0;
		for(; it.hasNext(); sum += it.next().getValue());
		return this.upload_rate - sum;
	}
	
	/**
	 * TODO - Comment!
	 * @param node_id
	 * @param data_id
	 * @return
	 */
	private Float getBufferedUpload(Integer node_id, Integer data_id) {
		Iterator<Entry<DataTransfer, Float>> it = 
				this.buffered_uploads.entrySet().iterator();
		while(it.hasNext()) {
			Entry<DataTransfer, Float> entry = it.next();
			DataTransfer dt = entry.getKey();
			Float mbs = entry.getValue();
			if(dt.getId() == data_id && dt.getNode().getId() == node_id) {
				return mbs;
			}
		}
		return null;
	}
}
