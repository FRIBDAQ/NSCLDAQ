
use std::env;
use std::process;
use std::{thread, time};
use std::io::{self, Write};
use portman_client;

const SERVICE_NAME : &str = "RING_MONITOR";
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
/// This is the functino thats' run by child processe:
/// 
fn monitor(port : u16) {
    println!("Running server on {}", port);
    let sleep_time = time::Duration::from_secs(5);    // seconds to sleep:
    thread::sleep(sleep_time);
    return;
}