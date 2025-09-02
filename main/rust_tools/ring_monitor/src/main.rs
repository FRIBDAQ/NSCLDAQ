
use std::{env, net, process, thread, time};
use std::sync::mpsc;
use std::io::{self, Write};
use portman_client;
use ringmaster_client;
use rust_ringitem_format;
use std::collections::HashMap;
use serde::Serialize;
use serde_json;
use std::net::{TcpListener, TcpStream};


const SERVICE_NAME : &str = "RING_MONITOR";
const BUFFER_SIZE : usize = 1024;
const UPDATE_TIME : u64  = 5;       // How often to update statistics -> main thread.
const RING_POLL_INTERVAL : u64  = 1; // Secs between polls for new rings.

///
/// The information we want to maintain for each ringbuffer is:
/// 
#[derive(Clone, Debug, Serialize, PartialEq)]
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
        self.bytes = self.bytes.wrapping_add(bytes);
        self.bytes_this_run =self.bytes_this_run.wrapping_add(bytes);
        self
    }
    /// Count some events:
    ///
    fn count_events(&mut self, events: usize) -> &mut RingBufferStatistics {
        self.events = self.events.wrapping_add(events);
        self.events_this_run = self.events_this_run.wrapping_add(events);
        self
    }
}

///  This is the structure of statistics update messages sent
///  to the main thread by the monitor threads:
/// 
#[derive(Debug, PartialEq)]
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
#[derive(Clone, Serialize, Debug, PartialEq)]
struct StatisticsAndRates {
    name             : String,               // yes duplicate info but for easier JSON processing.
    cum_statistics   : RingBufferStatistics, // Note this has the ring name.
    byte_rate         : f64,
    event_rate        : f64,
    byte_per_run_rate : f64,
    evts_per_run_rate : f64
}

impl StatisticsAndRates {
    // Create a newly initialized struc.
    // This requires a ring name.

    fn new(ring: &str) -> StatisticsAndRates {
        StatisticsAndRates {name: String::from(ring), cum_statistics: RingBufferStatistics::new(ring), 
             byte_rate: 0.0, event_rate: 0.0, byte_per_run_rate: 0.0, evts_per_run_rate: 0.0 }
    }
    fn update(&mut self, info : &UpdateMessage) -> &mut Self {
        // Let's be sure the message really was for us.
        // panic if not:

        if info.statistics.name != self.cum_statistics.name {
            let msg = 
                format!("BUGCHECK - StatiticsAndRates::update called with mismatched rings was {} should be {}", 
                info.statistics.name, self.cum_statistics.name
            );
            panic!("{}", msg);
        }


        // Need differences from last and now to get rates:

        let dt : f64 = info.interval.as_secs_f64();
        self.byte_rate = (info.statistics.bytes - self.cum_statistics.bytes) as f64 / dt;
        self.event_rate = (info.statistics.events - self.cum_statistics.events) as f64/dt;
        
        // If we started a new run and the events are < than last time, then we start from 0.
        // To compute the rate.

        if self.cum_statistics.bytes_this_run > info.statistics.bytes_this_run {
            // New run so:

            self.cum_statistics.bytes_this_run = 0;
            self.cum_statistics.events_this_run = 0;
        }
        self.byte_per_run_rate = 
            (info.statistics.bytes_this_run - self.cum_statistics.bytes_this_run) as f64 / dt;
        self.evts_per_run_rate =
            (info.statistics.events_this_run - self.cum_statistics.events_this_run) as f64 /dt;

        // Updtae the last statistics field:

        self.cum_statistics = info.statistics.clone();
        

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
///   To get the current idea of the statistics over all rings,
/// Forma connection to this program and you will receive a JSON string
/// containing the statistics.  This will be a serialization of an array of
/// StatistcsAdRates structs.
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
    
    while nbytes - p >= lsize * 2 {    // there's room for a header.
    
        let size = u32::from_ne_bytes(data[p..p+lsize].try_into().unwrap());
        p += lsize;
        let item_type     = u32::from_ne_bytes(data[p..p+lsize].try_into().unwrap());
        p += lsize;                       // Count the type field.

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

        if p >= nbytes {
            *next_offset = p - nbytes;
            if p == nbytes {
                *residual  = 0;
            }
            return result;
        }
    }
    // If we got here and p < the data size?, there's a partial header left in we need to hold on to
    // so that we can glue that to the next chunk of data

    if p  <  nbytes {              // This gets a resid if needed.
        *residual = nbytes - p;
    } else {
        *residual = 0;
    }
    // offset is always zero if we fell out of the loop.  Caller will put the residual at the
    // beginning of the next data buffer.

    *next_offset = 0;                   
                                 

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
                analyze_ring_data(n+residual, &data, &mut next_item_offset, &mut residual);
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

fn follow_rings(
    list : &Vec<ringmaster_client::RingInformation>, 
    sender : &mpsc::Sender<UpdateMessage>, 
    thread_map: &mut HashMap<String, thread::JoinHandle<()>>) {
    
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
/// Provides the current aggregated statistics to the peer:
/// 
fn serve_statistics(sock: &mut TcpStream, stats: &HashMap<String, StatisticsAndRates>) {
    // We want a JSON array of StatisticsaAnd Rates so:

    let mut statsv = vec![];
    
    for v in stats.values() {
        statsv.push(v.clone());
    }

    let mut msg_string = serde_json::to_string(&statsv).unwrap();
    msg_string += "\n";
    let _ = sock.write_all(msg_string.as_bytes());
    let _ = sock.flush();                            // Make sure it's been sent.

    let _ = sock.shutdown(net::Shutdown::Both);       // Shut down the connection.
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
    let mut thread_map : HashMap<String, thread::JoinHandle<()>> = HashMap::new(); // Map of threads we have.
    let (sender, receiver) = mpsc::channel::<UpdateMessage>();
    let mut aggregated_stats = HashMap::<String, StatisticsAndRates>::new();
    
    let ip_spec = format!("0.0.0.0:{}", port);     // Listener specification.
    let server = TcpListener::bind(&ip_spec).expect("Cant start server");
    let _ = server.set_nonblocking(true);                           // don't stop the loop for connections.
    loop {
        let mut c = ringmaster_client::Client::new("localhost");
        
        match c.list_rings() {
            Ok(list) => {
                follow_rings(&list, &sender, &mut thread_map)
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
                break;                  // No more messages this time.
            }
        }
        // Handle connections by sending the aggregated statistics back to them:

        loop {
            let stat = server.accept();
            if let Ok((mut sock, _peer)) = stat {
                serve_statistics(&mut sock, &aggregated_stats);
            } else {
                // assume no connections.
                break;
            }
        }

        thread::sleep(time::Duration::from_secs(RING_POLL_INTERVAL));
    }
}

#[cfg(test)]
mod rbstat_tests {
    // Tests for the RingBufferStatistics implementation.
    use crate::*;

    #[test]
    fn new_1() {
        let stats = RingBufferStatistics::new("testing");
        assert_eq!(
            RingBufferStatistics {
                name: String::from("testing"), 
                bytes : 0, events: 0, bytes_this_run : 0, events_this_run: 0
            },
            stats
        );
    }
    #[test]
    fn count_bytes_1() {
        let mut stats = RingBufferStatistics::new("testing");
        stats.count_bytes(10);
        assert_eq!(
            RingBufferStatistics {
                name: String::from("testing"), 
                bytes : 10, events: 0, bytes_this_run : 10, events_this_run: 0
            },
            stats
        );
    }
    #[test]
    fn count_events_1() {
        let mut stats = RingBufferStatistics::new("testing");
        stats.count_events(5);
        assert_eq!(
             RingBufferStatistics {
                name: String::from("testing"), 
                bytes : 0, events: 5, bytes_this_run : 0, events_this_run: 5
            },
            stats
        );
    }
    #[test]
    fn new_run_1() {
        let mut stats = RingBufferStatistics::new("testing");
        stats.count_events(5)
            .count_bytes(10)
            .new_run();   // Should only clear the *this_run values:

        assert_eq!(
            RingBufferStatistics {
                name: String::from("testing"), 
                bytes : 10, events: 5, bytes_this_run : 0, events_this_run: 0
            },
            stats
        );

    }
}
#[cfg(test)]
mod updmsg_tests {
    use crate::*;
    use std::time::Duration;
    // Test the implementation of update messages:

    use crate::RingBufferStatistics;

    #[test]
    fn new_1() {
        let mut stats = RingBufferStatistics::new("test");
        stats.count_bytes(100)
            .count_events(10)
            .new_run()                 // Zero the run countes.
            .count_bytes(100)
            .count_events(10);              // More data.

        let update_time = Duration::from_secs(2);   // Yeah slow rate but meh.

        let msg = UpdateMessage::new(update_time, &stats);

        assert_eq!(
            UpdateMessage {
                interval: update_time,
                statistics: RingBufferStatistics {
                    name: String::from("test"),
                    bytes: 200, events: 20, bytes_this_run: 100, events_this_run: 10
                }
            },
            msg
        );
    }
}
#[cfg(test)]
mod sandr_tests {
    // Tests for impl StatisticsAndRates

    use crate::*;
    use std::time::Duration;

    #[test]
    fn new_1() {
        // Test proper initialization:

        let stats = StatisticsAndRates::new("test");
        assert_eq!(
            StatisticsAndRates {
                cum_statistics : RingBufferStatistics {
                     name: String::from("test"), 
                     bytes: 0, events: 0, bytes_this_run: 0, events_this_run: 0 },
                byte_rate: 0.0, event_rate: 0.0,
                byte_per_run_rate: 0.0, evts_per_run_rate: 0.0
            }, stats
        );
    }
    #[test]
    fn update_1() {
        // Test update and rate computation with no new run:

        let mut stats = StatisticsAndRates::new("test");
        let interval = Duration::from_secs(1);
        let mut incr_stat = RingBufferStatistics::new("test");
        incr_stat.count_bytes(100)
            .count_events(5);
        let msg = UpdateMessage::new(interval, &incr_stat);

        stats.update(&msg);

        // Check the rates should be pretty easy with intervals like 1.0:

        assert_eq!(
            StatisticsAndRates {
                cum_statistics: incr_stat.clone(),
                byte_rate: 100.0,
                event_rate: 5.0,
                byte_per_run_rate : 100.0,
                evts_per_run_rate: 5.0
            }
            ,stats
        );

    }
    #[test]
    fn update_2() {
        // Simulate a message update that indicates an new run started.

        let mut stats = StatisticsAndRates::new("test");
        let interval = Duration::from_secs(1);
        let mut incr_stat = RingBufferStatistics::new("test");
        incr_stat.count_bytes(100)
            .count_events(5);
        let msg = UpdateMessage::new(interval, &incr_stat);

        stats.update(&msg);        // Establishes baseline counts:

        incr_stat.new_run()
            .count_bytes(50)
            .count_events(2);             // Should make stats thing this is a new run.
        let msg = UpdateMessage::new(interval, &incr_stat);
        stats.update(&msg);
        
        assert_eq!(
            StatisticsAndRates {
                cum_statistics : RingBufferStatistics {
                    name : String::from("test"), 
                    bytes: 150, events: 7, bytes_this_run: 50, events_this_run: 2

                },
                byte_rate: 50.0, event_rate: 2.0, byte_per_run_rate : 50.0, evts_per_run_rate: 2.0
            },
            stats
        )

    }
    #[test]
    fn update_3() {
        // Two updates that change rate but no runs:

        let mut stats = StatisticsAndRates::new("test");
        let interval = Duration::from_secs(1);
        let mut incr_stat = RingBufferStatistics::new("test");
        incr_stat.count_bytes(100)
            .count_events(5);
        let msg = UpdateMessage::new(interval, &incr_stat);

        stats.update(&msg);        // Establishes baseline counts:

        incr_stat.count_bytes(50)
            .count_events(2);             // Should make stats thing this is a new run.
        let msg = UpdateMessage::new(interval, &incr_stat);
        stats.update(&msg);
        
        assert_eq!(
            StatisticsAndRates {
                cum_statistics : RingBufferStatistics {
                    name : String::from("test"), 
                    bytes: 150, events: 7, bytes_this_run: 150, events_this_run: 7

                },
                byte_rate: 50.0, event_rate: 2.0, byte_per_run_rate : 50.0, evts_per_run_rate: 2.0
            },
            stats
        )

    }
}
// The next test module requires the FRIB/NSCLDAQ port manager be running in the system.

#[cfg(test)]
mod port_tests {
    use portman_client::Client;

    use crate::already_advertised;

    #[test]
    fn allocate_1() {
        // allocate:

        let mut  c = Client::new(30000);
        let ptest = c.get("RINGMON_TEST1");
        assert!(ptest.is_ok());


    }
    #[test]
    fn check_2() {
        let svc = "RINGMON_TEST2";
        let mut c = Client::new(30000);

        assert!(!already_advertised(&mut c, svc));
        let ptest = c.get(svc);  // so its distint.
        assert!(ptest.is_ok());

        // New should show as advertised:

        assert!(already_advertised(&mut c, svc));
    }
}
#[cfg(test)]
mod analyze_tests {
    // Tests for analyze_ring_data

    // Utitity to put a ring item in a buffer:
    // Returns the next offset:
    fn add_item(item: &RingItem, buffer: &mut[u8], offset : usize) -> usize {

        // Get the bits and pieces of the ring item as byte things.
        let mut o = offset;
        let bytes = item.size().to_ne_bytes();
        let t          = item.type_id().to_ne_bytes();
        let body            = item.payload().as_slice();

        buffer[o..o+size_of::<u32>()].copy_from_slice(&bytes);
        o += size_of::<u32>();
        buffer[o..o+size_of::<u32>()].copy_from_slice(&t);
        o += size_of::<u32>();

        buffer[o..o+body.len()].copy_from_slice(body);


        item.size() as usize + offset
    }

    // Zero bytes is well handled (though we get insulated from that)

    
    use crate::*;
    use rust_ringitem_format::*;
    #[test]
    fn zero() {
        let data : [u8;BUFFER_SIZE] = [0;BUFFER_SIZE];
        let mut resid = 0;
        let mut next = 0;
        let result = analyze_ring_data(0, &data, &mut next, &mut resid );
        assert_eq!(false, result.0);
        assert!( result.1.is_none());
        assert_eq!(0, result.2);
        assert!(result.3.is_none());
        assert_eq!(0, resid);
        assert_eq!(0, next);
    }
    #[test]
    fn begin_0() {
        // Buffer contains  a begin run but no event.

        let begin_run = state_change::StateChange::new_without_body_header(
            state_change::StateChangeType::Begin,
            123, 0, 1, "This is a title", None
        );

        // Now we need to put it in the buffer:
        let raw   = begin_run.to_raw();
        let mut buffer: [u8;BUFFER_SIZE] = [0;BUFFER_SIZE];
        let nbytes = add_item(&raw, &mut buffer, 0);

        let mut offset = 0;
        let mut residual = 0;
        let result = analyze_ring_data(nbytes, &buffer, &mut offset, &mut residual);

        assert_eq!(0, offset);
        assert_eq!(0, residual);

        assert!(result.0);
        assert!(result.1.is_none());
        assert_eq!(0, result.2);
        assert!(result.3.is_some());
        assert_eq!(raw.size() as usize, result.3.unwrap());

    }
    #[test]
    fn begin_1() {
        // begin with a 'next offset'...by adding part of a physics item. after the begin item:

        let begin_run = state_change::StateChange::new_without_body_header(
            state_change::StateChangeType::Begin,
            123, 0, 1, "This is a title", None
        );

        // Now we need to put it in the buffer:
        let raw   = begin_run.to_raw();
        let mut buffer: [u8;BUFFER_SIZE] = [0;BUFFER_SIZE];
        let mut nbytes = add_item(&raw, &mut buffer, 0);

        // add part of a physics item by hand:
        let s: u32 = 125;
        let size  : [u8;4] = s.to_ne_bytes();
        let phystype : [u8;4] = PHYSICS_EVENT.to_ne_bytes();

        // add the header to the buffer but no body:

        buffer[nbytes..nbytes+size_of::<u32>()].copy_from_slice(&size);
        nbytes += size_of::<u32>();
        buffer[nbytes..nbytes+size_of::<u32>()].copy_from_slice(&phystype);
        nbytes += size_of::<u32>();

        let mut offset = 0;
        let mut residual = 0;
        let result = analyze_ring_data(nbytes, &buffer, &mut offset, &mut residual);

        // Still care that the result is correct:

        
        assert!(result.0);
        assert!(result.1.is_some());
        assert_eq!(1, result.1.unwrap());
        assert_eq!(1, result.2);
        assert!(result.3.is_some());
        assert_eq!((125+raw.size()) as usize, result.3.unwrap());

        // Should not be a residual since all headers fit:

        assert_eq!(0, residual);

        // However offset to the next item in the next buffer should be:

        assert_eq!(125 - 2*size_of::<u32>(), offset);

    }

    #[test]
    fn begin_2() {
        // Begin run and a chunk at the end of the data that isn't big enough for
        // a ring item:

        // begin with a 'next offset'...by adding part of a physics item. after the begin item:

        let begin_run = state_change::StateChange::new_without_body_header(
            state_change::StateChangeType::Begin,
            123, 0, 1, "This is a title", None
        );

        // Now we need to put it in the buffer:
        let raw   = begin_run.to_raw();
        let mut buffer: [u8;BUFFER_SIZE] = [0;BUFFER_SIZE];
        let mut nbytes = add_item(&raw, &mut buffer, 0);

        let sbresid = size_of::<u32>()*2 - 1;  // Item header size -1.
        nbytes += sbresid;                            // Don't actually need data.

        let mut offset = 0; 
        let mut resid = 0;      // Should become sbresid.

        let _ = analyze_ring_data(nbytes, &buffer, &mut offset, &mut resid);

        assert_eq!(sbresid, resid);
    
    }

    #[test]
    fn offset_1() {
        // Test that input offsets are properly handled:

        let begin_run = state_change::StateChange::new_without_body_header(
            state_change::StateChangeType::Begin,
            123, 0, 1, "This is a title", None
        );

        // Now we need to put it in the buffer:
        let raw   = begin_run.to_raw();
        let mut buffer: [u8;BUFFER_SIZE] = [0;BUFFER_SIZE];
        let mut offset = 10;
        let nbytes = add_item(&raw, &mut buffer, offset);  // 10 bytes in.
        
        let mut resid =0;
        let result = analyze_ring_data(nbytes, &buffer, &mut offset, &mut resid);

        // offset and resid should back to zero.

        assert_eq!(0, offset);
        assert_eq!(0, resid);

        // Check the results of the analysis:

        assert!(result.0);              // there was a begin run.
        assert!(result.1.is_none());    // no physics events.
        assert_eq!(0, result.2);
        assert!(result.3.is_some());
        assert_eq!(raw.size() as usize, result.3.unwrap());
    }
    #[test]
    fn evenht_1() {
        // begin run with a bunch of physics events.

        let begin_run = state_change::StateChange::new_without_body_header(
            state_change::StateChangeType::Begin,
            123, 0, 1, "This is a title", None
        );

        // Now we need to put it in the buffer:
        let raw   = begin_run.to_raw();
        let brunsize = raw.size();
        let mut buffer: [u8;BUFFER_SIZE] = [0;BUFFER_SIZE];
        let mut nbytes = add_item(&raw, &mut buffer, 0);

        // Create a simple physics item:

        let mut event = event_item::PhysicsEvent::new(None);
        event.add(1 as u16); event.add(2 as u16); event.add(3 as u16);
        let raw = event.to_raw();

        for _ in 0..5 {
            nbytes = add_item(&raw, &mut buffer, nbytes);
        }

        // Ok we have 5 events after a begin run:

        let mut offset = 0;
        let mut resid = 1243;    // Should get reset.
        let result = analyze_ring_data(nbytes, &buffer, &mut offset, &mut resid);

        assert_eq!(0, offset);
        assert_eq!(0, resid);

        assert!(result.0);          // was a begin run.
        assert!(result.1.is_some());   // THere are post begin run events.
        assert_eq!(5, result.1.unwrap());  // There were 5 of them to be exact.
        assert_eq!(5, result.2);          // 2 total events.
        assert!(result.3.is_some());
        assert_eq!((brunsize + 5*raw.size()) as usize, result.3.unwrap());   // 

    }

    #[test]
    fn event_2() {
        // Some events, then a begin run, the more events.

        let mut buffer: [u8;BUFFER_SIZE] = [0;BUFFER_SIZE];
        let mut nbytes = 0;                // NO bytes there yet

        // A couple of physics events:

        let mut event = event_item::PhysicsEvent::new(None);
        event.add(1 as u16); event.add(2 as u16); event.add(3 as u16);
        let raw = event.to_raw();
        let evsize = raw.size();

        nbytes = add_item(&raw, &mut buffer, nbytes);

        // Create and add a begin run item:

        let begin_run = state_change::StateChange::new_without_body_header(
            state_change::StateChangeType::Begin,
            123, 0, 1, "This is a title", None
        );

        // Now we need to put it in the buffer:
        let raw   = begin_run.to_raw();
        let brunsize = raw.size();
        nbytes = add_item(&raw, &mut buffer, nbytes);

        // Now some more events:

        let raw = event.to_raw();
        for _ in 0..3 {
            nbytes = add_item(&raw, &mut buffer, nbytes);
        }

        // Review, we have 1 events of size evsize.
        // followed by a begin run item of brunsize
        // followed by 3 more events of evsize.
        let mut next_offset = 0;
        let mut residual = 1234;
        let result = 
            analyze_ring_data(nbytes, &buffer, &mut next_offset, &mut residual);
        
        assert_eq!(0, next_offset);
        assert_eq!(0, residual);

        assert!(result.0);     // There is a begin run.
        assert!(result.1.is_some()); // THere are events after the begin:
        assert_eq!(3, result.1.unwrap()); // 3 of them.
        assert_eq!(4, result.2);  // 5 total events however.
        assert!(result.3.is_some());  // there are bytes after the begin.
        assert_eq!((brunsize+3*evsize) as usize, result.3.unwrap());

    }
}