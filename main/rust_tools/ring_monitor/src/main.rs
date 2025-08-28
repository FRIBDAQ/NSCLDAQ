
use std::{thread, time, process, env};
use std::io::{self, Write};
use std::sync::mpsc;
use portman_client;
use ringmaster_client;
use rust_ringitem_format;
use std::collections::HashMap;



const SERVICE_NAME : &str = "RING_MONITOR";
const BUFFER_SIZE : usize = 1024;
const UPDATE_TIME : u64  = 5;       // How often to update statistics -> main thread.

///
/// The information we want to maintain for each ringbuffer is:
/// 
#[derive(Clone, Debug)]
struct RingBufferStatistics {
    name:  String,         // name of the ringbuffer.
    bytes: usize,          // Total bytes observed through the ring.
    events: usize,         // Total PHYSICS_EVENT items observed through the ring.
    bytes_this_run: usize, // Total bytes through the ring since last BEGIN_RUN.
    events_this_run: usize, // Total PHYSICS_EVENT items since last BEGIN_RUN.
}
impl RingBufferStatistics {
    /// Create a ring buffer statistics item
    /// that has all counters zeroed:
    fn new(name :&str) -> RingBufferStatistics {
        RingBufferStatistics {
            name: String::from(name),
            bytes: 0, events: 0,
            bytes_this_run: 0, events_this_run: 0
        }
    }
    /// Zero the per run statistics:
    /// 
    fn new_run(&mut self) -> &mut RingBufferStatistics {
        self.bytes_this_run = 0;
        self.events_this_run = 0;
        self              // So chaining can be done.
    }
    /// Count some bytes:
    /// 
    fn count_bytes(&mut self, bytes : usize) -> &mut RingBufferStatistics {
        self.bytes += bytes;
        self.bytes_this_run += bytes;
        self
    }
    /// Count some events:
    ///
    fn count_events(&mut self, events: usize) -> &mut RingBufferStatistics {
        self.events += events;
        self.events_this_run += events;
        self
    }
}

///  This is the structure of statistics update messages sent
///  to the main thread by the monitor threads:
/// 
struct UpdateMessage {
    interval : time::Duration,
    statistics : RingBufferStatistics,
}

impl UpdateMessage {
    fn new(i : time::Duration, stats : &RingBufferStatistics) -> UpdateMessage {
        UpdateMessage { interval: i, 
            statistics : stats.clone()
        }
    }
}

/// The statistics the user is interested in also include rates:
/// 
struct StatisticsAndRates {
    last_statistics   : RingBufferStatistics, // Note this has the ring name.
    byte_rate         : f64,
    event_rate        : f64,
    byte_per_run_rate : f64,
    evts_per_run_rate : f64
}

impl StatisticsAndRates {
    // Create a newly initialized struc.
    // This requires a ring name.

    fn new(ring: &str) -> StatisticsAndRates {
        StatisticsAndRates { last_statistics: RingBufferStatistics::new(ring), 
             byte_rate: 0.0, event_rate: 0.0, byte_per_run_rate: 0.0, evts_per_run_rate: 0.0 }
    }
    fn update(&mut self, info : &UpdateMessage) -> &mut Self {
        // Let's be sure the message really was for us.
        // panic if not:

        if info.statistics.name != self.last_statistics.name {
            let msg = 
                format!("BUGCHECK - StatiticsAndRates::update called with mismatched rings was {} should be {}", 
                info.statistics.name, self.last_statistics.name
            );
            panic!("{}", msg);
        }

        // Need differences from last and now to get rates:

        let dt : f64 = info.interval.as_secs_f64();
        self.byte_rate = (info.statistics.bytes - self.last_statistics.bytes) as f64 / dt;
        self.event_rate = (info.statistics.events - self.last_statistics.events) as f64/dt;
        
        // If we started a new run and the events are < than last time, then we start from 0.

        if self.last_statistics.bytes_this_run > info.statistics.bytes_this_run {
            // New run so:

            self.last_statistics.bytes_this_run = 0;
            self.last_statistics.events_this_run = 0;
        }
        self.byte_per_run_rate = 
            (info.statistics.bytes_this_run - self.last_statistics.bytes_this_run) as f64 / dt;
        self.event_rate =
            (info.statistics.events_this_run - self.last_statistics.events_this_run) as f64 /dt;

        // Updtae the last statistics field:

        self.last_statistics = info.statistics.clone();
        

        self                                // If we add more methods we can chain.
    }
}

///  This program provides an FRIB/NSCLDAQ ringbuffer statistics
///  monitor.  Note that when run, it will run itself with an
///  added --server (portnum) option.  The spawned subprocess
///  is the actual monitor.  We will restart it if/when it exits.
///  this is needed because the monitor operates by 
///  attaching as a client to all rings, monitoring the
///  traffic through each ringbuffer in the system, and the
///  ringmaster will kill any processes attached to a ringuffer
///  that is being deleted e.g. with $DAQBIN/rinbuffer delete
///  Ring deletion is infrequent but ... we need to be able to
///  work in spite of that.
/// 
///  Assumptions:
///      The port manager runs on port 30000.  
///         In order to provide a stable point of contact, the parent interacts
///         with the port manager to allocate/advertise the service/port.  This
///         port is passed in to the child.  The long lifetime of the parent ensures
///         port stability so clients only need to translate the service once.
/// 
///
///
fn main() {
    let executable = env::current_exe().unwrap();
    let executable : String = executable.to_str().unwrap().to_string();

    // Run ourself with the --server 12345 parameter. e.g...unless we
    // have been run with the --server parameter.

    let args : Vec<String> = env::args().collect();
    if args.len() == 3 {
        if args[1] == "--server" {
            let port = args[2].parse::<u16>().unwrap();
            monitor(port);
            return;
        }
    }
    // We are the parent... if there's already a service
    // registered, we exit with a message.
    // Otherwise we register our service

    // Loop on running ourselves until we exit with the --server option..
    
    let mut pman = portman_client::Client::new(30000);      // port manager port hard  coded.
    if already_advertised(&mut pman, SERVICE_NAME) {
        eprintln!(
            "I think I'm already running as someone is advertising the {} service", 
            SERVICE_NAME
        );
        return;
    } 
    let port = allocate_port(&mut pman, SERVICE_NAME);
    println!("Allocated port {}", port);
    let port_string = port.to_string();
    loop {
        let output = process::Command::new(&executable)
            .arg("--server").arg(&port_string)
            .output()
            .expect("Failed to respawn");
        println!("Subprocess exited stdout: ");
        io::stdout().write_all(&output.stdout).unwrap();
        println!("\nStderr: ");
        io::stdout().write_all(&output.stderr).unwrap();
        io::stdout().flush().unwrap();
    }
}
///
/// Called to see if our service is already advertised.
/// returns true if the service_name is already advertised by the port manager.
fn already_advertised(client : &mut portman_client::Client , service_name: &str) -> bool {
    let allocation = client.find_my_service(service_name).unwrap();  // Pointless if portman isn't running.
    return allocation.len() == 1;    
}
/// Allocate my service.
/// panics on failure.

fn allocate_port(client: &mut portman_client::Client, service_name: &str) -> u16  {
    client.get(service_name).unwrap()
}

/// Analyze the ring data.  Note that given our buffer shape,
/// the the initial part (or even all of the buffer) might not
/// be a ring item so the parameters are:
/// 
/// n - amount of data from the ring buffer.
/// data - the data from the ring.
/// next_item_offset - number of bytes remaining in the last ringitem from the last read.
///       This will also be updated.
/// 
/// The returned tuple is in order:
///     bool - new_run - true if a BEGIN_RUN item was encountered int the data.
///     Option<usize> - Number of events in the data since the last BEGIN_RUN in the data.
///     usize - Total number of events in the data.
///     Option<usize> - Number of bytes since in the data since he last BEGIN_RUN in the data.
/// 
/// A note on the "Since the last BEGIN_RUN..." items. Those are only meaningful
/// if the 
///     
fn analyze_ring_data(nbytes : usize, data : &[u8], next_offset : &mut usize, residual: &mut usize) ->
    (bool, Option<usize>, usize, Option<usize>) {

    let mut result = 
        (false, None as Option<usize>, 0 as usize, None as Option<usize>);
    // Case when there's no full ring item in the data:
    if *next_offset > nbytes {
        *next_offset -= nbytes;            // Offset into the next chunk
        return (false, None, 0, None);
    }
    let mut p = *next_offset;
    
    let lsize = size_of::<u32>();
    
    while nbytes - p > lsize * 2 {    // there's room for a header.
    
        let size = u32::from_ne_bytes(data[p..p+lsize].try_into().unwrap());
        p += lsize;
        let item_type     = u32::from_ne_bytes(data[p..p+lsize].try_into().unwrap());

        // Count events.
        if item_type == rust_ringitem_format::PHYSICS_EVENT {
            result.2 += 1;
            if result.1.is_none() {
                result.1 = Some(1);

            } else {
                result.1 = Some(result.1.unwrap() + 1);
            }
            
        }
        // Update the # bytes since the last begin run.
        if result.3.is_none() {
            result.3 = Some(size as usize);
        } else {
            result.3 = Some(result.3.unwrap() + size as usize);
        }
        // Reset the per run counters if this is  a BEGIN_RUN item:

        if item_type == rust_ringitem_format::BEGIN_RUN {
            result.0 = true;                             // There was a begin run.
            result.1 = None;                             // no new events.
            result.3 = Some(size as usize);                             // For data we've had this item.
        }
        p += size as usize - 2*lsize;                            // next ring item.
        // If that took us off the end of the ring item, 
        // We need to set next_offset accordingly and return what we have:

        if p > nbytes {
            *next_offset = p - nbytes;
            return result;
        }
    }
    // If we got here and p < the data size?, there's a partial header left in we need to hold on to
    // so that we can glue that to the next chunk of data

    if p  <=  nbytes {              // This gets a resid if needed.
        *residual = nbytes - p;
    } else {
        *residual = 0;
    }


    result
}
///  send statistics updates to the main thread.
/// 
fn send_statistics(ch : &mpsc::Sender<UpdateMessage>, elapsed : time::Duration, stats : &RingBufferStatistics) {
    let msg = UpdateMessage::new(elapsed, stats);
    let _ = ch.send(msg);                             // Send the statistics.
}
///  Thread to monitor a single, named ringbuffer.
/// 
fn ring_monitor(name: &str, chan : mpsc::Sender<UpdateMessage>) {
    // Become a consumer of the ring that was passed in:

    let mut statistics = RingBufferStatistics::new(name);

    let uri = format!("tcp://localhost/{}", name);

    let mut consumer = ringmaster_client::RingBufferConsumer::attach(&uri)
        .expect("Monitor thread failed to attach as consumer");
    

    let mut data : [u8;BUFFER_SIZE] = [0; BUFFER_SIZE];
    let mut next_item_offset = 0;
    let mut residual = 0;
    let mut start_time = time::Instant::now();         // Start of stats gathering.
    loop {
        if let Ok(n) = consumer
            .consumer
            .timed_get(&mut data[residual..BUFFER_SIZE-1], time::Duration::from_secs(1)) {
            
            let (new_run, run_events, events, run_bytes) =
                analyze_ring_data(n, &data, &mut next_item_offset, &mut residual);
            statistics.count_bytes(n)
                .count_events(events); 
            if new_run {
                statistics.new_run();
                statistics.events_this_run  = run_events.unwrap();
                statistics.bytes_this_run   = run_bytes.unwrap();
               
            }
            // If there's a residual, we need to move those bytes to the bottom of the buffer for the next
            // read.  The number of bytes will be small (less than the size of a ring item header) so we
            // don't need to be fancy:

            for i in 0..residual {
                data[i] = data[(n-1) - residual + i];
            }
        }
        // See if we need to dump the statistics to the main thread:
        // if so we start then ext interval

       let elapsed = start_time.elapsed();
       if elapsed.as_secs() >= UPDATE_TIME {
            send_statistics(&chan, elapsed, &statistics);
            start_time = time::Instant::now();                    
       }
    }
    

}

fn follow_rings(list : &Vec<ringmaster_client::RingInformation>, sender : &mpsc::Sender<UpdateMessage>) {
    let mut thread_map : HashMap<String, thread::JoinHandle<()>> = HashMap::new(); // Map of threads we have.
    for ring in list.iter() {
        if !thread_map.contains_key(&ring.name) {
            let name = ring.name.clone();
            let chan = sender.clone();
            let handle = thread::spawn(move || {ring_monitor(&name, chan)});
            thread_map.insert(ring.name.clone(), handle);
        }
        
    }
    // in case the subprocesses exit:: try join them:
    // This double loop bit is ugly but to use the join
    // handle requires removing it from the map
    // and that soils the iterator.
    // therefore, one pass to figure out who's gone
    // and another pass to remove handles from the map, joinnig them.
    let mut to_delete = vec![];
    for k in thread_map.keys() {
        let handle = thread_map.get(&k.clone()).unwrap();
        if handle.is_finished() {
            
            to_delete.push(k.clone());
        }
    }
    // Delete the joined threads from the map:

    for k in to_delete.into_iter() {
        let handle = thread_map.remove(&k.clone()).unwrap();
        let _ = handle.join();
    }
}
///
/// This is the function thats' run by child processe:
/// 
/// The port paramteer is the port on which 
/// we will listen for connections from clients
/// asking for our statistics and so on.
/// 
/// For now, what we're going to do is periodically check the ringbuffer
/// status from the ringmaster.  If needed, we will spin off more
/// statistics gathering threads.  Since we will wind up being a client
/// of all rings, including proxies, eventually, if a ring is destroyed,
/// the ring master will destroy us which will cause our parent to respawn us
/// ..and the dance will start all over again.
/// 
/// Ring deletion is a rare occurence.
/// 
fn monitor(port : u16) {
    let (sender, receiver) = mpsc::channel::<UpdateMessage>();
    let mut aggregated_stats = HashMap::<String, StatisticsAndRates>::new();
    loop {
        let mut c = ringmaster_client::Client::new("localhost");
        
        match c.list_rings() {
            Ok(list) => {
                follow_rings(&list, &sender)
            },
            Err(reason) => eprintln!("Failed to list rings {}", reason),
        }
        // Read any statistics updates from the monitor threads:

        // Note that Disconnected is legitimate since we have several
        // senders, one for each ringbuffer:
        // THe loop processes all pending messages from monitors.
        loop {
            let status = receiver.try_recv();
            if let Ok(msg) = status {
                let name = msg.statistics.name.clone();
                if ! aggregated_stats.contains_key(&name) {
                    // Insert a new entry for this ring:

                    aggregated_stats.insert(
                        name, 
                        StatisticsAndRates::new(&msg.statistics.name)
                    );
                }
                // Update the statistics.
                aggregated_stats
                    .get_mut(&msg.statistics.name.clone())
                    .unwrap()
                    .update(&msg);
            } else {
                break;
            }
        }

        thread::sleep(time::Duration::from_secs(5));
    }
}