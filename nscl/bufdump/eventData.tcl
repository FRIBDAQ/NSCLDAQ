#
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#             Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321
#

package provide eventData 1.0
package require snit

#------------------------------------------------------------------------------
# eventFileVariable
#       Encapsulates an event file variable.  These can either be
#       run variables or state variables.
#       The raw data takes the form of a valid tcl set command.
snit::type eventFileVariable {
    option -variable    {}

    constructor args {
        $self configurelist $args
    }
    #####
    # configure -variable  list
    #      Provides the variable string.. and validates it.
    # Parameters:
    #   list   - The setting string.
    #
    onconfigure -variable list {
        if {[llength $list] != 3} {
            error [list Too Small A List]
        }
        if {[lindex $list 0] ne "set"} {
            error [list No Set In Variable]
        }

        set options(-variable) $list
    }
    method getName {} {
        return [lindex $options(-variable) 1]
    }
    method getValue {} {
        return [lindex $options(-variable) 2]
    }
}
#---------------------------------------------------------------------------------
# packetDescription
#      Decodes and presents all of the fields of a packet description
#
# Options:
#   -packet    - The packet contents.
#
snit::type packetDescription {
    option -packet

    variable packetName
    variable packetId
    variable packetDescription
    variable packetVersion
    variable packetDate

    constructor args {
        $self configurelist $args

        set description [split $options(-packet) :]

        if {[llength $description] != 7} {
            error {Bad Packet Format}
        }


        set packetName [lindex $description 0]
        set packetId   [lindex $description 1]
        set packetDescription [lindex $description 2]
        set packetVersion [lindex $description 3]

        # Parsing the date is a bit harder:

        set intro [lindex $description 4]
        set min   [lindex $description 5]
        set tail [lindex $description 6]

        set date [format "%s:%s:%s" $intro $min $tail]

        if {[catch {set packetDate [clock scan $date]}] == 1} {
            set packetDate [clock seconds]
        }

    }
    #####
    # getName
    #     Retrieve the contents of the packetName variable.
    #
    method getName {} {
        return $packetName
    }
    ######
    # getId
    #     Get the packet id variable
    #
    method getId {} {
        return $packetId
    }
    #######
    # getDescription
    #     Retrieve the packet description
    #
    method getDescription {} {
        return $packetDescription
    }
    #####
    # getVersion
    #      Retrieve the packte vesion id.
    #
    method getVersion {} {
        return $packetVersion
    }
    #####
    # getTimeStamp
    #     Return the date/timestamp of the packet.
    #
    method getTimeStamp {} {
        return $packetDate

    }
}
#---------------------------------------------------------------------------
#
#  Event
#     This encapsualtes an event...allowing you to get individual packets
#     from the event if it has a packet structure.
# Options:
#    -packets       - defines the set of packets that are known to the
#                     software.  This is a list of packetDescription objets.
#    -event         - The event as a Tcl list.
#
snit::type Event {
    option  -packets {}
    option  -event   {}

    variable packetOffsets

    #####
    # configure -event
    #     Iterates through the event and builds the packetOffsets array.
    #     any existing array elements are destroyed.
    #
    onconfigure -event event {
        # Destroy any existing offsets:

        foreach element [array names packetOffsets] {
            unset packetOffsets($element)
        }

        # set the options variable.

        set options(-event) $event


        # Setup to traverse packets...

        set  size [lindex $options(-event) 0]
        set  body [lrange $options(-event) 1 end]
        incr size -1
        set packets 0
        set offset  1

        while {$size} {
            set pktSize [lindex $body 0]
            set pktId   [lindex $body 1]

            set packetInfo [$self PacketDescription $pktId]
            if {$packetInfo == ""} {
                return
            }

            if {$pktSize > [llength $body]} {
                error [list Event Format Error]
            }
            if {$pktSize > $size} {
                error [list Event Format Error]
            }
            set packetOffsets($packets) $offset
            incr offset $pktSize
            incr packets
            incr size -$pktSize
            set body [lrange $body $pktSize end]
        }
        return $packets
    }

    ######
    # packetCount
    #      Gets count of the number of >recognized< packets.
    #      Since there are no 'illegal' packet size/id combos,
    #      we can really only format events with know packet types.
    # Implicit Inputs:
    #     options(-event)    - Holds the event.
    #     options(-packets)  - Holds the list of known packets.
    method packetCount {} {
        return [llength [array names packetOffsets]]

    }
    ######
    #  getPacket  n
    #      Gets the body of the n'th packet.
    # Parameters n
    #    n Packet number to get.
    # Returns:
    #    A list of the following sort:
    #    Index        Contents:
    #     0           Packet description
    #     1           Packet body.
    #  Or empty if the packet requested does not exist.
    #
    method getPacket n {
        if {[array names packetOffsets $n]  == [list]} {
            return [list]
        }
        set offset   $packetOffsets($n)
        set pktSize  [lindex $options(-event) $offset]
        set pktId    [lindex $options(-event) [expr {$offset +  1}]]
        set last     [expr {$offset + $pktSize -1}]
        set body     [lrange $options(-event) [expr $offset+2] $last]

        return [list [$self PacketDescription $pktId] $body]
    }

    ####
    # PacketDescription id
    #      Locate the packet description for a specific type of packet.
    # Parameters:
    #   id   - The packet id.
    # Implicit Inputs:
    #   options(-packets)   - The list of packte descriptions
    # Returns:
    #   string:
    #      empty:      Packet id does not have a description in the list.
    #      nonempty:   Name of the packet description object.
    #
    method PacketDescription id {
        set id [expr {$id & 0xffff}]
        foreach packet $options(-packets) {
            set packetid [$packet getId]
            if {$id == $packetid} {
                return $packet
            }
        }
        return [list]
    }
}
#----------------------------------------------------------------------------
# scalerEntityFinder
#       Ensemble command for locating the entities within a scaler buffer.
#       Note that it is the caller's responsibility to determine that the
#       buffer this is run against is, in fact, a scaler buffer.
#
snit::type scalerEntityFinder {
#    pragma -hastypeinfo     no
#    pragma -hastypedestroy  no
#    pragme -hasinstances    no

    typemethod getEntityList buffer {
        set entities [lindex $buffer 6]
        set base 26
        set result [list]

        # Entities are just longwords:

        for {set i 0} {$i < $entities} {incr i; incr base 2} {
            lappend result $base
        }
        return $result

    }
}
#---------------------------------------------------------------------------
# physicsEntityFinder
#      Ensemble command for locating entities in a physics buffer.
#      The caller is responsible for determining that the buffer is
#      a physics buffer.
#
snit::type physicsEntityFinder {
    typemethod getEntityList buffer {
        set entities [lindex $buffer 6]
        set base     16
        set result [list]
        for {set i 0} {$i < $entities} {incr i} {
            lappend result $base
            incr base [lindex $buffer $base]
        }
        return $result
    }
}
#----------------------------------------------------------------------------
# stringListEntityFinder
#      Ensemble command for locating entities in string list buffers
#
snit::type stringListEntityFinder {
    typemethod getEntityList buffer {
        set entities [lindex $buffer 6]
        set base 16
        set result [list]
        for {set i 0} {$i < $entities} {incr i} {
            lappend result $base
            #  Look for a char with at least one null.
            #
            while {[lindex $buffer $base] > 0xff} {
                incr base
            }
            incr base
        }
        return $result
    }
}
#----------------------------------------------------------------------------
# nsclBuffer
#       Encapsulates the overall decoding of an NSCL buffer.
#
# options:
#    -buffer  list    - The buffer to decode.
# Members:
#                <header>
#    usedSize        Returns the number of used words in the buffer.
#    type            Returns a stringified buffer type
#    addTypeName id str
#                    Add name for a buffer type.
#    run             Fetch the run number from the buffer.
#    sequence        Fetch the sequence number fromt he buffer.
#    entities        Fetch the entity count from the buffer.
#
#                <body> (Some are only available on some buffer types)
#
#    runTime         If possible, fetch the elapsed runtime from the buffer.
#
#    absoluteTime    If possible, fetch the absolute time from the buffer.
#    title           If possible, fetch the title from the bufffer.
#    getEntity       If possible, fetch an entity from the buffer.
#    startTime       If possible fetch the interval start time from the buffer.
#    endTime         If possible, fetch the interval end time from the bufer.
#
snit::type nsclBuffer {
    option -buffer {}

    variable typeMap
    variable entityIndices

    ####
    #  configure -buffer buffer
    #     Configures the object with a new buffer
    #
    onconfigure -buffer buffer {
        set options(-buffer) $buffer
        set type [$self typeid]
        set entityIndices [list]
        switch -exact -- $type {
            1 {
                set entityIndices [physicsEntityFinder getEntityList $buffer]
            }
            2 - 3 {
                set entityIndices [scalerEntityFinder getEntityList $buffer]
            }
            4 - 5 - 6 {
                set entityIndices [stringListEntityFinder getEntityList $buffer]
            }
            default {
            }
        }
    }
    ####
    # usedSize
    #     Return the number of words in the body of the buffer.  While NSCL Buffers
    #     are fixed sizes, they are not packed tightly.
    #
    method usedSize {} {
        return [expr int([lindex $options(-buffer) 0])]
    }
    ####
    # typeid
    #   Returns the buffer type id.
    #
    method typeid {} {
        return [expr int([lindex $options(-buffer) 1])]
    }
    ####
    # type
    #    Returns the textual buffer type.  If a textual correspondence for the type
    #    has been established, it is returned.  Otherwise, the string [list Type num]
    #    is returned.
    #
    method type {} {
        set type [$self typeid]
        if {[array names typeMap $type] ne ""} {
            return $typeMap($type)
        } else {
            return [list Type [lindex $options(-buffer) 1]]
        }
    }
    #####
    # addTypeName id name
    #      Adds a correspondence between a buffer type id and a name.
    # Parameters:
    #   id    - Buffer type id.-- in decimal.
    #   name  - Name of buffer type.
    #
    method addTypeName {id name} {
        set typeMap($id) $name
    }
    ######
    #  run
    #     Returns the run number of the current buffer.
    #
    method run {} {
        return [expr int([lindex $options(-buffer) 3])]
    }
    ########
    # sequence
    #     Returns the buffer sequence number.
    #     The implicit assumption is the source is little endian.
    #
    method sequence {} {
        return [$self getLongEntity 4]
    }
    #######
    #  entities
    #       Returns the buffer entity count
    #
    method entities {} {
        return [expr int([lindex $options(-buffer) 6])]
    }
    ######
    #  runTime
    #       If the buffer has such a thing, returnthe run time.
    #
    method runTime {} {
        set type [$self typeid]
        switch -exact $type {
            11 - 12 - 13 - 14 {
                return [$self getLongEntity 56]
            }
            2 - 3 {
                return [$self getLongEntity 16]
            }
            default {
                return [list]
            }
        }
    }
    ########
    #  absoluteTime
    #         If the buffer is a control buffer the absolute time is
    #         returned from the buffer, otherwise, an empty string is returned.
    # Return Value:
    #    The return string, if nonblank is of the form YYYY-MM-DD hh:mm:ss
    #
    method absoluteTime {} {
        set type [$self typeid]
        switch -exact -- $type {
            11 - 12 - 13 - 14 {
                set year  [lindex $options(-buffer) 60]
                set month [lindex $options(-buffer) 58]
                set day   [lindex $options(-buffer) 59]
                set hour  [lindex $options(-buffer) 61]
                set min   [lindex $options(-buffer) 62]
                set sec   [lindex $options(-buffer) 63]

                return [format "%04d-%02d-%02d %02d:%02d:%02d" $year $month $day \
                                                               $hour $min $sec]
            }
            default {
                return [list]
            }
        }
    }
    #############
    #  title
    #      If the buffer is of a type that has a title, it is returned.
    #      otherwise, an empty string is returned.
    #
    method title {} {
        set type [$self typeid]
        switch -exact -- $type {
            11 - 12 - 13 - 14 {
                return [$self getStringEntity 16]
            }
            default {
                return [list]
            }
        }
    }
    #######
    #  getLongEntity index
    #      Retrieves a longword from the buffer starting at the desired offset.
    # Parameters
    #   index   - index into the buffer at which the longword starts.
    # NOTE:
    #    There's an implicit assumption the buffer source was little endian.
    #
    method getLongEntity index {
        set low   [lindex $options(-buffer) $index]
        incr index
        set high  [lindex $options(-buffer) $index]

        return [expr {$low | ($high << 16)}]
    }
    ######
    # getEvent  index
    #     Get an event that starts at index
    # Parameter:
    #   index  - offset into buffer of start of the event.
    #
    method getEvent index {
        set size [lindex $options(-buffer) $index]
        set last [expr {$index + $size - 1}]
        return [lrange $options(-buffer) $index $last]
    }
    #######
    # getStringEntity index
    #     Get a string entity from the buffer.
    # Parameters:
    #   index - offset into the buffer where string starts.
    #
    method getStringEntity index {
        set string [list]
        while 1 {
            set data [lindex $options(-buffer) $index]
            set word [binary format s1 $data]
            binary scan $word a2 chars
            append string $chars
            incr index
            if {$data < 0xff} break
        }
        return [string trim $string "\000"]
    }
    #######
    #  getEntity n
    #     Retrieves an entity from the buffer.
    #     This is done in a buffer type dependent manner.
    #
    method getEntity n {
        if {($n < 0) || ($n >= [llength $entityIndices])} {
            return [list]
        }
        set index [lindex $entityIndices $n]
        set type  [$self typeid]
        switch -exact -- $type {
            1 {
                return [$self getEvent $index]
            }
            2 - 3 {
                return [$self getLongEntity $index]
            }
            4 - 5 - 6 {
                return [$self getStringEntity $index]
            }
            default {
                return [list]
            }
        }
    }
    ########
    #  getRawEntity n
    #      Get an entity but in raw, undecoded form.
    # Parameters:
    #  n - The entity to get.
    # Returns:
    #    List of hex numbers representing the entity or [list] if there
    #    is no entity by that number.
    #
    method getRawEntity n {
        if {($n < 0) || ($n >= [llength $entityIndices])} {
            return [list]
        }
        set start [lindex $entityIndices $n]
        incr n
        set end   [lindex $entityIndices $n]
        if {$end eq ""} {
            set end [lindex $options(-buffer) 0]
            incr end -1
        }
        return [lrange $options(-buffer) $start $end]
    }
    #######
    # startTime
    #       If the buffer has one, get it's interval start time.
    #
    method startTime {} {
        set type [$self typeid]
        if {($type == 2) || ($type == 3) } {
            return [$self getLongEntity 21]
        } else {
            return [list]
        }
    }
    ######
    # endTime
    #     If the buffer has one, get its interval end time.
    #
    method endTime {} {
        set type [$self typeid]
        if {($type == 2) || ($type == 3) } {
            return [$self getLongEntity 16]
        } else {
            return [list]
        }
    }
}
