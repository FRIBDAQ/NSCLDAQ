#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#    Genie Jhang
#	   FRIB
#	   Michigan State University
#	   East Lansing, MI 48824-1321

package provide mdpp 1.0

package require snit
package require VMUSBDriverSupport
package require cvmusb
package require cvmusbreadoutlist

snit::type mdpp {
    option -base            -default 0

    variable controller
    variable base

    # Address modifier
    variable setupAmod     0x09

    # Registers
    variable triggerSource 0x6058
    variable triggerOutput 0x605E
    variable monitorOn     0x607A
    variable monitorCh     0x607C
    variable monitorWave   0x607E

    constructor args {
       $self configurelist $args
    }

    method Initialize vme {
        #
        #  Get the controller as an object and pull the base out of the options.
        #
        set controller [::VMUSBDriverSupport::convertVmUSB $vme]
        set base $options(-base)

        puts "MDPP TCL driver is initialized!"
    }

    method Update vme {
        set controller [::VMUSBDriverSupport::convertVmUSB $vme]
    }

    method Set {vme parameter value} {
        if {$parameter eq "triggerSource"} {
            $controller vmeWrite16 [expr {$base + $triggerSource}] $setupAmod $value
        } elseif {$parameter eq "triggerOutput"} {
            $controller vmeWrite16 [expr {$base + $triggerOutput}] $setupAmod $value
        } elseif {$parameter eq "monitorOn"} {
            $controller vmeWrite16 [expr {$base + $monitorOn}] $setupAmod $value
        } elseif {$parameter eq "monitorCh"} {
            $controller vmeWrite16 [expr {$base + $monitorCh}] $setupAmod $value
        } elseif {$parameter eq "monitorWave"} {
            $controller vmeWrite16 [expr {$base + $monitorWave}] $setupAmod $value
        } elseif {[string index $parameter 0] eq "A"} {
            $controller vmeWrite16 [expr {$base + [string range $parameter 1 end]}] $setupAmod $value
        } else {
            return "ERROR - There's no such parameter: " $parameter
        }
    }

    method Get {vme parameter} {
        if {$parameter eq "triggerSource"} {
            return [$controller vmeRead16 [expr {$base + $triggerSource}] $setupAmod]
        } elseif {$parameter eq "triggerOutput"} {
            return [$controller vmeRead16 [expr {$base + $triggerOutput}] $setupAmod]
        } elseif {$parameter eq "monitorOn"} {
            return [$controller vmeRead16 [expr {$base + $monitorOn}] $setupAmod]
        } elseif {$parameter eq "monitorCh"} {
            return [$controller vmeRead16 [expr {$base + $monitorCh}] $setupAmod]
        } elseif {$parameter eq "monitorWave"} {
            return [$controller vmeRead16 [expr {$base + $monitorWave}] $setupAmod]
        } elseif {[string index $parameter 0] eq "A"} {
            return [$controller vmeRead16 [expr {$base + [string range $parameter 1 end]}] $setupAmod]
        } else {
            return "ERROR - There's no such parameter: " $parameter
        }
    }

    method addMonitorList vmusbList {}

    method processMonitorList data {
        return 0
    }

    method getMonitoredData {} {
        return [list]
    }
}
