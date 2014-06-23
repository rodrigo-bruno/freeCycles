package freeCycles.model;

import java.util.LinkedList;

public class Main {

	private static int TIME = 0;
	private static int NODE_ID = 0;
	
	public static int getTime() { return TIME; }
	public static void log(String msg) { System.out.println(msg); }
	public static void err(String msg) { System.err.println(msg); }
	
	public static String usage() {
		return "Usage: java freeCyclesModel\n"
				+ "\t\t<# volunteers> - number of available volunteers\n"
				+ "\t\t<upload rate> - upload rate for all nodes (MBs)\n"
				+ "\t\t<processing rate> - processing rate for volunteers (MBs)\n"
				+ "\t\t<# maps> - number of map tasks\n"
				+ "\t\t<map repl factor> - replication factor for map tasks\n"
				+ "\t\t<# reds> - number of reduce tasks\n"
				+ "\t\t<red repl factor> - replication factor for reduce tasks\n"
				+ "\t\t<input size> - input size (MBs)\n"
				+ "\t\t<interm size> - intermediate output size (MBs)\n"
				+ "\t\t<output size> - output size (MBs)\n"
				+ "Example: java freeCYclesModel 90 10 1000 16 3 4 3 1000 1000 1000";
	}
	
	public static void main(String[] args) throws InterruptedException {

		if(args.length != 10) { 
			err(usage()); 
			return; 
		}

		// Configuration
		int number_volunteers = Integer.parseInt(args[0]);
		float upload_rate =Float.parseFloat(args[1]);
		int processing_rate = Integer.parseInt(args[2]);
		int number_maps = Integer.parseInt(args[3]);
		int map_repl_factor = Integer.parseInt(args[4]);
		int number_reds = Integer.parseInt(args[5]);
		int red_repl_factor = Integer.parseInt(args[6]);
		int input_size = Integer.parseInt(args[7]);
		int interm_size = Integer.parseInt(args[8]);
		int output_size = Integer.parseInt(args[9]);
		
		
		// Create nodes
		Server server = new Server(NODE_ID++, upload_rate,	new MapReduceJob(
					number_maps, 
					number_reds, 
					input_size, 
					interm_size, 
					output_size,
					map_repl_factor, 
					red_repl_factor));
		LinkedList<Volunteer> volunteers = new LinkedList<Volunteer>();
		for(int i = 0; i < number_volunteers; i++) {
			volunteers.add(new Volunteer(
					NODE_ID++, upload_rate, processing_rate, server));
		}
		
		// Simulation
		try {
			while(true) {
				log("[Shit " + ++TIME + "] ##############################");
				server.update();
				for(Volunteer v : volunteers) { v.update(); }
				server.flushUploads();
				for(Volunteer v : volunteers) { v.flushUploads(); }
			}
		} catch(DoneException e) { err(new Integer(TIME).toString()); }
	}

}
