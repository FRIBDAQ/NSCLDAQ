#
#   ALevel3XLM72 for readout:
#

package provide ALevel3XLM72Readout 1.0
package require Itcl
package require xlm72

itcl::class ALevel3XLM72 {
    inherit AXLM72
    public variable filename

    global stoppedstate

    constructor {de sl} {
	AXLM72::constructor $de $sl
    } {}

    method XSet {dev offset value}

    method ResetTS {}

    method RunStop {stop}
    method sStamp  {stack}
    method sTrigger {stack}

    # Wrappers needed to make this an actual readout driver:

    method Initialize {controller}
    method addReadoutList {stack}
    method onEndRun {controller}
}

itcl::body ALevel3XLM72::ResetTS {} {
    AccessBus 0x10000

    XSet base 0x400050 1;	# Toggle the reset bit.
    XSet base 0x400050 0
    XSet base 0x40004c 1;	# Enable the timestamp counter.
    
    ReleaseBus
}

itcl::body ALevel3XLM72::RunStop stop {
    AccessBus 0x10000

    XSet base 0x400010 $stop

    ReleaseBus
}

itcl::body ALevel3XLM72::sStamp stack {
    sAccessBus  $stack 0x10000
    sRead       $stack fpga 72
    sRead       $stack fpga 84
    sReleaseBus $stack
}

itcl::body ALevel3XLM72::sTrigger stack {
    sAccessBus  $stack 0x10000
    sRead       $stack fpga 88
    sReleaseBus $stack
}

itcl::body ALevel3XLM72::XSet {name addr data} {
    Write $name $addr $data
}

#   Initialize - I suppose for this we must set the
#   controller and ??
# @param controllelr - vmusb like controller object.

itcl::body ALevel3XLM72::Initialize {controller} {
    SetController $controller


    
    # load the firmware:

    AccessBus 0x10000

    # Load the firmware:

    Configure $filename
    SetFPGABoot 0x10000
    BootFPGA

    ReleaseBus
}
##
# addReadoutList
#  @param stack - the stack to add to?
#  do we just read the stamp?  what is sTrigger for? Do we need to do that
# prior to the read?

itcl::body ALevel3XLM72::addReadoutList {stack } {
    sTrigger $stack;   # Do we need this?
    sStamp $stack
}
##
# onEndRun
#   Stujff done on end of run.. do we need to startstop? 
#
# @param controller - vmusb like controller object.

itcl::body ALevel3XLM72::onEndRun {controller} {
    SetController $controller

    # But what do we do???
}
