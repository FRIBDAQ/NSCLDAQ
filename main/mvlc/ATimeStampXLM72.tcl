package provide ATimeStampXLM72 1.0
#===================================================================
# class ATimeStampXLM72
#===================================================================
itcl::class ATimeStampXLM72 {
	inherit AXLM72

	constructor {} {
		AXLM72::constructor 
	} {}

	public method Clear {} {Write fpga 0 1; Write fpga 0 0}
	public method Init {}
	public method sReadStamp {stack}

	# Wrappers to support as a Tcl readout driver:

	public method Initialize {controller}
	public method addReadoutList {stack}
	public method onEndRun {controller}
}

itcl::body ATimeStampXLM72::Init {} {
	AccessBus 0x10000
	Clear
	ReleaseBus
}

itcl::body ATimeStampXLM72::sReadStamp {stack} {
	sAccessBus $stack 0x10000
	sRead $stack fpga 4
	sRead $stack fpga 8
	sReleaseBus $stack
}

## Implement wrappers to make this a mvlc readout driver:

itcl::body ATimeStampXLM72::Initialize {controller} {
	setController $controller
	Init
}

itcl::body ATimeStampXLM72::addReadoutList {stack} {
	sReadStamp $stack
}

itcl::body ATimeStampXLM72::onEndRun {controller} {
	setController $controller

	# Add any actual end run stuff here.
}