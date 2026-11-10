#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321



##
# @file remoteControl.tcl
# @brief Package to layer a Tcp/IP remote control server on the ReadoutGUI
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide ReadoutGuiRemoteControl 1.0
package require portAllocator
package require RunstateMachine
package require ui
package require Configuration
package require Bundles


##
# @class ReadoutGuiRemoteControl
#
# This snit::type layers a remote control server on top of the ReadoutGUI.  The
# server is largely compatible with the S800 server, however the port is
# allocated/registerd by the port allocator making it possible to do service
# location.
#
# The companion to this is the s800rctl class which serves as the backbone for
# the master readoutgui side of the remote control package. The s800Provider and
# RemoteGUI provider both are wrappers around that class. This forms the side of
# the remote control package utilized on the enslaved readout gui. 
#
# It listens for connections on a server socket and when a connection arrives,
# it accepts the first connection and then also tries to create a second
# connection. the first connection is for receiving requests from the master and
# the second connection is for send requests to the master. The communication
# between the two is implemented with symmetric transactions, meaning that any
# request made will expect the recipient to send a reply. These are 1-to-1
# connections. Also, the sockets maintained by this class are configured to be
# non-blocking. One might consider, however, that this implements a form of soft
# blocking. After a request is made, the execution will halt until the complete
# reply has been received. It is "soft" blocking rather than a true blocking
# scheme because all the while it is waiting for the response, the tcl event
# loop is active and processing events. In this way, we avoid locking up.
#
# @note as with the S800 remote control interface, only one 
#       connection is allowed at a time.
#
snit::type ReadoutGuiRemoteControl {

  option -service -default ReadoutGUIRemoteControl -readonly 1

  variable listenfd
  variable replyfd -1;             #< receives cmd from master, sends reply 
  variable requestfd -1;           #< sends comds to master, waits for reply
  variable requestReplyReceived 0; #< flags when full reply has been received
  variable requestReply "";        #< flags when full reply has been received
  variable manager  -1;            #< Port manager client instance.

  # verbs that we will process happily
  variable legalVerbs [list set begin end get init masterTransition]

  # For my status area:

  variable statusmanager ""
  variable statusbar     ""
  
  variable priorRunNumber ""
  variable priorTitle     ""
  variable host          ""

  ##
  # constructor:
  #    - No args are legal, they get processed to report them as illegal (we have no options).
  #    - Portallocator is asked for a port
  #    - Listen on that port is established dispatching to _onConnection
  # @param args - Option value pair list...better by empty.
  #
  constructor args {
    $self configurelist $args
  # Get the port.

    set manager [portAllocator %AUTO%]
    set port    [$manager allocatePort $options(-service)]

    # set up the listener:
    set listenfd [socket -server [mymethod _onConnection] $port]

  }

  ##
  # destructor
  #   - close the listen socket.
  #   - If we have a client (replyfd) ensure we're back to master mode.
  #   - close any active client (replyfd).
  #   - close the request socket 
  #
  destructor {
    close $listenfd

    if {$replyfd != -1}  {
      $self _setMaster;       # return control to the GUI.
      close $replyfd
    }

    if {$requestfd != -1} {
      close $requestFd
    }
  }

  ## @brief Send a message to the peer
  #
  # This is intended to be a symmetric communication such that a response is
  # expected.  However, we wait in a soft blocking mode until we have received
  # all of the reply from the peer. While waiting, other events will process.
  #
  # @param script - the message to send to the peer
  #
  # @returns reply from the peer
  #
  # @throws error if no request connection exists over which to communicate
  method send {script} {
  
    if {$requestfd != -1} {

      set requestReply ""

      # send the command
      
      ::ReadoutGUIPanel::Log RemoteControl debug "Sending '$script' to master"
      puts $requestfd $script

      # A response will be received from the peer and that will be handled by
      # _onRequestReadable. Once a complete message has been received, the 
      # requestReplyReceived will have been set. For that reason, we wait
      # on that variable.
      vwait [myvar requestReplyReceived]
      
      ::ReadoutGUIPanel::Log \
        RemoteControl debug "Received master reply $requestReply"
      return $requestReply
    } else {
      # there was no connection... therefore I cannot send!
      set msg "ReadoutGUIRemoteControl::send Request connection does not exist."
      return -code error $msg 
    }
  }

  ## @brief Test for connections
  #
  # Checks whether there is a valid connection at the moment.
  #
  # @returns list of boolean values, ie. {replyIsConnected requestIsConnected}
  #
  method getConnectionStatus {} {
    set reply   [expr {$replyfd != -1}]
    set request [expr {$requestfd != -1}]
    return [list $reply $request] 
  }
  ##
  # slaveMode
  #    Returns true if we are in slave mode already.
  #
  method slaveMode {} {

    set rctlPanel [::RunControlSingleton::getInstance]
    set result [$rctlPanel isSlave] 
    return $result
  }
  #-----------------------------------------------------------------------------
  # Private methods
  #
  
  ##
  # _setMaster
  #
  #   Called as we are being destroyed to set the state of the ui back
  #   to master.
  #
  method _setMaster {} {
    set rctlPanel [::RunControlSingleton::getInstance]
    set timePanel [::TimedRun::getInstance]
    if {[$self slaveMode]} {
      $rctlPanel master            
      $timePanel setSlave 0
    }
    ::ReadoutGUIPanel::Log RemoteControl output "Leaving slave mode"
  }

  # _onConnection
  #   Called when a server connection comes in:
  #   -  Close the connection if we already have a client..only one customer at
  #      a time is allowed.
  #   -  Set buffering to line.
  #   -  Set a fileevent to fire when the socket is readable.
  #   -  Save the socket in the replyfd attribute.
  #   - Try to make a second connection for making requests. We expect to find
  #   it via a service named "s800rctl". If unable to locate, this will print a
  #   message to the screen but that is it. 
  # 
  # @note This can only handle one s800rctl running on a single host. If more
  #       than one are running on a single host, then this will have problems.
  #
  # @param client     - socket fd open on client.
  # @param clientaddr - IP address of client.
  # @param clientport - Remote Port address.
  #
  method _onConnection {client clientaddr clientport} {
    if {$replyfd != -1} {
      close $client
    } else {

      # setup the request socket so that we could send messages to the master
      set allocator [portAllocator %AUTO% -hostname $clientaddr]

      # try to find the s800rctl service
      # note we could only use findServer if we know the user that is connecting
      # to us.  in 12.0 we should add to the master/slave connection protocol
      # a requirement that the master ship us its username.
      #
      #  What we are going to do is about as reliable as we can be lacking that
      #  information.
      #  1. Look for all instances of the s800rctl service in the client node.
      #  2. If there's only one instance that's the one.
      #  3. If there are multiple instances fail.
      #  4. Obviously if there are  no instances fail too:
      #
      
      ## daqdev/NSCLDAQ#1053 - if NSCLDAQ isn't running on the remote system
      #  listPorts fails - in that case just don't set up the output forwarder:
      
      if {![catch {$allocator listPorts} services]} {
        set foundCount 0
        foreach service $services {
          set port [lindex $service 0]
          set app  [lindex $service 1]
          if {$app eq "s800rctl"} {
            incr foundCount
            set svcport $port
          }
        }
        if {$foundCount == 1} {
          set requestfd [socket $clientaddr $svcport]
          chan configure $requestfd -blocking 0 -buffering line
          chan event $requestfd readable [mymethod _onRequestReadable]
        } elseif {$foundCount == 0} {
          set msg "ReadoutGUIRemoteControl::_onConnection Unable to locate "
          append msg "s800rctl service on $clientaddr"
          puts stderr $msg
          ::ReadoutGUIPanel::Log RemoteControl output $msg
        } else {
          set msg "ReadoutGUIRemoteControl::_onConnection Multiple s800rctl services"
          append msg " on $clientaddr\n"
          append msg "Can't determine which one to connect to"
          puts stderr $msg
          ::ReadoutGUIPanel::Log RemoteControl output $msg
        }
      } else {
        puts "Remote controlling system does not appear to be running NSCLDAQ"
        puts "Output forwarding disabled"
      }
      $allocator destroy

      # now setup the reply socket for receiving requests from the master 
      # This is actual connection that triggered the _onConnection callback
      chan configure $client -buffering line -blocking 0
      set replyfd $client
      chan event $replyfd readable [mymethod _onReplyReadable]
      if {$statusmanager eq ""} {
        set statusmanager [::StatusBar::getInstance]
        set statusbar     [$statusmanager addMessage] 
        $statusmanager setMessage $statusbar "Remote controlled by: $clientaddr"
      }
      ##
      #  Save the run number and title so that they can be restored when
      #   the client disconnects.
      set priorRunNumber [::ReadoutGUIPanel::getRun]
      set priorTitle     [::ReadoutGUIPanel::getTitle]
      ::ReadoutGUIPanel::Log RemoteControl output "Remote control connectionfrom $clientaddr"
      set host $clientaddr
      
    }
  }
  ##
  # _reply
  #   Send a reply to the client (or rather attempt to send a reply
  #   to the client.  The actual transfer is done inside a catch command
  #   so that we don't blow up if the wiley client has gone away between sending
  #   us their request and reading our reply.
  #
  # @param status - Status field of the command
  # @param tail   - If present this is a message put on the end of the status.
  # 
  # @note the reply is a list not just textual glomming.
  #
  method _reply {status {tail ""}} {
    
    ReadoutGUIPanel::Log RemoteControl debug "Reply to master '$status $tail'"
    set message $status
    if {$tail ne ""} {
      lappend message $tail
    }
    if {[catch {puts $replyfd $message}]} {
      $self _onClientExit
    }
  }

  ##
  # _isLegalCommand
  #   Determines if a message is a legal command.  At this point we just
  #   check legal verbs
  #
  # @param line - line of text from the client
  # @return bool - true if the command verb is legal.
  #
  method _isLegalCommand line {
    set verb [lindex $line 0]
    return [expr {$verb in $legalVerbs}]
  }
  ##
  # _executeCommand
  #    Executes a command that has been determined to have a legal verb.
  #    Each legal verb maps to a method in this class..just precede the
  #    verb name with an _ and you've got the method name...e.g.
  #    set title {some title string}
  #    executes:
  #     $self _set title {some title string}
  #
  # @param line - the line of text gotten from the client.
  #
  method _executeCommand line {
    
    set verb [lindex $line 0]
    set tail [lrange $line 1 end]


    
    $self _$verb {*}$tail;    # The executors are responsible for replying.
    ::ReadoutGUIPanel::Log RemoteControl debug "Executing '$line'"
  }

  ##
  # _onClientExit
  #    Called when an end file is seen on a client.
  #    -  Close the socket
  #    -  set the replyfd attribute to -1 so new clients canconnect
  #
  method _onClientExit {} {

    if {$replyfd != -1} {
      catch {close $replyfd}
      set replyfd -1
    }
    if {$requestfd != -1} {
      catch {close $requestfd}
      set requestfd -1
    }
    ::ReadoutGUIPanel::Log RemoteControl output "Master disconnected regaining control"
    ::ReadoutGUIPanel::setRun $priorRunNumber
    ::ReadoutGUIPanel::setTitle $priorTitle
    $statusmanager setMessage $statusbar "Remote controlled by: nobody"
    
    #  If we are in slave mode, call onEmancipation:
    

     if {[$self slaveMode]} {
      set bundles [BundleManager getInstance]
      $bundles invoke remotecontrol OnEmancipation $host
     }
    
    $self _setMaster
    
    
    
  }

  ##
  # _onCommand
  #    Called when the client has a command.
  #    - Retrieve a line of text from the socket.
  #    - ignore "" lines since those happen just before a close
  #    - check the line for legality
  #    - Execute legal commands.
  #
  method _onCommand {line} {

    #set line [gets $replyfd]

    #  empty lines can be read just before a channel goes EOF:

    if {$line eq ""} {
      return
    }
    ::ReadoutGUIPanel::Log \
      RemoteControl debug "Received request '$line' from master"
    if {[$self _isLegalCommand [lindex $line 0]]} {
      $self _executeCommand $line
    } else {
      $self _reply FAIL "Invalid command '$line'"
    }
  }


  ## @brief Handle a generic read from a socket
  #
  # This is the core read method for both the request and reply sockets. It
  # simply tries to read however much it can from both sockets.
  #
  # If no end of file existed and a failure occurred while reading from the
  # socket, a message is printed to stderr. 
  #
  # Other methods typically call this... (@see _onReplyReadable , @see
  # _onRequestReadable).
  #
  # @param fd   the channel file descriptor to read from
  #
  # @returns mixed 
  # @reval successful read = list of two elements
  #     - first element is boolean indicating if this is the end of the message
  #     - second element is the actual string read from the channel
  # @retval failure to read = ""
  #
  method _onReadable {fd} {

    # allow the specific handlers to handle how to deal with end of file
    if {($fd != -1) && ![eof $fd]} {
      # read what we can at the moment and return it
      if {[catch {gets $fd line} len]} {
        set msg "ReadoutGUIRemoteControl::_onReadable unable to read data from "
        append msg "peer"
        puts stderr $msg
      } else {
        # return a list consisting of whether it blocked and the data read
        return [list [chan blocked $fd] $line]
      }
    } 
  }

  ##
  # @brief Receive requests from the master and then replies
  #
  # The actual reading of the socket is outsourced to the _onReadable method and
  # this mainly handles the response appropriately. 
  #
  # The logic is as follows:
  #    - If EOF call _onClientExit
  #    - If not EOF call _onCommand
  #
  # @returns ""
  method _onReplyReadable {} {

  # check to see if the channel eof is reached
    if {[eof $replyfd]} {
      $self _onClientExit

      set replyfd -1
    } else {
    # read whatever we can from the channel
      set readInfo [$self _onReadable $replyfd]


      # not at end of file so we should have data 
      # index 1 has the data read from the channel
      #
      # in any case, append whatever we read to the previous reads that had been
      # read.
      append replyReply [lindex $readInfo 1]

      # index 0 contains the response of [chan blocked] 
      # if 0, then we got all that there was to receive from the channel.
      # the blocked indicator is useful because the peer is waiting for a
      # response and not sending new requests during this time.
      if {![lindex $readInfo 0]} {
        $self _onCommand $replyReply
      }
    }
  }


  ## @brief Reads replies from the master in response to requests
  #
  # Outsources the reading of the channel to _onReadable. Once all of the
  # message has been received, we flag that the reply has been received.
  #
  # @returns ""
  method _onRequestReadable {} {
    if {[eof $requestfd]} {
  
      catch {close $requestfd}
      $self _onClientExit
      set requestfd -1 
    } else {
      set readInfo [$self _onReadable $requestfd]

      append requestReply [lindex $readInfo 1]

      if {![lindex $readInfo 0]} {
        set requestReplyReceived 1
      }
    }
  }

  #-------------------------------------------------------------------------
  # Methods that execute specific command verbs.
  #

  ##
  # _set
  #    process the set command.  This can set the following things:
  #    *  slave  the slave or master mode (1 - slave, 0 - not slave)
  #    *  run    The desired run number (positive integer)
  #    *  title  The desired run title (string)
  #    *  recording - whether event recording will be done (bool)
  #    *  destination - Where event data are recorded.
  #
  # @param what  - the item to set (see above).
  # @param value - The new value for the item.
  method _set {what value} {

  ::ReadoutGUIPanel::Log RemoteControl output "_set '$what' '$value'"
  # None of these are legal if the run is not halted:

  set sm [::RunstateMachineSingleton %AUTO%]
  set state [$sm getState]
  $sm destroy

  if {$what eq "slave"} {
    if {[_notBoolean $value]} {
      $self _reply ERROR "Slave state must be a boolean was: $value"
      return
    } else {

      # Make the run control buttons display properly for slave state or not
      set rctlPanel [::RunControlSingleton::getInstance]
      set timePanel [::TimedRun::getInstance]
      set rctlIsSlave [$rctlPanel isSlave]
      set timeIsSlave [$timePanel isSlave]


      if {$value} {
        if {$state ne "Halted" } {
          $self _reply ERROR "State must be halted to perform set operations"
          return
        }

        # if not a slave, set it...
        if {!$rctlIsSlave} {
          $rctlPanel slave
          set bundles [BundleManager getInstance]
          $bundles invoke remotecontrol OnEnslaved $host
        }  
        if {!$timeIsSlave} {
          $timePanel setSlave 1
        }
        # if we are already a slave, then that is not an error.
        $self _reply OK
      } else {
        if {$rctlIsSlave} {
          $rctlPanel master
        }
        if {$timeIsSlave} {
          $timePanel setSlave 0
          set bundles [BundleManager getInstance]
          $bundles invoke remotecontrol OnEmancipation $host
        }
        $self _reply OK
      }
    }            

  } elseif {$what eq "run" } {
    if {[_notRunNumber $value ]} {
      $self _reply ERROR "Run number must be a positive integer was '$value'"
      return
    } else {
      if {![$self _requireSlaveMode]} {
        return
      }
      ::ReadoutGUIPanel::setRun $value
      $self _reply OK
    }

  } elseif {$what eq "title"} {
    if {![$self _requireSlaveMode]} {
      return
    }
    ::ReadoutGUIPanel::setTitle $value
    $self _reply OK

  } elseif {$what eq "recording"} {
    if {[_notBoolean $value]} {
      $self _reply ERROR "set recording value must be a boolean was: $value"
      return
    } else {
      if {![$self _requireSlaveMode]} {
        return
      }
      if {$value} {
        ::ReadoutGUIPanel::recordOn
      } else {
        ::ReadoutGUIPanel::recordOff
      }
      $self _reply OK
    }

  } elseif {$what eq "destination"} {
  #  Need to be able to write to the destination:

    if {![file writable $value]} {
      $self _reply ERROR "set destination - cannot write to $value"
      return
    } else {
      if {![$self _requireSlaveMode]} {
        return
      }
      ::Configuration::Set Stagearea $value
      ::Configuration::Set Experiment [file join $value experiment]
      $self _reply OK
    }
  } else {
    $self _reply ERROR "Invalid set: '$what'"
    return
  }
  }
  ##
  # _begin
  #    *  Run must be - beginnable.
  #    *  Use the state machine to start the run.  Callback bundles do all
  #       the rest.
  #
  method _begin {} {

    ::ReadoutGUIPanel::Log RemoteControl output _begin
      if {![$self _requireSlaveMode]} {
	  $self _reply ERROR "Must be in slave mode!"
      return
    }

    set sm [::RunstateMachineSingleton %AUTO%]
    set currentState [$sm getState]
    if {"Active" in [RunstateMachine listTransitions $currentState]} {

      
      # Perform the precheck and report the results back to us and the
      # master.
      
      set precheckErrors [$sm precheckTransitionForErrors "Active"]
      if {[llength $precheckErrors] > 0} {
        $self _reply "ERROR $precheckErrors"
      } else {
	  $sm masterTransition "Active"
        $self _reply OK
      }
    } else {
      $self _reply ERROR "The current state ($currentState) does not allow a begin run"
    }
    $sm destroy
  }
  ##
  # _end
  #   * Run must be endable._
  #   * Use the state machine to end the run, and let the callback bundles do
  #     everything else.
  #
  method _end {} {
    ::ReadoutGUIPanel::Log RemoteControl output _end
    flush stdout
    if {![$self _requireSlaveMode]} {
      return
    }
    set sm [::RunstateMachineSingleton %AUTO%]
    set currentState [$sm getState]

    if {"Halted" in [RunstateMachine listTransitions $currentState]} {
      $sm masterTransition "Halted"
      $self _reply OK
    } else {
      $self _reply ERROR "The current state ($currentState) does not allow the run to be ended"
    }
    $sm destroy

  }

  ##
  # _masterTransition
  #  
  #  Respond to a state transition request from the master.
  #
  # @param to   state to transition to
  #
  method _masterTransition {to} {
    
      


    set sm [::RunstateMachineSingleton %AUTO%]
    set status [catch {  
      if {![$self _requireSlaveMode]} {
        return
      }
      
      set currentState [$sm getState]
      
      # Actually check if the transition is allowed:
      
      set allowedFinals [RunstateMachine listTransitions $currentState]
      if {$to ni $allowedFinals} {
        $self _reply ERROR "The current state ($currentState) does not allow transitionsto $to"
      } else {
        ::ReadoutGUIPanel::Log \
          RemoteControl output "Attempting state transition from '$currentState' to '$to'"
        
        # If the to state is requires precheck do it and report errors to master
        
        if {$to in  [list "Active"]} {
          set precheckErrors [$sm precheckTransitionForErrors $to]
          if {[llength $precheckErrors] > 0} {
            $self _reply ERROR "Precheck of transition to $to failed: $precheckErrors"
          } else {
            set status [catch {$sm masterTransition $to} msg]
            if {$status} {
              $self _reply ERROR "Transition failed: $msg"
            } else {
              $self _reply OK
            }
          }
        } else {
        
          if {[catch {$sm masterTransition $to} msg]} {
            $self _reply ERROR "Transition of $currentState) -> $to failed:  $msg"
          } else {
            $self _reply OK
          }
        }
      }
    } msg]
    $sm destroy
    if {$status} {
      $self _reply ERROR "Slave failed transition to $to $msg"
    }
  }

  ##
  # _init
  #   * Run must be Halted. 
  #   * Let the data source manager singleton do the rest of the work
  #
  method _init {} {
    ::ReadoutGUIPanel::Log RemoteControl output "_init"
    if {![$self _requireSlaveMode]} {
      return
    }
    set sm [::RunstateMachineSingleton %AUTO%]
    set currentState [$sm getState]

    # if we are halted we can init
    if {$currentState eq "Halted"} {

      # send initall via the single data source manager
      # and reply ok
      set dataManager [DataSourcemanagerSingleton %AUTO%]
      $dataManager initall
      $dataManager destroy
      $self _reply OK
    } else {
      # otherwise, report failure
      $self _reply ERROR "init can only be called from Halted state."
    }
    $sm destroy
  }

  ##
  # _get
  #   Returns the value of one of the following:
  #   *   state - active or inactive to be compatible with s800.
  #   *   slave - boolean true if we are in slave mode.
  #
  method _get what {
    ::ReadoutGUIPanel::Log RemoteControl debug "_get '$what'"
    if {$what eq "state"} {
      flush stdout
      set sm [::RunstateMachineSingleton %AUTO%]
      set currentState [$sm getState]
      #
      #  TODO:  This is probably not a sufficient listing of state
      #         but it's all the S800readoutcallouts supports at this time.
      #
      if {$currentState eq "Active"} {
	  set state active
      } elseif {$currentState eq "Paused"} {
	  set state paused
      } else {
        set state inactive
      }
      $self _reply OK $state
      $sm destroy
    } elseif {$what eq "slave"} {
      set rctlPanel [::RunControlSingleton::getInstance]
      set current [$rctlPanel isSlave]
      $self _reply OK $current

    } else {
      $self _reply ERROR "Invalid get: $what"
    }
  }

  ##
  # _transitionTo
  #   * Run must be endable._
  #   * Use the state machine to end the run, and let the callback bundles do
  #     everything else.
  #
  method _transitionTo {state} {
    ::ReadoutGUIPanel::Log RemoteControl output "_transitionTo '$state'"
    flush stdout
    if {![$self _requireSlaveMode]} {
      return
    }
    set sm [::RunstateMachineSingleton %AUTO%]
    $sm transition $state
    $sm destroy

  }

  #---------------------------------------------------------------------------
  #  Type scoped methods (not associated with an object)


  ##
  # _notBoolean
  #   @param value - the value to check
  #   @return boolean true if value is not a valid boolean.
  # 
  proc _notBoolean value {
    return [expr {![string is boolean -strict $value]}]
  }
  ##
  # _notRunNumber
  #
  # @param value - the value to check
  # @return boolean - true if value is not a valid run number.
  #
  proc _notRunNumber value {
    if {[string is integer -strict $value]} {
      if {$value >= 0} {
        return false
      } else {
        return true
      }
    } else {
      return true
    }
  }
  ##
  # _requireSlaveMode
  #   If not in slave give an error repply
  #
  # @return bool - slave state (true if can continue).
  method _requireSlaveMode {} {

    if {![$self slaveMode]} {
      $self _reply ERROR "Must be in slave mode to do this"
      return false
    }
    return true
  }
}
##
# @class OutputMonitor
#    Provides a mechanism for a remote client to monitor what is sent to the
#    output window.
#
#
snit::type OutputMonitor {
  option -service -default ReadoutGUIOutput -readonly  1
  variable clientfds [list]

  variable listenfd
  ##
  # Constructor get a listen port for ReadoutGUIOutput
  # establish that as a server socket.
  # 
  constructor {args} {
    $self configurelist $args
    set manager [portAllocator %AUTO%]
    set port    [$manager allocatePort $options(-service)]        
    set listenfd [socket -server [mymethod _onConnect] $port]
  }
  ##
  # destructor
  #   Close all the sockets (including the listener).
  #   If we had listeners, remove the -monitorcmd callback.
  #
  destructor  {
    if {[llength $clientfds] > 0} {
      foreach client $clientfds {
        close $client
      }
      set ow [::Output::getInstance]
      $ow configure -monitorcmd [list]
    }
    close $listenfd
  }
  ##
  # _onConnect
  #
  #   A client connected.
  #   - If there are no current listeners, establish the -monitorcmd
  #   - Add the client to our client list.
  #   - Set buffering to line.
  #   - Establish a readable filevt to handle remote closes.
  #
  # @param fd     - Data transfer fd
  # @param client - IP of client
  # @param port   - Remote port of the client.
  #
  method _onConnect {fd client port} {
    if {[llength $clientfds] == 0} {
      set ow [::Output::getInstance]
      $ow configure -monitorcmd [mymethod _onGuiOutput]
    }
    lappend clientfds $fd
    fconfigure $fd -buffering line
    fileevent $fd readable [mymethod _onClientInput $fd]
  }
  ##
  # _onGuiOutput
  #    The GUI has output something to its output window.  We need to
  #    relay it to all our clients.
  #
  # @param output -the output
  #
  method _onGuiOutput output {
    foreach fd $clientfds {
      puts $fd $output
    }
  }
  ##
  # _onClientInput
  #    The client fd became readable..clients are not allowed to send us stuff
  #    therefore we assume this comes about because a client has closed the
  #    connection.  If that's not the case, the client is insane so we close
  #    anyway.
  # @param fd - The file descriptor that is readable.
  #
  method _onClientInput fd {
    close $fd
    set listIndex [lsearch -exact $clientfds $fd]
    set clientfds [lreplace $clientfds $listIndex $listIndex]
    if {[llength $clientfds] == 0} {
    # Last one, remove the -monitorcmd

      set ow [::Output::getInstance]
      $ow configure -monitorcmd [list]

    }
  }
}



## @brief Utility namespace to simplify the setup of the remote control 
#
# The basic functionality is to call  
#
# @code 
#  RemoteControlClient::initialize
# @endcode
#
# This will instantiate the ReadoutGUIRemoteControl and OUtputMonitor objects
# and then will redefine what it means to end a run to forward if possible. To
# end a run locally, the original end command has been renamed local_end. If the
# user wants to end a run locally without being forwarded, they can change their
# filter to call a tcl command to send "local_end" rather than "end".
#
# Because for any ReadoutGUI, there is only going to be one master, it is
# possible to just provide a very simple interface for setting this up that
# manages the names of the single control and output. 
#
namespace eval RemoteControlClient {

  variable control "" ;#< ReadoutGUIRemoteControl instance
  variable output ""  ;#< OutputMonitor instance
  variable initialized 0; #< ensure that we are initialized

  proc initialize {{service ReadoutGUIRemoteControl} {monitor ReadoutGUIOutput}} {
    variable control
    variable output
    variable initialized

    if {$initialized} {
      set msg "RemoteControlClient::initialize can be called only once!"
      return -code error $msg
    } else {
      set initialized 1
    }
    
    set control [ReadoutGuiRemoteControl %AUTO% -service $service]
    set output  [OutputMonitor %AUTO% -service $monitor]

    # move the default version of end to a new name called "local_end"
    # if the end has not been defined yet, then that means our new definition
    # of end will simply be overridden by the real end! This is bad and results
    # in an error.
    if {[catch {rename ::end ::local_end} msg]} {
      set msg "Unable to override the \"end\" command. Forwarding of and "
      append msg "\"end\" command will fail!"
      return -code error $msg
    }


    # define a new proc called end that will handle whether to forward or not
    proc ::end {} {
      ::ReadoutGUIPanel::Log Master debug "Substitute end"
    # get the connection status
      set connectionStatus [$::RemoteControlClient::control getConnectionStatus]

      if {[lindex $connectionStatus 1]} {
      # connection to send requests on is good... go ahead and forward
        ::ReadoutGUIPanel::Log Master output "Asking client to end"
        $::RemoteControlClient::control send "end"
      } else {
      # not connected so don't try to forward... just end as normal
        local_end
      }

    } ;# end end


    ## Define a new proc to send state transitions
    proc ::masterTransition to {
      ::RedaoutGUIPanel::Log Master output "masterTransition  '$to'"
      set stateMachine [::RunstateMachineSingleton %AUTO%]
      $stateMachine masterTransition $to
      $stateMachine destroy
    }

    set stateMachine [RunstateMachineSingleton %AUTO]
    $stateMachine setSlave 1
    $stateMachine destroy

  }  ;# end setup

} ;# end namespace
