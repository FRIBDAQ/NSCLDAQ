
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
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


# (C) Copyright Michigan State University 1938, All rights reserved 
#
# ssh is a package which supports the issuance of a remote shell
# command on a system with shared filesystem.
#

#NOTE:  This version has a basic understanding of singularity containers.
#       specifically, if the envirionment variable
#       SINGULARITY_CONTAINER is defined, it is assumed to be the path
#       of a singularity container running the application.  It's also assumed
#       that
#         1.   The same container exists in the remote system.
#         2.   The user wants to run the command inside that container environment.
#       This gets done by ssh-ing a singularity exec container-name command.
#       To make things more exciting, if the ~/stagarea is defined,
#       it is read and the container runs with a bindpoint matching the link
#       target.  Additional bindpoints can be defined in the file
#       ~/.singularity_bindpoints which must consist sources which get mapped
#        to the same mount point.
#
#

package provide ssh 2.0
package require Wait
namespace eval  ssh {

	##
	# _uniquifyList 
	#   Given a list that may have dups, remove the dups.
	#   The input list could be empty in which case it returns empty:
	#
	proc _uniquifyList list {
		#  The if is probably not needed as I think [array names] will 
		#  give an empty list for a nonexistent array.
		#
		if {[llength $list] > 0} {
			foreach item $list {
				set list_array($item) ""
			}
			return [array names list_array]
		} else {
			return [list]
		}
	}

	#
	# getSingularityBindings
	#
	#   We want to allow for getting the bindings from the environment,
	#   if possible (APPTAINER_BIND and if that's not defined SINGULARITY_BIND)
	#   and then retaining ~/.singularity_bindpoints as an additional set of bindings the
	#   user might want in containers started by ssh.
	# 
	#   We also need to unquify source:targets between the various possibilities.
	#
	#   @return string
	#      This is either --bind bind1,bind2,...
	#      if there are bindings or an empty string if there are none.
	#
	proc getSingularityBindings {} {

		# If we can get bindings from APPTAINER_BIND do so:

		set bindings "";                               # so it's defined.
		if {[array names ::env APPTAINER_BIND] ne ""} {
			set bindings $::env(APPTAINER_BIND))
		} elseif {[array names ::env SINGULARITY_BIND] ne ""} {
			set bindings $::env(SINGULARITY_BIND)
		}


		#  Turn the bindings from the environment into a Tcl list:

		set bindings [split $bindings ","]

		# Add in the stagearea link as a favor to the users.
		
		set status [catch {file readlink ~/stagearea} value]
		if {!$status} {
			lappend bindings $value;                  # stagearea link auto-binds.
		}
		
		#  Add in the bindings in the user's ~/.singularity_bindpoints file as well:

		if {[file readable ~/.singularity_bindpoints]} {
			# User has additional bind points:
			
			set fd [open ~/.singularity_bindpoints r]
			while {![eof $fd]} {
				set line [string trim [gets $fd]]
				if {$line ne ""} {
					lappend bindings $line
				}
			}
			
			close $fd
		}
		# Bindings is now the full list of bindings and may have some dups
		# 

		set bindings [ssh::_uniquifyList $bindings]
	
		# If at the end of this we have any bindings we need to return a --bind
		# option else an empty string
		
		if {[llength $bindings] > 0} {
			return "--bind  [join $bindings ,]"
		} else {
			return ""
		}
	}
	##
	# getContainerImage
	#   Return the path to the container image file (full path).
	#
	#   There are a couple of possibilitites:
	#   
	#   1.  APPTAINER_CONTAINER - is defined - in that case that's the full image path.
	#   2.  SING_IMAGE          - is defined - that's the full image path.
	#   3.  None of the above -running natively.
	#
	#  @return string
	#  @retval "" if running natively.
	proc getContainerImage {} {
		if {[array names ::env APPTAINER_CONTAINER] ne ""} {
			return $::env(APPTAINER_CONTAINER)
		} elseif {[array names ::env SINGULARITY_CONTAINER] ne ""} {
			return $::env(SINGULARITY_CONTAINER)
		}
		if {[array names ::env SING_IMAGE] ne ""} {
			return $::env(SING_IMAGE)
		}
		return ""
	}
	# actualCommand
	#    If we are in a singularity container the command returned runs
	#    the input command in the container See the NOTE comments above for what
	#    that means.
	#
	# @parma command - the plain old command we're trying to run.
	#
	proc actualCommand {command} {
		set container [getContainerImage]
		if {$container eq ""} {
			# not in a container env.
			
			return $command
		}
		#
		#  We're in a container:
		
		set bindings  [ssh::getSingularityBindings]
		return "SING_IMAGE=$container singularity exec $bindings $container bash -c '$command'"
	}
	proc shellCommand { } {
		set container [getContainerImage]
		if {$container eq ""} {
		
        	# not in a container env.
			
			return bash
		}
		
		set bindings  [ssh::getSingularityBindings]
		return "SING_IMAGE=$container singularity shell --shell /bin/bash $bindings $container "
    }
    #-------------------------------------------------------------------------
    proc ssh {host command} {
	set command [ssh::actualCommand $command]
	puts "Command will be '$command'"
		set stat [catch {set output [eval exec ssh $host $command]} error]
		if {$stat != 0} {
			append output "\n"  $error
		}
		return $output
	}
    proc sshpipe {host command access} {
		set command [ssh::actualCommand $command]
		lappend command {"2>&1"}
		return [open "|ssh $host $command '2>&1'  " $access]
    }
	
	proc sshcomplex {host command access} {
        set shell [ssh::shellCommand]
	    set result  [open "| ssh -t -t -o \"StrictHostKeyChecking no\" $host $shell |& cat" a+]
	    puts $result  $command
	    flush $result
	    return $result
			 
    }
    
    #
    #   sshpid - Uses the Pipe command to open a pipe to the
    #            command.  The pipe has an input and an output end.
    #            The command runs asynchronously.
    #   Parameters:
    #       host   command
    #   Returns:
    #     list containing in order:
    #        pid    - Process ID of the ssh shell.
    #        inpipe - Pipe to read from to get output/error from process.
    #        outpipe- Pipe to write to to send data to the process.
    #
    #
	proc sshpid {host command} {
	    set pipe [ssh::sshcomplex $host $command a+]
	    set pid [lindex [pid $pipe] 0]
	    return [list $pid $pipe $pipe]
    }

    namespace export ssh sshpipe sshpid sshcomplex
}
