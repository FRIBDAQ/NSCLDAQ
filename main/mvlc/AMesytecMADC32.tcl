package provide AMesytecMADC32 1.0

#===================================================================
# class AMesytecMADC32
#===================================================================
itcl::class AMesytecMADC32 {
	private variable device
	private variable base
	
	constructor {de ba} {
		set device $de
		set base $ba
	}
	destructor {}
	method setController {controller} {set device $controller}
# interactive functions
	public method GetID {} {
		error "[info class]::GetId - not supported in mvlcgenerate"
		#return [$device Read16 [expr $base+0x6004]]
	}
	public method GetFirmware {} {
		error "[info class]::GetFirmware - not supported in mvlcgenerate"
		return [$device Read16 [expr $base+0x600e]]
	}
	public method SetThreshold {channel value}
	public method SetThresholds {value}
	public method SetIRQLevel {level} {$device vmeWrite16 [expr $base+0x6010] $::CVMUSBReadoutList::a32UserData $level}
	public method SetDataLength {code} {$device vmeWrite16 $::CVMUSBReadoutList::a32UserData [expr $base+0x6032] $code}
	public method SetIRQVector {vector} {$device vmeWrite16 [expr $base+0x6012] $::CVMUSBReadoutList::a32UserData $vector}
	public method ResetFIFO {} {$device vmeWrite16 [expr $base+0x6034] $::CVMUSBReadoutList::a32UserData 0}
	public method SetMultiEvent {mode} {$device vmeWrite16 [expr $base+0x6036] $::CVMUSBReadoutList::a32UserData $mode}
	public method SetMaxTransfer {words} {$device vmeWrite16 [expr $base+0x601a] $::CVMUSBReadoutList::a32UserData $words}
	public method Init {}
# stack functions
	public method sRead {stack}
	public method sResetFIFO {stack}

# Wrapper functions to make this a readout driver:

	public method Initialize {controller}
	public method addeadoutList {stack}
	public method onEndRun {controller}
}

# interactive functions implementation
itcl::body AMesytecMADC32::SetThreshold {channel value} {
	$device vmeWrite16 [expr $base+0x4000+$channel*2] $::CVMUSBReadoutList::a32UserData $value
}

itcl::body AMesytecMADC32::SetThresholds {value} {
	for {set i 0} {$i < 32} {incr i} {
		SetThreshold $i $value
	}
}

itcl::body AMesytecMADC32::Init {} {
	SetIRQVector 0
	SetIRQLevel 0
	SetMultiEvent 3
	SetMaxTransfer 1
	SetThresholds 0
	SetDataLength 2
	ResetFIFO
}

# stack functions implementation
itcl::body AMesytecMADC32::sRead {stack} {
	$stack addFifoRead $base $::CVMUSBReadoutList::a32UserBlock 35 0
	#$device sReadSBLT32 $stack $base 35 0; # Read up to 35 x 32 bit words of data
#	$device sReadNBLT32 $stack [expr $base+0x6030] $base 0xff 0 
	sResetFIFO $stack
}

itcl::body AMesytecMADC32::sResetFIFO {stack} {
	$stack addWrite16 [expr $base+0x6034] $::CVMUSBReadoutList::a32UserData 0
	# $device sWrite16 $stack [expr $base+0x6034] 0
}

#  Wrapper bodies to make this a valid driver:

itcl::body AMesytecMADC32::Initialize {controller} {
	setController $controller
	Init;     # This is pretty unsatisfying - all hard coded, no threshold values etc.
}

itcl::body addReadoutList {stack} {
	sRead $stack
}

itcl::body onEndRun {controller} {
	setController $controller

	# Whever.
}
