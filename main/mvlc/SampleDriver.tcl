##
# @file SampleDriver.tcl
# @brief Template code for a smple mvclgenerate Tcl driver in snit.
#
#  Tcl drivers allow users to extend the set of devices for which
# mvlcgenerate can generate code.  It is important to recall that
# these drivers run in a translator and are, therefore,
# not connected to any hardware.  This means they cannot
# perform VME reads.
#
#  In general a driver consists of a command ensemble that is
# then incorporated into the mvlctranslator using the 
# addtcldriver command.  Note that in Tcl the term
# command ensemble, refers to a command that has subcommands.
# Within Tcl itself, for example, the 'file' command is a command
# ensemble.   
#
#  In various Tcl object oriented extensions, like
#  snit, itcl, TclOO, a class can be thought of as a generator of command 
#  ensembles where the object is a command and its methods are subcommands.
#
#  Our snit driver will have the same configuration parameters as the Sample
# driver.  Note that the option validation methods supplied by VMUSBReadout
# may be useful but option value validation is beyond the scope of this
# example.
#
#  to use the sample driver, once we've got its code incoropriated
# into our config script (via source or package require), just
#
# ```tcl
#    set driverInstance [SampleDriver %AUTO%]
#    $driverInstance config ....;   # set the configuration options.
#    addtcldriver create sample -ensemble $driverInstance
# ```

##
#  Here is the driver class:
package require snit;    # need to include the snit package.


snit::type SampleDriver {
    #
    # Define the options and their default values:
    #
    option -marker -default 0
    option -resetloc
    option -resetvalue -default 0xaaaa
    option -writeendrun -default false
    option -endrunloc
    option -endrunvalue -default 0xbbbb

    ##
    #  Construtor - in snit, the parameter to the contructor is 
    #  potentially a list of option/values  the built in
    #  configurelist method processs those into our option database:
    # configure and cget are also built in methods.
    constructor args {
        $self configurelist $args
    }

    ##
    # Initialize 
    #   This is called by the wrapper to initialize the hardware.
    #  The parameter passed is a command ensemble whose methods are the 
    #  same as CVMUSB with parameters in the same order.
    #  In our case assume all is good and we have a -resultloc configured
    #  that makes some sense.
    # Note that variables containing the address modifier values are created
    # in the CVMUSBReadoutList namespace
    #
    # @param vme - a command ensemble that behaves like CVMUSB.
    #
    method Initialize vme {
        $vme vmeWrite16 $options(-resetloc) $::CVMUSBReadoutList::a32UserData $options(-resetvalue)
    }
    ##
    # addReadoutList
    #   Called to generate the read operations.  We just insert the -markervalue into the
    #
    # @param list - a command ensemble that behaves like a CVMUSBReadoutList.
    #
    method addReadoutList list {
        $list addMarker  $options(-marker)
    }

    ##
    # onEndRun
    #    Called to emit shutdown code.  If -writeendrun is true write
    #    -endrunvalue -> -endrunloc.
    #
    # @param vme - Command ensemble that behaves like a CVMUSB object.
    #
    method onEndRun vme {
        if {$options(-writeendrun)} {
            $vme vmeWrite16 $options(-endrunloc) \
                $::CVMUSBReadoutList::a32UserData $options(-endrunvalue)
        }
    }
}

###
#  The reminader of this file is sample usage of the driver.
#  Not that the if 0 at the beginning of this block effectively 
#  makes the entire block of code a comment:


if 0 {
    
    set driver [SampleDriver %AUTO%]
    $driver configure -marker 0x1245;    # Marker value
    $driver configure -resetloc 0x12340000 -resetvalue 0x6789; # Reset location and value.
    $driver configure -writeendrun true -endrunloc 0x12341000 -endrunvalue 1;  #end run config.

    addtcldriver create sample -ensemble $driver

    stack create event 
    stack config event -trigger nim1 -modules sample
}