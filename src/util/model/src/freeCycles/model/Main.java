package freeCycles.model;

import java.util.LinkedList;

public class Main {

	private static int TIME = 0;
	private static int NODE_ID = 0;

	// Configuration parameters
	private static int number_volunteers 	= 0;
	private static int session_time 		= 0;
	private static int volunteer_rate 		= 0;
	private static float upload_rate 		= 0f;
	private static int processing_rate 		= 0;
	private static int number_maps 			= 0;
	private static int map_repl_factor 		= 0;
	private static int number_reds 			= 0;
	private static int red_repl_factor 		= 0;
	private static int input_size 			= 0;
	private static int interm_size 			= 0;
	private static int output_size 			= 0;
	
	public static int getTime() { return TIME; }
	public static void log(String msg) { System.out.println(msg); }
	public static void err(String msg) { System.err.println(msg); }
	
	public static String usage() {
		return "Usage: java freeCyclesModel\n"
				+ "\t\t<# volunteers> - number initial volunteers\n"
				+ "\t\t<avg session time> - minutes until failure (inf = 0)\n"
				+ "\t\t<new volunteer rate> - minutes until a new volunteer comes (inf = 0)\n"
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
	
	private static Volunteer newVolunteer(Server server) {
		return new Volunteer(
				NODE_ID++,
				0 + (int)(Math.random() * ((session_time - 0) + 1)),
				upload_rate, 
				processing_rate, 
				server);
	}
	
	public static void main(String[] args) throws InterruptedException {

		if(args.length != 12) { 
			err(usage()); 
			return; 
		}

		// read configuration
		number_volunteers = Integer.parseInt(args[0]);
		session_time = Integer.parseInt(args[1])*60;
		volunteer_rate = Integer.parseInt(args[2])*60;
		upload_rate =Float.parseFloat(args[3]);
		processing_rate = Integer.parseInt(args[4]);
		number_maps = Integer.parseInt(args[5]);
		map_repl_factor = Integer.parseInt(args[6]);
		number_reds = Integer.parseInt(args[7]);
		red_repl_factor = Integer.parseInt(args[8]);
		input_size = Integer.parseInt(args[9]);
		interm_size = Integer.parseInt(args[10]);
		output_size = Integer.parseInt(args[11]);
		
		
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
			volunteers.add(newVolunteer(server));
		}
		
		// Simulation
		try {
			while(true) {
				log("[Shit " + ++TIME + "] ##############################");
				
				// if session time != infinite
				if(session_time != 0) {
					for(Volunteer v : volunteers) { 
						// TODO - check deactivate
						if(v.getTimeToLeave() <= 0) { v.deactivate(); }
					}
				}
				
				if(volunteer_rate > 0 && TIME % volunteer_rate == 0) {
					volunteers.add(newVolunteer(server));
				}
				
				server.update();
				for(Volunteer v : volunteers) { v.update(); }
				server.flushUploads();
				for(Volunteer v : volunteers) { v.flushUploads(); }
			}
		} catch(DoneException e) { 
			err("Finish time - " + new Integer(TIME).toString()); 
		}
	}

}
