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
use std::env;

fn main() {
    let executable = env::current_exe().unwrap();
    let executable : String = executable.to_str().unwrap().to_string();
    println!("Hello, world! from {}", executable);
}
