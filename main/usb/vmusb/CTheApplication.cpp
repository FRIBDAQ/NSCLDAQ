/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


#include <config.h>
#include "CTheApplication.h"
#include "Globals.h"
#include "event.h"

#include <COutputThread.h>
#include <TclServer.h>
#include <CVMUSBusb.h>
#include <CVMUSBFactory.h>

#include <TCLInterpreter.h>
#include <TCLLiveEventLoop.h>
#include <Exception.h>
#include <ErrnoException.h>
#include <tcl.h>
#include <DataBuffer.h>
#include <CRingBuffer.h>
#include <CAcquisitionThread.h>
#include <os.h>
#include <CRunState.h>
#include <CControlQueues.h>


#include <CPortManager.h>
#include <CMutex.h>

#include <vector>
#include <cstdlib>
#include <stdexcept>

#include <usb.h>
#include <sysexits.h>
#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>
#include <string.h>
#include <NSCLDAQLog.h>

#include "cmdline.h"


#include <string>



#ifndef NULL
#define NULL ((void*)0)
#endif

using namespace std;

//   Configuration constants:

static const char* versionString = VERSION ; // NSCLDAQ-version.

static const string   daqConfigBasename("daqconfig.tcl");
static const string   ctlConfigBasename("controlconfig.tcl");
static const uint32_t bufferCount(32);                      // Number of buffers that can be inflight.
static const uint32_t bufferSize(13*1024*sizeof(uint16_t)); // 13kword buffers...+pad
static       int      tclServerPort(27000);		    // Default value.


// Mutext log logging:

static CMutex loggingLock;

// Static member variables and initialization.

bool CTheApplication::m_Exists(false);



/*!
   Construct ourselves.. Note that if m_Exists is true,
   we BUGCHECK.
*/
CTheApplication::CTheApplication()
  : m_systemControl()
{
  if (m_Exists) {
    cerr << "Attempted to create more than one instance of the application\n";
    Tcl_Exit(EX_SOFTWARE);
  }
  m_Exists = true;
  m_pInterpreter = static_cast<CTCLInterpreter*>(NULL);
  Globals::running = false;
  
}
/*!
   Destruction is a no-op since it happens at program exit.
*/
CTheApplication::~CTheApplication()
{
}

/*!
   Thread entry point.  We don't care that much about our command line parameters.
   Note that the configuration files are as follows:
   $HOME/config/daqconfig.tcl     - Data acquisition configuration.
   $HOME/config/controlconfig.tcl - Controllable electronics configuration.
   We will not be returned to after calling startInterpreter().
   startInterpreter will eventually return control to the Tcl event loop which
   will exit directly rather than returning up the call chain.

   \param argc : int
      Number of command line parameters (ignored).
   \param argv : char**
      The command line parameters (ignored).

*/
int CTheApplication::operator()(int argc, char** argv)
{
  m_Argc   = argc;		// In case someone else wants them.
  m_Argv   = argv; 
  Globals::pApplication = this;
  
  // Process them via gengetopt:

  struct gengetopt_args_info parsedArgs;
  cmdline_parser(argc, argv, &parsedArgs);	// Actually fails if the commandline is incorrect.
  
  if (parsedArgs.quickstart_arg == quickstart_arg_on) {
    std::cerr << "Warning: You've enabled the --quickstart option\n";
    std::cerr << "This option will compute a digest on your --daqconfig file and\n";
    std::cerr << "if the VME crate has not power cycled and the file has not changed\n";
    std::cerr << "since the prior begin run, initialization and stack load won't be done\n";
    std::cerr << "at the start of a run.  While this can result in quicker begin runs.\n";
    std::cerr << "This really only has a hope of working if the daqconfig file is self contained\n";
    std::cerr << "by which I mean that it depends on no other files.\n";
    std::cerr << "Even then, there are cases where this may not work correctly.\n";
    std::cerr << "YOU HAVE BEEN WARNED.  If you are not sure this is for you turn --quickstart off.\n";
    m_canQuickstart = true;
  } else {
    m_canQuickstart = false;
  }
  
  if (parsedArgs.log_given) {
    m_logFile = parsedArgs.log_arg;
  }
  m_logLevel  = parsedArgs.debug_arg;
  initializeLogging();
  // Save the data source id:
  
  Globals::sourceId = parsedArgs.sourceid_arg;
  
  // If a timstamp lib was given save that as well:
  
  Globals::pTimestampExtractor = 0;
  if (parsedArgs.timestamplib_given) {
    size_t libLen = strlen(parsedArgs.timestamplib_arg);
    Globals::pTimestampExtractor = reinterpret_cast<char*>(malloc(libLen + 1));
    strcpy(
        Globals::pTimestampExtractor, parsedArgs.timestamplib_arg
    );
  }

  //  cerr << "VM-USB scriptable readout version " << versionString << endl;

  // If --enumerate was given, just enumerate the VM-USB modules connected to the
  // system on stdout and exit.

  if (parsedArgs.enumerate_given) {
    enumerateVMUSB();
    exit(-1);
  }
  if (parsedArgs.init_script_given) {
    m_systemControl.setInitScript(string(parsedArgs.init_script_arg));
  }
  try {				// Last chance exception catching...
    
    // How the USB controller is created depends on the parameters.
    // if host_given we need a remote server otherwise local with the serialno.
    //

    const char* connectionString;
    CVMUSBFactory::ControllerType type;
#ifdef HOST_ARG_DEFINED
    if (parsedArgs.host_given) {
      type             = CVMUSBFactory::remote;
      connectionString = parsedArgs.host_arg;
    } else {
#endif
      type             = CVMUSBFactory::local;
      connectionString = parsedArgs.serialno_given ? parsedArgs.serialno_arg :  0;
#ifdef HOST_ARG_DEFINED
    }
#endif

    Globals::pUSBController  = CVMUSBFactory::createUSBController(type, connectionString);
    
    std::cerr << "Attached VMUSB controller with firmware: " << std::hex << 
      Globals::pUSBController->readFirmwareID() << std::dec << std::endl;

    // If the idle busy workaround is enabled, then set the O1 to true as it used to be 
    // at power up.

#ifdef VMUSB_IDLE_BUSY_WORKAROUND
    uint32_t defaultDevSelect = Globals::deviceSelectorValue;

    // We need to mask out the busy, and set the O1 source to end of event, inverted.

    defaultDevSelect &= ~CVMUSB::DeviceSourceRegister::nimO1Busy;
    defaultDevSelect |= CVMUSB::DeviceSourceRegister::nimO1EndOfEvent | 
      CVMUSB::DeviceSourceRegister::nimO1Invert;
    Globals::pUSBController->writeDeviceSource(defaultDevSelect); // O1 is now asserted.
#endif

    // Set default configuration file names and then override with the ones supplied on
    // the command line (if any).
    
    setConfigFiles();
    if (parsedArgs.daqconfig_given) {
      Globals::configurationFilename = std::string(parsedArgs.daqconfig_arg);
    }
    if (parsedArgs.ctlconfig_given) {
      Globals::controlConfigFilename = std::string(parsedArgs.ctlconfig_arg);
    }
    
    initializeBufferPool();
    startOutputThread(destinationRing(parsedArgs.ring_given ? parsedArgs.ring_arg :
				      reinterpret_cast<const char*>(NULL)));

    // Replace the default server port if the user supplied one and start the Tcl server.

    if (parsedArgs.port_given) {
      std::string portString = parsedArgs.port_arg;
      if (portString == "managed") {      // Use port manager.
         // We'll use VMUSBReadout:connectionstring as our app.
         
         std::string appName="VMUSBReadout:";
         if (connectionString) {
            appName += connectionString;
         } else {
            appName += "FirstController";
         }
         CPortManager* pManager = new  CPortManager();      // Hold connection for app lifetime.
         tclServerPort = pManager->allocatePort(appName);
      } else {
        char* end;
        long port = strtol(portString.c_str(), &end, 0);
        if(end == portString.c_str()) {       // failed.
            std::cerr << "--port string must be either a number or 'managed'\n";
            cmdline_parser_print_help();
            Tcl_Exit(EXIT_FAILURE);
        } else {
            tclServerPort = port;
        }
      }
    }

    startTclServer();


    startInterpreter();
  }
  catch (string msg) {
    cerr << "CTheApplication caught a string exception: " << msg << endl;
  }
  catch (const char* msg) {
    cerr << "CTheApplication caught a char* exception " << msg << endl;
    Tcl_Exit(EXIT_FAILURE);

  }
  catch (exception& exc) {
    cerr << "CTheApplication caught an exception: " << exc.what() << endl;
    Tcl_Exit(EXIT_FAILURE);

  }
  catch (CException& error) {
    cerr << "CTheApplication caught an NCLDAQ exception: " 
	 << error.ReasonText() << " while " << error.WasDoing() << endl;

  }
  catch (...) {
    cerr << "CTheApplication thread caught an excpetion of unknown type\n";

  }
  /**
   *  If we get here an exception occured.  If the run is active,
   *  We'll try to shut it down.  In that way when we exit, the
   *  VMUSB Data fifo should be flushed.
   *
   * TODO:
   *   A couple of edge cases that are not handled here:
   *   *  If the readout thread has exited because it's thrown an exception
   *      too, the attempt to EndRun will deadlock.
   *   *  If the Router thread has given up the ghost unexpectly, the
   *      buffer queue could be full and again this will all neatly
   *      deadlock.
   */
  CRunState* pState = CRunState::getInstance();
  if (pState->getState() == CRunState::Active) {
    CControlQueues* pControl = CControlQueues::getInstance();
    pControl->EndRun();
  }
  return EX_SOFTWARE; // keep compiler happy, startInterpreter should not return.
}
/**
 * getInstance
 *    Convenience method to return the application instance to the caller.
 *
 *  @return CTheApplication*
 */
CTheApplication*
CTheApplication::getInstance()
{
  return Globals::pApplication;
}
/**
 * logStateChangeRequest
 *    issue a log item for state change request level severity (info).
 * @param msg - the message to log.
 * @note we do this in a critical section on loggingLock
 */
void
CTheApplication::logStateChangeRequest(const char* msg)
{
  if (!m_logFile.empty() ) {
    CriticalSection l(loggingLock);
    
    daqlog::info(msg);
  }
}
/**
 * logStateChangeStatus
 *
 * logs the completion status of a state change.
 *
 * @param msg - the message to log
 */
void
CTheApplication::logStateChangeStatus(const char* msg)
{
  if (!m_logFile.empty() ) {
    CriticalSection l(loggingLock);
    
    daqlog::debug(msg);
  }  
}
/**
 * logProgress
 *    Logs progress through an operation.
 * @param msg - the message.
 */
void
CTheApplication::logProgress(const char* msg)
{
  if (!m_logFile.empty() ) {
    CriticalSection l(loggingLock);
    
    daqlog::trace(msg);
  }
}

/*
   Start the output thread.  This thread is responsible for 
   reformatting and transferring buffers of data from the VM-USB to 
   spectrodaq.  This thread is continuously running for the life of the program.
   .. therefore we are sloppy with storage management.

   @param ring - std::ring that contains the name of the ring buffer into which data should be put.

*/
void
CTheApplication::startOutputThread(std::string ring)
{
  COutputThread* router = new COutputThread(ring, m_systemControl);
  router->start();
  m_pOutputThread = router;
  Os::usleep(500);


}
/* 
   Start the Tcl server.  It will listen on port tclServerPort, seee above..
   Again, the tcl server runs the lifetime of the program so we are 
   sloppy about storage management.
*/
void
CTheApplication::startTclServer()
{
  // we have to set the global pTclServer immediately because
  // the following line potentially will depend on the Globals::pTclServer
  Globals::pTclServer = new TclServer(m_systemControl);
  Globals::pTclServer->start(tclServerPort, Globals::controlConfigFilename.c_str(),
		   *Globals::pUSBController);

  // TclServer is a CSynchronizedThread which uses condition variables to ensure that
  // the calling thread waits until the spawned thread is done initializing. Because that
  // is done when start is called and the TclServer updates its state to running only after it
  // is done initializing, we are gauranteed to know where an error occurred or not by
  // checking isRunning.
  if ( ! Globals::pTclServer->isRunning() ) {
    string msg("Slow control subsystem failed to initialize. ");
    msg += "This is a fatal error.";
    throw runtime_error(msg);
  }
}
/*
    Start the Tcl interpreter, we use the static AppInit as a trampoline into the
    interpreter configuration and back to the interpreter event loop so the
    default Tcl event loop can be used.
*/
void
CTheApplication::startInterpreter()
{

  m_systemControl.run(m_Argc, m_Argv);
}


/**
 * Enumerate the set of VM-USB serial numgers that are currently active in the system.
 * The enumeration goes to cout.;
 *
 */
void
CTheApplication::enumerateVMUSB()
{
  try {
    vector<struct usb_device*> controllers = CVMUSBusb::enumerate();
    for (int i = 0; i < controllers.size(); i++) {
      std::string serial = CVMUSBusb::serialNo(controllers[i]);
      std::cout << "[" << i << "] : " << serial << std::endl;
    }
    cout.flush();
  }
  catch(std::string msg) {
    std::cerr << "Unable to enumerate VM-USB modules: " << msg << std::endl;
  }
}
/**
 *  initializeLogging
 *      Given the parsed values, initialize the logging system.
 */
void
CTheApplication::initializeLogging()
{
  if (!m_logFile.empty()) {
    daqlog::setLogFile(m_logFile);
    switch (m_logLevel) {
      case 0:
        daqlog::setLogLevel(daqlog::Info);
        break;
      case 1:
        daqlog::setLogLevel(daqlog::Debug);
        break;
      case 2:
    case 3:
        daqlog::setLogLevel(daqlog::Trace);
        break;
      default:
        std::cerr << "Invalid log level "
          << m_logLevel
          << "Must be 0, 1, or 2\n";
        exit(EXIT_FAILURE);
    }
    CVMUSB::m_logTransactions   = m_logLevel > 2;           // Export that to the full app.
  }
 
}
/*---------------------------------------------------------------------------
 *   Static utilties.
 */

/* 
  Set the configuration files to the global storage
*/
void
CTheApplication::setConfigFiles()
{
  Globals::configurationFilename = makeConfigFile(daqConfigBasename);
  Globals::controlConfigFilename = makeConfigFile(ctlConfigBasename);

}


/*
   Create the buffer pool.  The following are configurable parameters at the
   top of this file;
   - bufferCount  - Number of buffers to create.
   - bufferSize   - Size (in bytes) of the buffer (payload).

*/
void
CTheApplication::initializeBufferPool()
{
  Globals::usbBufferSize = bufferSize;
  for(uint i =0; i < bufferCount; i++) {
    DataBuffer* p = createDataBuffer(bufferSize);
    gFreeBuffers.queue(p);
  }
}

/*
   Make a configuration filename:  This is done by taking a basename
   and prepending the home directory and config subdir to its path:

*/
string
CTheApplication::makeConfigFile(string baseName)
{
  string home(getenv("HOME"));
  string pathsep("/");
  string config("config");
  string dir;
  
  // The user can define a CONFIGDIR env variable to move the configuration dir.

  if (getenv("CONFIGDIR")) {
    dir =getenv("CONFIGDIR");
  } else {
    dir = home + pathsep + config;
  }


  string result = dir +  pathsep + baseName;
  return result;

}

/**
 * Determine the output ring.  If one is specified that one is used.
 * if not, a ring named after the current logged in user is used instead.
 * 
 * @param pRingName - If not null, this is the name of the ring and overrides the default ring.
 *                    If null a default ring name is constructed and returned.
 * 
 * @return std::string
 * @retval Name of ring to which we should connnect.
 *
 * @throws std::string - If the ring name is defaulted but  we can't figure out what it should be
 *                       due to system call failures.
 */
std::string
CTheApplication::destinationRing(const char* pRingName)
{
  if (pRingName) {
    return std::string(pRingName);
  } else {
    return CRingBuffer::defaultRing();
  }
}

/** 
 * ExitHandler
 *   This is invoked when the application is about to exit.  If the acquisition thread
 *   is actively taking data, we stop data taking and flush the buffers.  This is an attempt
 *   to reduce the number of times the VM-USB is left in the hung state.
 * 
 * @param pData - Actually a pointer to the application (in case we need it for later).
 *
 */
void
CTheApplication::ExitHandler(ClientData pData)
{
  CAcquisitionThread* pReadout = CAcquisitionThread::getInstance();
  if (pReadout->isRunning()) {
    pReadout->stopDaq();	// Flushes buffers etc. too.
  }
}


/*-------------------------------------------------------------------------------------------*/

/*
  Initialize the application.  All we need to do is instantiate a CTheApplication object
  and pass it the argc/argv unmolested.

  @param argc - The count of command line words.
  @param argv - pointer to an array of command word pointers.

  

*/

int
main(int argc, char** argv)
{
  CTheApplication app;
  
  return app(argc, argv);
}

void* gpTCLApplication(0);	// Needed by the TclPlus library
