
use std::env;
use std::process;
use std::{thread, time};
use std::io::{self, Write};
use portman_client;
use ringmaster_client;
use nscldaq_ringbuffer;
use std::collections::HashMap;

const SERVICE_NAME : &str = "RING_MONITOR";
const MB : u32 = 1024*1024;
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
///
///  Thread to monitor a single, named ringbuffer.
/// 
fn ring_monitor(name: &str) {
    // Become a consumer of the ring that was passed in:

    let uri = format!("tcp://localhost/{}", name);

    let mut consumer = ringmaster_client::RingBufferConsumer::attach(&uri)
        .expect("Monitor thread failed to attach as consumer");
    let mut bytes : usize = 0;                          // Total bytes transferred.
    

    let mut data : [u8;1024] = [0; 1024];
    loop {
        if let Ok(n) = consumer.consumer.timed_get(&mut data, time::Duration::from_secs(1)) {
            bytes = bytes + n;
        }
    }
    

}

fn follow_rings(list : &Vec<ringmaster_client::RingInformation>) {
    let mut thread_map : HashMap<String, thread::JoinHandle<()>> = HashMap::new(); // Map of threads we have.
    for ring in list.iter() {
        if !thread_map.contains_key(&ring.name) {
            let name = ring.name.clone();
            let handle = thread::spawn(move || {ring_monitor(&name)});
            thread_map.insert(ring.name.clone(), handle);
        }
        
    }
    // in case the subprocesses exit:: try join them:

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

    loop {
        let mut c = ringmaster_client::Client::new("localhost");
        
        match c.list_rings() {
            Ok(list) => {
                follow_rings(&list)
            },
            Err(reason) => eprintln!("Failed to list rings {}", reason),
        }
        thread::sleep(time::Duration::from_secs(5));
    }
}