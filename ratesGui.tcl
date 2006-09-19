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
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#

#  This file contains code to implement the rates GUI.
#

# As far as package requires go, we assume the SpecTcl GUI is up
# and it has imported browser and snit.
# We need this to be true in order to hook ourselves into its gui in any event.

#----------------------------------------------------------------------------
#  editRatesDialog
#
#   This is a snidget to edit the rates list.
#
#   Layout of the dialog is as follows:
#
#   +-----------------------------------------------------+
#   |  Spectrum browser                 Scrolling-listbox |
#   |                                                     | 
#   +-----------------------------------------------------+
#   |   [ OK ]   [ CANCEL ]   [ HELP ]                    |
#   +-----------------------------------------------------+
#
#  Double clicking a spectrum in the browser moves it into the rates listbox
#  and filters it out of the browser.
#  Double clicking a rate in the listbox moves it out of the listbox
#  and back to the spectrum browser.
#
#  OPTIONS
#    -rates          - Gets/set the contents of the rates list.
#    -okcommand      - Sets a script to fire on the ok click
#    -cancelcommand  - Sets a script to fire on the cancel clik.
#    -helpcommand    - Sets a script to fire on the help command.
# METHODS (Widget commands)
#
#   update           - Updates browser based on the contents of the list box
#                      and current spectrum definitions (usually not needed).
#   modal            - Makes the dialog modal.
#                      This will block in the event loop until either
#                      OK or CANCEL are clicked, or until the dialog is destroyed
#                      It will be up to the invoker to determine the differences
#                      between these cases.  Destroy will invoke the cancel script
#                      if defined.
#
snit::widget editRatesDialog {
    hulltype toplevel
    option   -rates         {}
    option   -okcommand     {}
    option   -cancelcommand {}
    option   -helpcommand   {}

    variable hiddenWindow   {}

    constructor args {

	label   $win.speclabel   -text "Spectra:"
	label   $win.rateslabel  -text "Rates list:"
	
	listbox   $win.rates     -yscrollcommand [list $win.ratescroll set]
	scrollbar $win.ratescroll -orient vertical                       \
	    -command [list $win.rates yview]                             
	browser $win.browser     -spectrumscript [mymethod addRate]      \
	    -restrict spectra -detail 0             \
	    -showcolumns type                       \
	    -filterspectra [mymethod filter]        \
	    -width 4in


	frame $win.command -relief groove -borderwidth 4
	button $win.command.ok     -text Ok        -command [mymethod onOk]
	button $win.command.cancel -text Cancel    -command [mymethod onCancel]
	button $win.command.help   -text Help      -command [mymethod onHelp]
	    
	# Layout the widgets,

	grid $win.speclabel          $win.rateslabel       x
	grid $win.browser            $win.rates            $win.ratescroll -sticky news
	grid $win.command -row 2 -column 0 -columnspan 3 -sticky news
	grid $win.command.ok                 $win.command.cancel
	grid columnconfigure $win.command 0 -weight 1
	grid columnconfigure $win.command 1 -weight 1
	grid columnconfigure $win.command 3 -weight 3
	grid $win.command.help -column 3 -row 0 -sticky e


	bind $win <Destroy> [mymethod onDestroy %W]
	bind $win.rates <Double-1> [mymethod removeRate %x %y]

	
	# Process  the options ...
	
	$self configurelist $args
    }

    # Configuration processing:

    #####
    #   Called when the -rates configuration option is supplied.
    #   The list box is set to contain the rates and the browser is updated
    #   to ensure that the rates are not listed.
    #
    onconfigure -rates rateList {
	$win.rates delete 0 end
	foreach rate $rateList {
	    $win.rates insert end $rate
	}
	$self update
    }
    ####
    #   Called when the -rates configuration option is fetched:

    oncget -rates {
	return [$win.rates get 0 end]
    }


    #####
    #    Filter the browser so that items that are in the rates list
    #    will not be available in the browser.

    method filter item {
	set name [lindex $item 1]
	set rates [$self cget -rates]
	if {[lsearch -exact $rates $name] == -1} {
	    return 1
	} else {
	    return 0
	}

    }
    ####
    #   Called for a double click in the browser.
    #   adds the spectrum in the path to the rates list.
    #   also updates the browser.
    #

    method addRate path {
	set pathList [split $path .]
	set pathList [lrange $pathList 1 end]
	set name     [join $pathList .]

	$win.rates insert end $name
	$self update
    }
    ####
    #  Called for a double click on the rates listbox.
    #  The element clicked on  is removed from the list box.
    #  update is called to allow the browser to show it in its list.
    #
    method removeRate {x y} {
	set item [$win.rates index @$x,$y]
	$win.rates delete $item
	$self update
    }
    ####
    #  Called when the Ok button is clicked.
    #  If it is defined, the okcommand is invoked.
    #  If the hidden window is defined, it is destroyed in order to break out of
    #  modality:
    #
    method onOk {} {
	if {$options(-okcommand) ne ""} {
	    eval $options(-okcommand)
	}
	if {$hiddenWindow ne ""} {
	    destroy $hiddenWindow
	}
    }
    ######
    # Called when the Cancel button is invoked. 
    # If defined, the cancelcommand is invoked.
    # If the hidden window is defined, it is destroyed in order to break out of
    # modality.
    #
    method onCancel {} {
	if {$options(-cancelcommand) ne ""} {
	    eval $options(-cancelcommand)
	}
	if {$hiddenWindow ne ""} {
	    destroy $hiddenWindow
	}
    }
    ####
    #  Called when the help button is clicked.  If the helpcommand is
    #  defined it is invoked.  If not a default help dialog is displayed.
    #
    method onHelp {} {
	if {$options(-helpcommand) ne ""} {
	    eval $options(-helpcommand)
	} else {
	    tk_messageBox -icon info -type ok -title {rates editor help} \
		-message {Double click on the spectrum tree view to add a spectrum to the rates list, double click in the rates list to remove one}

	}
    }
    ####
    # Called when the widget is being destroyed.  If $win is the widget,
    # all is lost and we invoke onCancel.
    #
    method onDestroy widget {
	if {$widget eq $win} {
	    $self onCancel
	}
    }
    ####
    #  Called to make the dialog application modal.
    # A hidden window is made, and then a grab is done to make the window
    # modal. We then wait for the hidden window to be destroyed.
    # Note that we also make every effort to ensure that the window is visible.
    method modal  {} {
	set hiddenWindow [frame $win.hidden]
	focus $win
	wm deiconify $win
	grab $win
	tkwait window $hiddenWindow
	grab release $win
	set hiddenWindow "";		# no longer modal
    }
    ####
    #  Update the browser can be called when new spectra are created or
    #  when something requires the browser to refilter (e.g. a change in the
    #  rates window.
    #
    method update {} {
	$win.browser update
    }
}

#-----------------------------------------------------------------------
#
#  addSpectrumToRates  name
#     Add a spectrum to the rates list.
#     An error dialog will be popped up if the name is already in the list.
#
proc addSpectrumToRates name {
    set existingRates [rate list]
    foreach item $existingRates {
	if {[lindex $item 0] eq $name} {
	    tk_messageBox -icon error -title {Duplicate rate} -type ok \
		-message "Spectrum $name is already in the rates display"
	    return
	}
    }
    # Not a duplicate so add.

    rate create $name
    tk_messageBox -icon info -title {Rate added} -type ok  \
	-message "Spectrum $name added to the rates list"
}

#---------------------------------------------------------------------
#  saveRates - prompt for a file in which to save the rates 
#              definitions.  The rates are saved as a tcl script
#              that can be sourced in to load those rate definitions.
#
proc saveRates {} {
    set file [tk_getSaveFile                          \
		  -defaultextension .tcl              \
		  -title { Save to which file?}       \
		  -filetypes {                        \
				{"Tcl scripts" .tcl } \
		                {"Rate Definitions" .rdef } \
				{"All files" * }}]

    if {$file ne ""} {
	set f [open $file w]
	set rateList [rate list]
	foreach item $rateList {
	    puts $f "rate create [lindex $item 0]"
	}
	close $f
    }
}
#-----------------------------------------------------------------------
# readRates - Empty the current rate list and restore an existing
#             set of rates.   The rates list is emptied in order to
#             ensure there will be no dups.
#
proc readRates {} {
    set answer [tk_messageBox -icon question -type okcancel -title {Replace rates?} \
		    -message {Reading a rates file will overwrite the existing definitions click ok to continue}]
    if {$answer eq "ok"} {
	set file [tk_getOpenFile                    \
		  -defaultextension .tcl              \
		  -title {Restore which file?}       \
		  -filetypes {                        \
				{"Tcl scripts" .tcl } \
		                {"Rate Definitions" .rdef } \
				{"All files" * }}]
	
	set ratesList [rate list]
	foreach item $ratesList {
	    rate delete [lindex $item 0]
	}
	source $file
    }

}


#  Hook some stuff into the SpecTcl gui


#  Add 'Add To Rates' to spectrum context menu (after separator).

if {![winfo exists .spectrumcontextmenu]} {
    createSpectrumContextMenu
}
.spectrumcontextmenu add separator
.spectrumcontextmenu add command -label {Add To Rates}

#  We need to rename spectrumContextMenu so that we can
#  intervene to set up our own command configuration.

rename spectrumContextMenu standard_spectrumContextMenu
proc spectrumContextMenu {path x y} {
    

    set name [pathToName $path]
    .spectrumcontextmenu entryconfigure 10 \
	-command [list addSpectrumToRates $name]

    standard_spectrumContextMenu $path $x $y
}

# Hook our Items into the main menu too:

# We can just append to the spectrum menu.

.topmenu.spectra add separator
.topmenu.spectra add command -label {Edit Rates List...} -command editRates

#  We're going to assume there's a separator and 'exit' at the end
#  of the file menu and insert our save/restore before exit:

set position [.topmenu.filemenu index end]
incr position -1

.topmenu.filemenu insert [incr position] command -label {Save Rates List...}    \
                                     -command saveRates
.topmenu.filemenu insert [incr position] command -label {Restore Rates List...} \
                                     -command readRates
.topmenu.filemenu insert [incr position]  separator