
# should be able to construct an ACAENV1290 and wrap it in
# a tcldriver.  What you cannot do is configure this as
# that appears hard coded with the InitReA3 method.  I know
# Scott W, the probable original author could have done it differently,
# but he chose not to, instead making this only usable for Chandana and his
# Rea3 diagnostics -- which probably means I really didn't have to do any
# of this.

package provide ACAENV1290 1.0


#===================================================================
# class ACAENV1290
#===================================================================
itcl::class ACAENV1290 {
	private variable device
	private variable base
	private variable matchWindowWidth
	private variable windowOffset
	private variable extraSearchWindowWidth
	private variable rejectMargin
	private variable triggerTimeSubtraction
	
	constructor {ba} {
		set device "";     # will be filled in by Initialize and OnEnd
		set base $ba
	}
	
	destructor {}

# interactive functions
	public method GetVariable {v} {set $v}
	public method SetControlRegister {value} {$device Write16 [expr $base+0x1000] $value}
	public method ReadStatusRegister {} {return [$device Read16 [expr $base+0x1002]]}
	public method Reset {}
	public method Clear {}
	public method WriteOpcode {opcode}
	public method ReadOpcode {}
	public method SetTriggerMatchingMode {}
	public method SetContinuousStorageMode {}
	public method ReadTriggerConfiguration {name}
	public method SetMatchWindowWidth {width}; # width in ns
	public method SetWindowOffset {offset}; # offset in ns >0 or <0
	public method SetExtraSearchWindowWidth {width}; # width in ns
	public method SetRejectMargin {margin}; # margin in ns
	public method EnableTriggerSubtraction {}
	public method DisableTriggerSubtraction {}
	public method SetEdgeDetection {config}
	public method SetResolution {code}
	public method SetMaximumHits {hits}
	public method EnableChannel {channel}
	public method DisableChannel {channel}
	public method EnableAllChannels {}
	public method DisableAllChannels {}
	public method EnableTDCMarkers {}
	public method DisableTDCMarkers {}
	public method EnableTDCErrors {}
	public method DisableTDCErrors {}
	public method InitReA3 {}

	# Interface methods:

	public method Initialize {controller}
	public method addReadoutList {list}
	public method onEndRun {controller}
# stack functions
	public method sRead {stack}
}

#interactive functions implementation
itcl::body ACAENV1290::Reset {} {
	$device vmeWrite16 [expr $base+0x1014] 0
	after 1000
}

itcl::body ACAENV1290::Clear {} {
	$device vmeWrite16 [expr $base+0x1016] 0
	after 1000
}

itcl::body ACAENV1290::WriteOpcode {opcode} {
	# Wait for micro ready:

	$device loopUntil16 [expr $base + 0x1039 $CVMUBSReadoutList::a32UserData 1 1
	
	#Write the opcode/data register.

	$device vmeWrite16 [expr $base+0x102e] $opcode
}

itcl::body ACAENV1290::ReadOpcode {} {
	error "mvlcgenerate does not support reading from devices"
}

itcl::body ACAENV1290::SetTriggerMatchingMode {} {
	WriteOpcode 0
	after 1000
	WriteOpcode 0x200
	
}

itcl::body ACAENV1290::SetContinuousStorageMode {} {
	WriteOpcode 0x100
	after 1000
	WriteOpcode 0x200
	
}

itcl::body ACAENV1290::ReadTriggerConfiguration {name} {
	error "ReadTriggerConfiguration is not supported for mvlcgenerate"
}

itcl::body ACAENV1290::SetMatchWindowWidth {width} {
	set value [expr int($width/25)&0xfff]; # convert in units of 25 ns
	WriteOpcode 0x1000
	WriteOpcode $value
	after 1000
	
}

itcl::body ACAENV1290::SetWindowOffset {offset} {
	set value [expr int($offset/25)]
	if {$value > 40} {
		set value 40
	} elseif {$value < 0} {
		set value [expr $value+65536]
	}
	set value [expr $value&0xfff]
	WriteOpcode 0x1100
	WriteOpcode $value
	after 1000
	
}

itcl::body ACAENV1290::SetExtraSearchWindowWidth {width} {
	set value [expr int($width/25)&0xfff]; # convert in units of 25 ns
	WriteOpcode 0x1200
	WriteOpcode $value
	after 1000
	
}

itcl::body ACAENV1290::SetRejectMargin {margin} {
	set value [expr int($margin/25)&0xfff]; # convert in units of 25 ns
	WriteOpcode 0x1300
	WriteOpcode $value
	after 1000
	
}

itcl::body ACAENV1290::EnableTriggerSubtraction {} {
	WriteOpcode 0x1400
	after 1000
	
	
}

itcl::body ACAENV1290::DisableTriggerSubtraction {} {
	WriteOpcode 0x1500
	after 1000
	
}

itcl::body ACAENV1290::SetEdgeDetection {config} {
	# 00: pair mode; 01: trailing only; 10: leading only; 11: trailing & leading
	set config [expr $config&3]
	WriteOpcode 0x2200
	WriteOpcode $config
	after 1000
	WriteOpcode 0x2300
	
}
	
itcl::body ACAENV1290::SetResolution {code} {
	# only valid in trailing & leading edge detection modes
	# 00: 800 ps; 01: 200 ps; 10: 100 ps; 11: 25 ps (default)
	set code [expr $code&3]
	WriteOpcode 0x2400
	WriteOpcode $code
	after 1000
	WriteOpcode 0x2600
	
}

itcl::body ACAENV1290::SetMaximumHits {hits} {
	# max hits is 2^(hits-1) up to hits=8; 9: no limit; >10: meaningless
	set code [expr $hits&0xf]
	WriteOpcode 0x3300
	WriteOpcode $code
	after 1000
	WriteOpcode 0x3400
	
}

itcl::body ACAENV1290::EnableChannel {channel} {
	set channel [expr $channel&0x1f]
	set code [expr 0x4000+$channel]
	WriteOpcode $code
	after 1000
}

itcl::body ACAENV1290::DisableChannel {channel} {
	set channel [expr $channel&0x1f]
	set code [expr 0x4100+$channel]
	WriteOpcode $code
	after 1000
}

itcl::body ACAENV1290::EnableAllChannels {} {
	WriteOpcode 0x4200
	after 1000
}

itcl::body ACAENV1290::DisableAllChannels {} {
	WriteOpcode 0x4300
	after 1000
}

itcl::body ACAENV1290::EnableTDCMarkers {} {
	WriteOpcode 0x3000
	after 1000
	WriteOpcode 0x3200
		
}

itcl::body ACAENV1290::DisableTDCMarkers {} {
	WriteOpcode 0x3100
	after 1000
	WriteOpcode 0x3200
	
}

itcl::body ACAENV1290::EnableTDCErrors {} {
	WriteOpcode 0x3500
	after 1000
}

itcl::body ACAENV1290::DisableTDCErrors {} {
	WriteOpcode 0x3600
	after 1000
}

itcl::body ACAENV1290::InitReA3 {} {
	Reset
	SetTriggerMatchingMode
	SetMatchWindowWidth 1500
	SetWindowOffset -1000
	SetMaximumHits 1
	DisableTDCMarkers
	DisableTDCErrors
	EnableTriggerSubtraction
	SetControlRegister 0x21; # enable BERR, INL
}

# stack functions implementation
itcl::body ACAENV1290::sRead {stack} {
	$stack addBlockRead32 $base $CVMUSBReadoutList::a32UserBlock 1024
}

itcl::body ACAENV1290::Initialize {controller} {
	set device $controller
	InitReA3
}
itcl::body ACAEN1290::addReadoutList {list} {
	sRead list
}
itcl::body onEndRun {controller} {
	set device $controller ;   # in case we need it sometime.
}