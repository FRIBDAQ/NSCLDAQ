/**
 * @file SampleDriver.cpp
 * @brief Template code for a samplel mvlcgenerate compiled driver
 * 
 * Compiled drivers allow users to extend the set of devices
 * that mvlcgenerate can generate configuration code for.
 * 
 * On the whole these drivers are quite similar to those suported
 * by VMUSBReaout (this is intentional, of course).  Because
 * of the fact that static configuration code is generated, rather 
 * than being run, drivers cannot perform VME reads.  
 * 
 * They must also, rather than
 * 
 * #include <CVMUSB.h>
 * #include <CVMUSBReadoutList.h>,
 * 
 * #include <mvlc/CVMUSB.h>
 * #include <mvlc/CVMUSBReadoutList.h>
 * 
 * To get the the classes that, rather than issuing operations,
 * will store them to be later dumped into the configuration.
 * 
 * A driver consists of two classes:
 * 
 * The driver itself, and a driver command which will be 
 * responsible for creating driver instances (the base class
 * can handle configuration and so on).
 * 
 * This all must be packaged (potentially with other drivers) in a 
 * shared object library that is loaded via the daqconfig script
 * using the Tcl load command.  The package initialization code
 * must then regiser the driver commands with the mvlcgenerate's 
 * configuration parser.
 * 
 * This example will show all of those bits and pieces.  
 * To compile this example you'll need to  use a command like:
 * 
 * g++ -shared -FPIC -olibSampleDriver.so SampleDriver.cpp -I$DAQINC \
 *    -I/usr/include/tcl8.6 
 * 
 * In that case, the initialization code must be a C function
 * named Sampledriver_Init 
 * 
 * See the documentation of the Tcl 'load' command at 
 * https://www.tcl-lang.org/man/tcl8.6/TclCmd/load.htm e.g.
 * 
 * In the event this library is in the current working directory,
 * your driver gets made known via:
 * 
 * load ./libSampleDriver.so; # Full path is needed.
 * 
 * After which instances of your device can be created using the
 * command your DriverCommand implements.
 */

// Headers you'll typically need:  Note: Pleaes specify the
// mvlc subdirectory in your includes rather than doing a -I so
// that there will be no conflicts to headers for VMUSBReadout which
// are in $DAQINC  so e.g. -I$DAQINC rather than -I$DAQINC/mvlc where
// an additional -I$DAQINC might pull in the wrong headers.

#include <mvlc/CReadoutHardware.h>
#include <mvlc/CReadoutModule.h>
#include <mvlc/DeviceCommand.h>
#include <mvlc/CVMUSB.h>
#include <mvlc/CVMUSBReadoutList.h>
#include <mvlc/MVLCConfigParser.h>
#include <XXUSBConfigurableObject.h>
#include <stdint.h>
#include <tcl.h>


 /**
  * @class
  *   SampleDriver
  * This class must be derived from a CReadoutHardware
  * Our sample driver will produce a marker when read it will
  * also demonstrate the initialization and end run methods.alignas
  * 
  * base class and may implement the following methods
  * 
  * * onAttach - called as the driver is being initialized.
  * * Initialize - Called to generate operations that need to be done
  *   when data taking is about to begin.
  * * addReadoutlist - Called to add to the list of operations that
  *   will be done to read an event.
  * * onEndRun - Called to generate operations that will be done as the
  *    run is ending.
  * 
  * Our sample driver will implement all of these with detailed
  * explanations of each method.
  */

class SampleDriver : public CReadoutHardware {
private:
    XXUSB::CConfigurableObject* m_pConfiguration;  // Pointer to config.
public:
    SampleDriver();
    ~SampleDriver();

    virtual void onAttach(XXUSB::CConfigurableObject& configuration);
    virtual void Initialize(CVMUSB& controller);
    virtual void addReadoutList(CVMUSBReadoutList& list);
    virtual void onEndRun(CVMUSB& interface);
};

/** 
 * SampleDriver constructor.
 *    At this point you don't konw where in VME space your
 * device is.  Typically this entry doesn't need to do anything
 * I'll initialize the m_Configuration pointer to null.  More
 * about that pointer in the onAttach Method. comments
 */

SampleDriver::SampleDriver() :
    m_pConfiguration(nullptr)
{}
/**
 * SampleDriver destructor.
 *   If you have created any dynamic data in your constructor, or
 * in any of your methods, you should delete it here.  I have not
 * so my destructor is boring:
 */
SampleDriver::~SampleDriver() {}

/**
 * onAttach
 *    As you know the driver command has a config subcommand that
 * can be used to configure driver instance options, 
 * like the device base address.  The actual configuration is maintained
 * in an XXUSB::CConfigurableObject that is passed in to us in this
 * method.   What onAttach typically does is keep a pointer to that 
 * object so it can be used to fetch the values of various configuration
 * options.  
 * 
 * XXUSB::CConfigurableObject allows for options with type checkers and
 * provides convenience methods for creating typically typed options.
 * We will create the following options:
 * 
 * - -markervalue - the value of the marker to put in the data.
 * - -resetloc    - A VME address to write to at initialization time.
 * - -resetvalue  - the value to write to -resetloc.
 * - -writeendrun - A bool, that if true will write to somewhere at endrun.
 * - -endrunloc   - Where to write at end run.
 * - -endrunvalue - Value to write and end run.
 * 
 * More on each of the rather odd values in the methods that use them.
 * 
 * @note Our caller retains ownership of the configuration and we should
 * not delete it if we are destroyed.alignas
 * 
 * @param configuration  - reference to our configuration object.
 */
void
SampleDriver::onAttach(XXUSB::CConfigurableObject& configuration) {
    m_pConfiguration = &configuration;

    // Marker is a 32 bit integer - we'll default it to zero.
    // no range checking is done.

    m_pConfiguration->addIntegerParameter("-marker", 0);

    // On initialization we'll do a 16 bit write to a VME
    // address as shown below.  The second addIntegerParameter
    // supplies range limits and a default value of 0xaaaa

    m_pConfiguration->addIntegerParameter("-resetloc");
    m_pConfiguration->addIntegerParameter("-resetvalue", 0, 0xffff, 0xaaaa);

    // on end run if -writeendrun is true we'll write a 16 bit value
    // to a vme VME location.  By default we don't do that.

    m_pConfiguration->addBooleanParameter("-writeendrun", false);
    m_pConfiguration->addIntegerParameter("-endrunloc");
    m_pConfiguration->addIntegerParameter("endrunvalue", 0, 0xffff, 0xbbbb);
}
/**
 * Initialize
 *    This is normally called to produce operations to initialize
 * a device.  In VMUSBReadout drivers, often VMUSBReaoutLists are
 * created and then executed.  This is supported, however the lists
 * created can't actually return any read data.  Their operations are
 * simply added to the set of operations memorized by the CVMUSB
 * object passed in to this method.
 * 
 * It is also important to remember that any delays must be added
 * to the CVMUSB or CVMUSBReadoutList (if used), as we are not
 * connected to an actual device, just generating operations
 * to perform.
 * 
 * @param controller - Referencs a CVMSUB oject tha will memorize the
 * commands we ask it to perform to be later dumped to the generated 
 * configuration
 * 
 * In our case we use -resetloc and -restvalue to write some value
 * to some VME location.  We use a32 non supervisory data space
 * for the address modifier (CVMUSBReadouList::a32UserData).
 * 
 * Note that, for compatiblity with VMUSBReadout, the CVMUSB
 * write operatinos do return an integer, this is always 0 indicating
 * success.
 */
void
SampleDriver::Initialize(CVMSUB& controller) {
    uint32_t addr = m_pConfiguration->getIntegerParameter("-resetloc");
    uint32_t value = m_pConfiguration->getIntegerparameter("-resetvalue");

    controller.vmeWrite16(addr, CVMUSBReadoutList::a32UserData, value);
}
/**
 * addReadoutList
 *    This is called to let the driver provide the VME operations needed
 * to read out the device.  In our case, we need to just add a 
 * marker to the output event.  The value of the marker is in our
 * configuration parameter -marker.  One difference between the
 * VMUSB and MVLC data is that for the VMUSB, markers are 16 bit items,
 * while they are 32 bit itms in the MVLC.  
 * 
 * @param list - list of operations that are being accumulaed for
 *     event readout.
 */
void
SampleDriver::addReadoutList(CVMUSBReadoutList& list) {
    uint32_t markerValue = m_pConfiguration->getIntegerParameter("-marker");
    list.addMarker(markerValue);
}

/**
 * onEndRun
 *    This is called to provide a sequence of operations to perform
 * as the run ends. Typically this is used to shutdown the device
 * the driver manages.  In  our case, we have three configuration
 * parameers that will drive our actions:
 * 
 * * -writeendrun - a boolean that has to be true for us to do anything.
 * * -endrunloc   - An address to write to.
 * * -endrunvalue - a 16 bit value to write.alignas
 * 
 * REMEMBER: We're not actually connected to hardware, we're just generating
 * operations that will be stored for use when the readout is running.
 * 
 * @param controller - the CVMUSB like object that will save our operations.
 * 
 */
void
SampleDriver::onEndRun(CVMUSB& controller) {
    if (m_pConfiguration->getBoolParameter("-writeendrun")) {
        uint32_t addr = m_pConfiguration->getIntegerParameter("-endrunloc");
        uint32_t value = m_pConfiguration->getIntegerParameter("-endrunvalue");
        controller.vmeWrite16(addr, CVMUSBReadoutList::a32UserData, value);
    }
}


//////////////////////////////////////////////////////////////////

/**
 * @class SampleDriverCommand
 *    This class actually will develop a Tcl command with the 
 * subcommands: 
 *    - create - create a new instance of a SampleDriver 
 *    - config - Configure the option values of the driver instance.
 *    - cget   - Introspect the option values of the driver instance.
 * 
 * Because of the regular structure of drivers, these subcommands
 * can be implemented in the base class, however the 'create' subcommand
 * needs a method to actually create the driver.
 * 
 * Therefore our command class looks like the definition below, where
 * the main work we have is in createDevice and the constructor.
 */
class SampleDriverCommand : public DeviceCommand {
public:
    SampleDriverCommand(CTCLInterpreter& interp, TCLConfigParser& parser);
    ~SampleDriverCommand();

protected:

    virtual CReadoutModule* createDevice(std::string name)    

};

/** 
 * SampleDriverCommand constructor.
 * 
 * Usually, this will just need to define the command name string,
 * which is done by calling the base class constructor.  For complex
 * drivers (e.g. drivers which manage other drivers), you may
 * need to retain a copy of the configuration parser so you can
 * locate other devices that have already been defined.  That's beyond
 * the scope of this example, however look at the the source code for
 * CCAENChain in our git repository for an example of this.alignas
 * 
 * @param  interp - references the interpreter on which the command
 * will be registered.
 * @param parser - references the parser object that will interpret the
 * daq configuration script.   See the mvlc/TclConfigParser.h file
 * for the services this parser provides.   In this case we don't need it.
 * Most simple drivers don't need it.
 * 
 * Note that the base class constructor takes three parameters,
 * the interpreter, the command name and the parser.  We'll define the
 * command 'example'  e.g. 'example create sample'  Makes a SampleDriver
 * instance named sample.
 * 
 * 
 */
SampleDriverCommand::SampleDriverCommand(
    CTCLInterpreter& interp, TCLConfigParser& parser) :
    DeviceCommand(interp, "example", parser)
{}

/**
 *  SampleDriverCommand destructor
 *    For most driver commands this can be empty.  However, if your
 * command creates dynamic storage, of course, it should be deleted 
 * here.
 */
SampleDriverCommand::~SampleDriverCommand() {}


/**
 * SampleDriver createDevice
 * 
 * What we actually create is a CReadoutModule objecct with a 
 * SampleDriver as its driver.  A CReadoutModule just bundles
 * together:
 *    - A driver instance,
 *    - The driver's configuration.
 *   
 *  These are held by the configuration parser in a dictionary indexed
 * by the device instance's name.  What we therefore must do is
 * create a CReadoutModule and set its driver to a new instance of
 * the SampleDriver.   The caller will take care of making the configuration
 * and calling our driver instance/s onAttach method to get that 
 * configuration set up.
 * 
 * @param name - Name of the instance being created - normally ignored.
 * @return CReadoutModule*  - pointer to the module we created.  This
 * must be created with new and ownership/delete responsibility passes 
 * to the caller.
 * 
 */
CReadoutModule*
SampleDriverCommand::create(std::string name) {
    CReadoutModule* result = new CReadoutModule;
    result->SetDriver(new SampleDriver);

    return result;
}

////////////////////////////////////////////////////////

/**
 * Tcl initialization code.
 *   This is a C call bindings function that is invoked by Tcl
 * when the shared object is loaded.  The name of the function
 * must be prefix_Init and a single raw interpreter object is 
 * passed in.  The prefix must be the name of the shared library (without
 * the preceding lib), first letter capitalized and the remainder lower
 * case.  E.g. we said we'd build as libSampleDriver.so  In that case,
 * our inintialization function must be Sampledriver_Init:
 * 
 * @param pInterp - pointer to raw interpreter.
 */

 extern "C" {               // Force C bindings.
    int Sampledriver_Init(Tcl_Interp* pInterp) {

        // Encapsulate the raw interpreter in a CTCLInterpreter object.
        // we must do a new here  because the destruction of the
        // object also destroys the raw interpreter prematurely.
        //
        CTCLInterpreter* interp = new CTCLInterpreter(pInterp);

        TCLConfigParser*  parser = TCLConfigParser::getInstance();
        parser->addExtension(new SampleDriverCommand(*interp, *parser));
    }
 }