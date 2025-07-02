#===================================================================
# class ACAENV785
# 
# v0.1 S.J.Williams Dec 5th 2012
#      Supports CAEN V785(N) 32(16) Channel VME ADC 
#===================================================================
package provide ACAENV785 1.0


itcl::class ACAENV785 {
    private variable device
    private variable base
    private variable version

    constructor {ba} {
	set device ""
	set base $ba
    }

    destructor {}

# interactive functions
    public method GetVariable {v} {set $v}
    public method GetVersion {}
    public method EnableHiResThresholds {}
    public method SetThreshold {chan threshold}
    public method ReadThreshold {chan}
    public method SetThresholds {threshold}
    public method DisableChannel {channel}
    public method DisableChannels {start end}
    public method DataReset {}
    public method SoftwareReset {}
    public method SetReadoutMode {value}
    public method SetBitSet2 {bit}
    public method ClearBitSet2 {bit}
    public method ReadBitSet2 {}
    public method SetBitSet1 {bit}
    public method ClearBitSet1 {bit}
    public method ReadBitSet1 {}
    public method Init {}

    # Wrappers to allow use in mvlcgenerate:

    public method Initialize {controller}
    public method addReadoutList {rdolist}
    public method onEndRun {controller}

# stack functions
    public method sRead {stack}
}

# interactive functions implementation
itcl::body ACAENV785::GetVersion {} {
    # Get the version type of the 785 board. 0x1N refers to ECL inputs,
    # 0xEN refers to NIM inputs (where N is any hex number). This version
    # type affects the register map for the thresholds. Since we don't 
    # care about the subversion, discard it.

    # Can't read so we'll set to 0xE which allows larger thresholds.
    set version 0xe
    
}

itcl::body ACAENV785::SetBitSet2 {bit} {
    # Set a bit of the BitSet2 register. A write of 0 to any other
    # bit with this call will not unset those bits.
    
    #$device Write32D16 [expr $base+0x1032] [expr 1<<$bit]
    $device vmeWrite16 [expr $base+0x1032] $::CVMUSBReadoutList::a32UserData [expr 1<<$bit]
}

itcl::body ACAENV785::ReadBitSet2 {} {
    # Read the current contents of the BitSet2 register
    
    error "ACAENV785::ReadBitSet2 is not allowe for mvlcgenerate"
    
}

itcl::body ACAENV785::ClearBitSet2 {bit} {
    # Unset a bit of the BitSet2 register (requires a write of 1
    # to the bit of interest)

    #$device Write32D16 [expr $base+0x1034] [expr 1<<$bit]
    $device vmeWrite16 [expr $base+0x1034] $::CVMUSBReadoutList::a32UserData [expr 1<<$bit]
}
itcl::body ACAENV785::SetBitSet1 {bit} {
    # Set a bit of the BitSet1 register. A write of 0 to any other
    # bit with this call will not unset those bits.
    
    #$device Write32D16 [expr $base+0x1006] [expr 1<<$bit]
    $device vmewWrite16 [expr $base+0x1006] $::CVMUSBReadoutList::a32UserData [expr 1<<$bit]
}

itcl::body ACAENV785::ReadBitSet1 {} {
    # Read the current contents of the BitSet1 register
    error "ACAENV785::ReadBitSet1 is not supported by mvlcgenerate"
    
}

itcl::body ACAENV785::ClearBitSet1 {bit} {
    # Unset a bit of the BitSet1 register (requires a write of 1
    # to the bit of interest)

    #$device Write32D16 [expr $base+0x1008] [expr 1<<$bit]
    $device vmeWrite16 [expr $base+0x1008] $::CVMUSBReadoutList::a32UserData [expr 1<<$bit]
}
    

itcl::body ACAENV785::EnableHiResThresholds {} {
    # Enable (threshold << 1) comparison
    SetBitSet2 8
}
    
itcl::body ACAENV785::SetThreshold {chan threshold} {    
    # Sets lower level discriminator on ADC channel chan (referred to 
    # as zero suppression in manual). Action depends on firmware in card. 
    # From release 5.1 onward there are two resolution options, where the 
    # adc value is compared to either (threshold << 4) or (threshold << 1).
    # This option is dependant on if EnableHiResThresholds has been called.
    # The default is (threshold << 4).

    
    set regsize 2;   # assume 32 channels.
    
    # I think this is wrong - the multiplication should only happen if 
    # Hi res thresholds was called -- and even that doesn't do what Scott things
    # it does - it's really a high _level_ threshold alowing a greater range
    # for the thresholds but we're going to leave well enough alone.

    #$device Write32D16 [expr $base+0x1080+($chan*$regsize)] $thresholds
    $device vmeWrite16  [expr $base+0x1080+($chan*$regsize)] $::CVMUSBReadoutList::a32UserData $thresholds
}

itcl::body ACAENV785::ReadThreshold {chan} {
    # Read back lld
    # check that we know if we have a V785 or a V785N

    error "ACAENV785::ReadThreshold is not allowed for mvlcgenerate"

}

itcl::body ACAENV785::SetThresholds {threshold} {
    # set all thresholds to a common value

    # check that we know if we have a V785 or a V785N
    
    set nchan 32;   # assume 32 channels.

    for {set i 0} {$i < $nchan} {incr i} {
    	SetThreshold $i $threshold
    }
}

itcl::body ACAENV785::DisableChannel {channel} {
    # Disable a channel by setting a kill bit.

    
    set regsize 2;   # Assume 32 chans.

    #$device Write32D16 [expr $base+0x1080+($channel*$regsize)] 0x100

    $device vmeWrite16  [expr $base+0x1080+($channel*$regsize)] $::CVMUSBReadoutList::a32UserData 0x100
}

itcl::body ACAENV785::DisableChannels {start end} {
    # Disable a range of channels.
    for {set i $start} {$i < [expr $start+$end]} {incr i} {
	    DisableChannel $i
    }
}

itcl::body ACAENV785::DataReset {} {
    # Perform a data reset

    SetBitSet2 2
    ClearBitSet2 2
}

itcl::body ACAENV785::SoftwareReset {} {
    # Perform a software reset

    SetBitSet1 7
    ClearBitSet1 7
}


itcl::body ACAENV785::sRead {stack} {

    error "ACAENV785::sRead should never have been called!!"

    # Read data from the multiple event buffer (MEB). Needs to have
    # SetReadoutMode 0x0024 to enable single event, BERR generation
    # at end of event

    $device sReadSBLT32 $stack $base 34 0; # Read up to 34 x 32 bit words of data
}

itcl::body ACAENV785::SetReadoutMode {value} {
    # Set the single/full MEB readout and BERR generation modes
    set mode [expr $value&0xffff]
    $device Write32D16 [expr $base+0x1010] $value
}

itcl::body ACAENV785::Init {} {
    # Initialize card
    DataReset
    GetVersion
    SetReadoutMode 0x0024; # Enable BLOCK MODE AND BERR for single event block mode readout
}

#  Implement wrapper methods:

itcl::body ACAENV785::Initialize {controller} {
    set device $controller
    Init
}

itcl::body ACAENV758::addReadoutList {rdolist} {
    $rdolist addBlockRead32 $base $::CVMUSBReadoutList::a32UserBlock 34 0 
}

itcl::body ACAENV785::onEndRun {controller} {
    set device $controller

    # In case finalization is ever needed.
}