# ReadougGUIPanel.tcl --
#
# UI generated by GUI Builder Build 107673 on 2005-08-09 08:48:08 from:
#    //homedir/home2/fox/My Documents/DAQDocs/2005a/daq/DirectoryStructure/code/ReadougGUIPanel.ui
# This file is auto-generated.  Only the code within
#    '# BEGIN USER CODE'
#    '# END USER CODE'
# and code inside the callback subroutines will be round-tripped.
# The proc names 'ui' and 'init' are reserved.
#

package require Tk 8.4
package require ScalerParameterGUI
package require selectReadoutDialog
# Declare the namespace for this dialog
namespace eval ReadougGUIPanel {
    variable OutputSavedLines 500
    variable TimedRun          0
    variable RecordData        0
    variable ScalerChannels    0
    variable ScalerPeriod      2
    variable UseTestColors     0
    variable TestNonRecordingBg red
    variable TestRecordingBg    blue
    variable TestNonRecordingFg blue
    variable TestRecordingFg    red

    variable sourceDefaultPath ./

    #  Callbacks:

    variable beginCallback    {}
    variable endCallback      {}
    variable pauseCallback    {}
    variable resumeCallback   {}
    variable restartCallback  {}
    variable startCallback    {}
    variable exitCallback     {}
    variable sourceCallback   {}

    variable buttonRelief     raised
    variable buttonBorder     1
}

# Source the ui file, which must exist
source [file join [file dirname [info script]] ReadougGUIPanel_ui.tcl]

# BEGIN USER CODE
package provide ReadoutGUIPanel 1.0

#  This code ensures that pkg_mkIndex does not choke
# on this.

if {[info var ::argv0] == ""} {
    set ::argv0 ""
}
if {[info var ::argv] == ""} {
    set ::argv ""
}

#
#  Call to require that the test colors be used
#
proc ReadougGUIPanel::runInTestVersion {} {
    set ReadougGUIPanel::UseTestColors 1
}
# ReadougGUIPanel::addUserMenu ident label
#     Adds a user menu to the menubar of the
#     GUI. The caller is then responsible
#     for populating this menu with items (e.g. commands
#     separators checkboxes etc).
# Parameters:
#    ident   - Will be used as the last element of the widget
#              path for the new menu.
#    label   - The label of the menu on the menubar.
# Returns:
#    The full widget path for the new menu.
#
proc ReadougGUIPanel::addUserMenu {ident label} {
    append menuname $::ReadougGUIPanel::ROOT . $ident
    append menubar  $::ReadougGUIPanel::ROOT . menu
    menu   $menuname
    $menubar add cascade -label $label -menu $menuname
    return $menuname

}
# ReadougGUIPanel::addUserFrame ident
#    Adds a user frame to the bottom of the panel.
#    The user can place any widgets they please in this
#    frame laying it out as they desired.  The frame will
#    span the entire bottom of the gui.
#
# Parameters:
#  ident  - The last element of the wiget path of the frame
#           create.
# Returns:
#   The full path name of the created frame.
#
proc ReadougGUIPanel::addUserFrame ident {
    set top $::ReadougGUIPanel::ROOT
    append framename $top . $ident

    set geometry [grid size $top]
    set columns  [lindex $geometry 0]
    set rows     [lindex $geometry 1]
    incr rows

    frame $framename
    grid $framename -in $top -row $rows -columnspan $columns \
                    -sticky news

    return $framename
}


#
#  procs to set the callbacks for the run control buttons.
#
# ReadougGUIPanel::startStopCallbacks startcb stopcb
#     Set the start/stop callbacks.
# Parameters:
#    startcb - new callback for start of run.
#    stopcb  - new callback for stop of run.
#
proc ReadougGUIPanel::startStopCallbacks {startcb stopcb} {
    set ReadougGUIPanel::beginCallback $startcb
    set ReadougGUIPanel::endCallback   $stopcb
}
#     ReadougGUIPanel::pauseResumeCallbacks pausecb resumecb
#
proc ReadougGUIPanel::pauseResumeCallbacks {pausecb resumecb} {
    set ReadougGUIPanel::pauseCallback  $pausecb
    set ReadougGUIPanel::resumeCallback $resumecb
}
# ReadougGUIPanel::setExitCallback callback
#     Set the exit callback.
# Parameters:
#    callback  -  callback script to call on exit.
#
proc ReadougGUIPanel::setExitCallback {callback} {
    set ReadougGUIPanel::exitCallback $callback
}
# ReadougGUIPanel::setStartCallback  callback
#     Set a client callback to be invoked when the
#     user clicks the File->Start menu entry.
# Parameters:
#   callback   - The script to invoke.
#
proc ReadougGUIPanel::setStartCallback callback {
    set ReadougGUIPanel::startCallback $callback
}
# ReadougGUIPanel::setRestartCallback callback
#    Set a client callback to be invoked when the
#    user clicks the File->Restart menu entry.
# Parameters:
#    callback  - The script to invoke.
#
proc ReadougGUIPanel::setRestartCallback callback {
    set ReadougGUIPanel::restartCallback $callback
}
# ReadougGUIPanel::setSourceCallback callback
#    Sets a client callback for the source menu entry.
#
# Parameters:
#   callback  - The callback to invoke when the user
#               wants to source a file.
#
proc ReadougGUIPanel::setSourceCallback callback {
    set ReadougGUIPanel::sourceCallback $callback

}
# Lcal function to improve the contrast of a widget
# foreground when ghosted.
proc ReadougGUIPanel::ImproveEntryContrast {widget} {
    # Not all widgets/versions of Tk support this.

    catch {$widget config -disabledforeground [$widget cget -foreground]}
}


# Local function to set a new state for a widget list.
#
proc ReadougGUIPanel::SetWidgetListState {widgets state} {
    set base $::ReadougGUIPanel::ROOT
    foreach widget $widgets {
        $base.$widget configure -state $state
        ReadougGUIPanel::ImproveEntryContrast $base.$widget
    }
}

# ReadougGUIPanel::ghostWidgets
#    Ghosts the widgets that should be disabled
#    on a start run.
#
proc ReadougGUIPanel::ghostWidgets {} {
    ReadougGUIPanel::SetWidgetListState \
            {runnumber title recording
             timed days hours minutes seconds} disabled
}
# ReadougGUIPanel::ghostBegin
#    Ghost the start/stop button.
#
proc ReadougGUIPanel::ghostBegin {} {
    ::ReadougGUIPanel::SetWidgetListState startstop disabled

}
# ReadougGUIPanel::unghostBegin
#    Enable the start/stop button.
#
proc ReadougGUIPanel::unghostBegin {} {
    ::ReadougGUIPanel::SetWidgetListState startstop normal
}
# ReadougGUIPanel::unghostWidgets
#   Reactivate widgets that should be disabled on
#   a start run.
#
proc ReadougGUIPanel::unghostWidgets {} {
    ReadougGUIPanel::SetWidgetListState \
        {runnumber title recording
        timed days hours minutes seconds} normal
 }
# ReadougGUIPanel::readoutRunning
#    enables Begin/End, Restart
#    widgets now that there's a Readout Program.
#
proc ReadougGUIPanel::readoutRunning {} {
    set base $::ReadougGUIPanel::ROOT
    append begin $base . startstop
    append filemenu $base . file

    $begin configure -state normal
    $filemenu entryconfigure Restart... -state normal
    $filemenu entryconfigure Start      -state normal
}
# ReadougGUIPanel::readoutNotRunning
#    Ghosts begin/end, pause/resume,  Restart
#   widgets since there is no readout program
#   to manipulate.
proc ReadougGUIPanel::readoutNotRunning {} {
    set base $::ReadougGUIPanel::ROOT
    append begin $base . startstop
    append pause $base . pauseres
    append filemenu $base . file

    $begin configure -state disabled
    $pause configure -state disabled
    $filemenu entryconfigure Restart... -state disabled
}


# Retrieve stuff from texts:

# ReadougGUIPanel::getHost
#    Retrieve the contents of the host entry widget.
#
proc ReadougGUIPanel::getHost {} {
    set base $::ReadougGUIPanel::ROOT
    append host $base . host
    return [$host cget -text]
}

# ReadougGUIPanel::getPath
#   Get the path to the readout program in the
#   filesystem of the remote host.
#
proc ReadougGUIPanel::getPath {} {
    set base $::ReadougGUIPanel::ROOT
    append path $base . path
    return [$path cget -text]

}
#  ReadougGUIPanel::setTitle  title
#      Set the new run title.
# Parameters:
#   title   - The run title.
#
proc ReadougGUIPanel::setTitle title {
    set base $::ReadougGUIPanel::ROOT
    append w $base . title
    $w delete 0 end
    $w insert end $title
}
# ReadougGUIPanel::getRunNumber
#    Returns the current requested run number.
#
proc ::ReadougGUIPanel::getRunNumber {} {
    append widget $::ReadougGUIPanel::ROOT . runnumber
    return [$widget get]
}
#   ReadougGUIPanel::setRun     run
#       Set the run number entry.
# Parameters:
#     run   - The new run number.
#
proc ReadougGUIPanel::setRun run {
    set base $::ReadougGUIPanel::ROOT
    append w $base . runnumber


    set oldState [::ReadougGUIPanel::allowRunNumberEdit 1]


    $w configure -validate none
    $w delete 0 end
    $w insert end $run
    $w configure -validate key

    ::ReadougGUIPanel::allowRunNumberEdit $oldState
    return
}
#
#  Disable/enable user modification of the run number widget
#  
#  @param flag - true for enable/false for disable.
#
#  @return boolean - prior value.
#
proc ReadougGUIPanel::allowRunNumberEdit flag {
    set base $::ReadougGUIPanel::ROOT
    append w $base . runnumber
    set oldState  [$w cget -state]
    set newState [expr {$flag ? "normal" :  "disabled"}]
    $w configure -state $newState

    set result [expr {$oldState eq "normal" ? 1 : 0}]
    return $result


}
#  ReadougGUIPanel::incrRun
#    Increments the run number.
#
proc ReadougGUIPanel::incrRun {} {
    set run [::ReadougGUIPanel::getRunNumber]
    incr run
    ::ReadougGUIPanel::setRun $run
}
#   ReadougGUIPanel::setHost    $host
#       Set the readout host value
# Parameters
#   host - The host on which the readout should run.
#
proc ReadougGUIPanel::setHost host {
    set base $::ReadougGUIPanel::ROOT
    append w $base . host
    $w configure -text $host

#    append status $base . monitor
#    $status configure -host $host
}
#  ReadougGUIPanel::setPath    $path
#     Set the readout path for the gui.
#
proc ReadougGUIPanel::setPath path {
    set base $::ReadougGUIPanel::ROOT
    append w $base . path
    $w configure -text $path
}

# ReadougGUIPanel::recordOff
#    Turns of the record data checkbox.
#
proc ReadougGUIPanel::recordOff {} {
    set base $::ReadougGUIPanel::ROOT
    append w $base . recording
    $w deselect
    set ::ReadougGUIPanel::RecordData 0
}
# ReadouGUIPanel::recordOn
#     Turn on the recording data checkbox.
proc ReadougGUIPanel::recordOn {} {
    set base $::ReadougGUIPanel::ROOT
    append w $base . recording
    $w select
    set ::ReadougGUIPanel::RecordData 1
}
# ReadougGUIPanel::setScalers channels period
#    Sets the scaler parameters.  This allows us
#    to preload the scaler dialog with the current
#    set of defaults.
# Parameters:
#   channels    - Number of scaler channels.
#   period      - Number of seconds between readouts.
#
proc ReadougGUIPanel::setScalers {channels period} {
    set ::ReadougGUIPanel::ScalerChannels $channels
    set ::ReadougGUIPanel::ScalerPeriod   $period
}
# ReadougGUIPanel::getScalers
#    Retrieves the current scaler parameters.
#    This can be any of the following:
#    - The default initial values if no other value
#      has been set and if the scaler menu has not
#      been invoked.
#    - An external setting made by setScalers if
#      the scaler menu entry has not been invoked since
#      the last setScalers call.
#    - The most recent results of the menu.
# Returns:
#    A 2 element list consisting of {channelcount period}
#
proc ReadougGUIPanel::getScalers {} {
    return [list $::ReadougGUIPanel::ScalerChannels \
                 $::ReadougGUIPanel::ScalerPeriod]
}
##
# ReadougGUIPanel::runIsStarting
#
#   Called ot indicate that a run is starting.
#  - Begin/End button is set to text Starting
#  - Border removed from the begin/end button to make it look
#    much more like a label
#  - Relief is set to flat as well.
#  These are all restored by runIsActive
#
proc ReadougGUIPanel::runIsStarting {} {

    set root $::ReadougGUIPanel::ROOT
    append startstop $root . startstop
    set ::ReadougGUIPanel::buttonRelief [$startstop cget -relief]
    set ::ReadougGUIPanel::buttonBorder [$startstop cget -borderwidth]

    $startstop configure -relief flat -borderwidth 0 -text {Starting..}
    update idletasks
    update idletasks
    update idletasks

}
##
# Similar to the above but indicates the run is ending.
#
proc ReadougGUIPanel::runIsEnding {} {
    set root $::ReadougGUIPanel::ROOT
    append startstop $root . startstop
    set ::ReadougGUIPanel::buttonRelief [$startstop cget -relief]
    set ::ReadougGUIPanel::buttonBorder [$startstop cget -borderwidth]

    $startstop configure -relief flat -borderwidth 0 -text {Ending...}
    update  idletasks
    update  idletasks
    update  idletasks
}
#
#
# ReadougGUIPanel::runIsActive
#    Called to indicate on the GUI that the run
#    is now active. The following actions are taken:
#    The startstop button is labelled End
#    The Pause/Resume button is unghosted and labelled Pause
#    ::ReadougGUIPanel::ghostWidgets is called to disable
#    the appropriate set of widgets.
#
proc ReadougGUIPanel::runIsActive {} {
    set root $::ReadougGUIPanel::ROOT
    append startstop $root . startstop
    append pauseres  $root . pauseres

    $startstop configure -text End -relief $::ReadougGUIPanel::buttonRelief \
	-borderwidth $::ReadougGUIPanel::buttonBorder
    $pauseres  configure -text Pause -state normal 

    ::ReadougGUIPanel::ghostWidgets

}
# ReadougGUIPanel::runIsHalted
#     Called to set the widgets as they should be when
#     the run is completely inactive.
#
proc ReadougGUIPanel::runIsHalted {} {
    set root $::ReadougGUIPanel::ROOT
    append startstop $root .startstop
    append pauseres   $root .pauseres

    $startstop configure -text Begin -relief $::ReadougGUIPanel::buttonRelief \
	-borderwidth $::ReadougGUIPanel::buttonBorder
    $pauseres  configure -text Pause -state disabled

    ::ReadougGUIPanel::unghostWidgets
}
# ReadougGUIPanel::runIsPaused
#     Called to set widgets to the appropriate state
#     for a paused run.
#
proc ReadougGUIPanel::runIsPaused {} {
    append pauseres $::ReadougGUIPanel::ROOT . pauseres
    $pauseres configure -text Resume
}
# ReadougGUIPanel::isRecording
#    Sets the background of the text and status line to
#    indicate event recording is on... a nice big
#    visible indicator... the background will be
#    spartan green.
#
proc ::ReadougGUIPanel::isRecording {} {
    append text   $::ReadougGUIPanel::ROOT . output
    append status $::ReadougGUIPanel::ROOT . statusline

    if {$ReadougGUIPanel::UseTestColors} {
	set bg $ReadougGUIPanel::TestRecordingBg
	set fg $ReadougGUIPanel::TestRecordingFg
    }    else {
	set bg {dark green}
	set fg {white}
    }
    puts "setting $fg $bg"
    $text configure -background $bg  -foreground $fg
    $status configure -background $bg -foreground $fg

}
# ReadougGUIPanel::notRecording
#    Sets the background of the text and status line back to 'normal'.
#    Normal is defined as the current background of the toplevel widget
#    and the current foreground of the 'host label'.
#    All this 'cause I can't figure out how to read the foreground
#    value of labels etc. from the option database.
#
proc ::ReadougGUIPanel::notRecording {} {
    append text   $::ReadougGUIPanel::ROOT . output
    append status $::ReadougGUIPanel::ROOT . statusline

    if {$ReadougGUIPanel::UseTestColors} {
	set bg $ReadougGUIPanel::TestNonRecordingBg
	set fg $ReadougGUIPanel::TestNonRecordingFg
    }    else {
	set bg black
	set fg {dark green}
    }
    puts "setting $fg $bg"
    $text   config  -background $bg -foreground $fg
    $status config  -background $bg -foreground $fg
}

# ReadougGUIPanel::normalColors
#    Resets the text and status areas to normal coloration.
#    (run is inactive).
#
proc ::ReadougGUIPanel::normalColors {} {
    append text   $::ReadougGUIPanel::ROOT . output
    append status $::ReadougGUIPanel::ROOT . statusline
    append host   $::ReadougGUIPanel::ROOT . host
    append title  $::ReadougGUIPanel::ROOT . title



    if {$ReadougGUIPanel::UseTestColors} {
	set bgcolor [$title cget -background]
	set fgcolor $ReadougGUIPanel::TestRecordingFg
    } else {

	set bgcolor [$title cget -background]
	set fgcolor [$host              cget -foreground]
    }
    puts "setting normal"

    $text   config  -background $bgcolor -foreground $fgcolor
    $status config  -background $bgcolor -foreground $fgcolor
}

# ReadougGUIPanel::recordData
#     True if the user requested data be recorded.
#
proc ::ReadougGUIPanel::recordData {} {
    return $::ReadougGUIPanel::RecordData
}
# ReadougGUIPanel::outputText text
#   Outputs text to the output window.  The caller is responsible
#   for adding newlines at the appropriate places.
# Parameters:
#    text    - The text to add to the output window.
# NOTE:
#    In order to prevent this from being a memory leak, if the
#    number of lines of text in the text widget is bigger
#    than OutputSavedLines after the new text is appended,
#    the oldest lines are discarded so that the number of
#    lines in the text widget is about 10% of that
#
proc ReadougGUIPanel::outputText text {
    append widget $::ReadougGUIPanel::ROOT . output
    $widget configure -state normal
    $widget insert end $text
    set size [$widget index end]
    set lines [expr {int($size)}]
    if {$lines > $::ReadougGUIPanel::OutputSavedLines} {
        set linestoleave [expr {int($::ReadougGUIPanel::OutputSavedLines * 0.1)}]
	set linestokill [expr $lines - $linestoleave]
        $widget delete 0.0 $linestokill.0
    }
    $widget see end
    $widget configure -state disabled

}
# ReadougGUIPanel::setActiveTime days hours minutes seconds
#     Sets the contents of the elapsed active run time label.
#     This will contain a representation of the time for  which
#     the run has actually been taking data.
# Parameters:
#   stamp  - The time you want to appear on the time widget.
#
proc ::ReadougGUIPanel::setActiveTime {stamp} {
    append widget $::ReadougGUIPanel::ROOT . elapsed
    $widget configure -text $stamp
}
# ReadougGUIPanel::setStatusLine line
#     puts a new text string in the status line label.
# Parameters:
#    line - New contents of the status line label.
#
proc ::ReadougGUIPanel::setStatusLine line {
    append widget $::ReadougGUIPanel::ROOT . statusline
    $widget configure -text $line
}

# ReadougGUIPanel::getTitle
#    Returns the current title string.
#
proc ::ReadougGUIPanel::getTitle {} {
    append widget $::ReadougGUIPanel::ROOT . title
    return [$widget get]
}

# ReadougGUIPanel::isTimed
#    nonzero if the timed checkbox is selected indicating
#    the run should be a timed run.
#
proc ReadougGUIPanel::isTimed {} {
    return $::ReadougGUIPanel::TimedRun
}
# ReadougGUIPanel::setTimed state
#     Set state of timed run.
#
proc ReadougGUIPanel::setTimed {state} {
    set ::ReadougGUIPanel::TimedRun $state
    append widget $::ReadougGUIPanel::ROOT . timed
    if {$state} {
        .timed select
    } else {
        .timed deselect
    }
}
# ReadougGUIPanel::getRequestedRunTime
#   Find out how long the user requested the
#   run to be.  This is done by reading the
#   days, hours, minutes and seconds widgets
#   and computing what that means in seconds.
#
proc ReadougGUIPanel::getRequestedRunTime {} {
    append wdays    $::ReadougGUIPanel::ROOT . days
    append whours   $::ReadougGUIPanel::ROOT . hours
    append wminutes $::ReadougGUIPanel::ROOT . minutes
    append wseconds $::ReadougGUIPanel::ROOT . seconds

    set elapsed [$wdays get];            # Days.
    set elapsed [expr {$elapsed*24 + [$whours   get]}]; #Hours
    set elapsed [expr {$elapsed*60 + [$wminutes get]}]; # Minutes
    set elapsed [expr {$elapsed*60 + [$wseconds get]}]; # seconds.

    return $elapsed
}
# ReadougGUIPanel::setRequestedRunTime time
#       Set the requested run time in seconds.
#
proc ReadougGUIPanel::setRequestedRunTime {time} {
    append wdays    $::ReadougGUIPanel::ROOT . days
    append whours   $::ReadougGUIPanel::ROOT . hours
    append wminutes $::ReadougGUIPanel::ROOT . minutes
    append wseconds $::ReadougGUIPanel::ROOT . seconds

    set secs [expr $time % 60]
    $wseconds set $secs

    set time [expr $time/60]
    set min  [expr $time % 60]
    $wminutes set $min

    set time  [expr $time /60]
    set hours [expr $time % 60]
    $whours set $hours

    set days   [expr $time / 24]
    $wdays set $days
}
# END USER CODE

# BEGIN CALLBACK CODE
# ONLY EDIT CODE INSIDE THE PROCS.

# ReadougGUIPanel::days_command --
#
# Callback to handle days widget option -command
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::days_command args {}

# ReadougGUIPanel::days_invalidcommand --
#
# Callback to handle days widget option -invalidcommand
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::days_invalidcommand args {}

# ReadougGUIPanel::days_validatecommand --
#
# Callback to handle days widget option -validatecommand
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::days_validatecommand args {}

# ReadougGUIPanel::days_xscrollcommand --
#
# Callback to handle days widget option -xscrollcommand
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::days_xscrollcommand args {}

# ReadougGUIPanel::host_invalidcommand --
#
# Callback to handle host widget option -invalidcommand
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::host_invalidcommand args {}

# ReadougGUIPanel::host_validatecommand --
#
# Callback to handle host widget option -validatecommand
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::host_validatecommand args {}

# ReadougGUIPanel::host_xscrollcommand --
#
# Callback to handle host widget option -xscrollcommand
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::host_xscrollcommand args {}

# ReadougGUIPanel::hours_command --
#
# Callback to handle hours widget option -command
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::hours_command args {}

# ReadougGUIPanel::hours_invalidcommand --
#
# Callback to handle hours widget option -invalidcommand
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::hours_invalidcommand args {}

# ReadougGUIPanel::hours_validatecommand --
#
# Callback to handle hours widget option -validatecommand
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::hours_validatecommand args {}

# ReadougGUIPanel::hours_xscrollcommand --
#
# Callback to handle hours widget option -xscrollcommand
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::hours_xscrollcommand args {}

# ReadougGUIPanel::minutes_command --
#
# Callback to handle minutes widget option -command
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::minutes_command args {}

# ReadougGUIPanel::minutes_invalidcommand --
#
# Callback to handle minutes widget option -invalidcommand
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::minutes_invalidcommand args {}

# ReadougGUIPanel::minutes_validatecommand --
#
# Callback to handle minutes widget option -validatecommand
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::minutes_validatecommand args {}

# ReadougGUIPanel::minutes_xscrollcommand --
#
# Callback to handle minutes widget option -xscrollcommand
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::minutes_xscrollcommand args {}

# ReadougGUIPanel::output_xscrollcommand --
#
# Callback to handle output widget option -xscrollcommand
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::output_xscrollcommand args {}

# ReadougGUIPanel::path_invalidcommand --
#
# Callback to handle path widget option -invalidcommand
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::path_invalidcommand args {}

# ReadougGUIPanel::path_validatecommand --
#
# Callback to handle path widget option -validatecommand
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::path_validatecommand args {}

# ReadougGUIPanel::path_xscrollcommand --
#
# Callback to handle path widget option -xscrollcommand
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::path_xscrollcommand args {}

# ReadougGUIPanel::pauseres_command --
#
# Callback to handle pauseres widget option -command
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::pauseres_command args {
    append button $::ReadougGUIPanel::ROOT . pauseres
    if {[$button cget -text] == "Pause"} {
        if {[info commands $::ReadougGUIPanel::pauseCallback] != ""} {
            $::ReadougGUIPanel::pauseCallback
        }
    } else {
        if {[info commands $::ReadougGUIPanel::resumeCallback] != ""} {
            $::ReadougGUIPanel::resumeCallback
        }
    }
}

# ReadougGUIPanel::recording_command --
#
# Callback to handle recording widget option -command
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::recording_command args {
    set ::ReadougGUIPanel::RecordData \
        [expr $::ReadougGUIPanel::RecordData ? 0 : 1]
}

# ReadougGUIPanel::runnumber_invalidcommand --
#
# Callback to handle runnumber widget option -invalidcommand
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::runnumber_invalidcommand args {
    bell
}

# ReadougGUIPanel::runnumber_xscrollcommand --
#
# Callback to handle runnumber widget option -xscrollcommand
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::runnumber_xscrollcommand args {}

# ReadougGUIPanel::seconds_command --
#
# Callback to handle seconds widget option -command
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::seconds_command args {}

# ReadougGUIPanel::seconds_invalidcommand --
#
# Callback to handle seconds widget option -invalidcommand
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::seconds_invalidcommand args {}

# ReadougGUIPanel::seconds_validatecommand --
#
# Callback to handle seconds widget option -validatecommand
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::seconds_validatecommand args {}

# ReadougGUIPanel::seconds_xscrollcommand --
#
# Callback to handle seconds widget option -xscrollcommand
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::seconds_xscrollcommand args {}

# ReadougGUIPanel::startstop_command --
#
# Callback to handle startstop widget option -command
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::startstop_command args {
    #  The callback invoked is
    # dependent on the text in the widget.

    append button $::ReadougGUIPanel::ROOT . startstop
    if {[$button cget -text] == "Begin"} {
        if {[info commands $::ReadougGUIPanel::beginCallback] != ""} {
            $::ReadougGUIPanel::beginCallback
        }
    } else {
        if {[info commands $::ReadougGUIPanel::endCallback] != ""} {
            $::ReadougGUIPanel::endCallback
        }
    }
}

# ReadougGUIPanel::timed_command --
#
# Callback to handle timed widget option -command
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::timed_command args {
    set ::ReadougGUIPanel::TimedRun [expr {$::ReadougGUIPanel::TimedRun ? 0 : 1}]
}

# ReadougGUIPanel::title_invalidcommand --
#
# Callback to handle title widget option -invalidcommand
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::title_invalidcommand args {
    bell
}

# ReadougGUIPanel::title_validatecommand --
#
# Callback to handle title widget option -validatecommand
# Require the data in the title string be 80 characters long at most.
#
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::title_validatecommand args {
    set widget [lindex $args 0]
    set type [lindex $args 1]
    if {$type == 1} {
	set string [$widget get]
	if {[string length $string] == 80} {
	    return 0
	}
    
    }
    return 1
}

# ReadougGUIPanel::title_xscrollcommand --
#
# Callback to handle title widget option -xscrollcommand
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::title_xscrollcommand args {}

# ReadougGUIPanel::_entry_1_invalidcommand --
#
# Legacy command found in callback code. Add user comments inside body.
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::_entry_1_invalidcommand args {}

# ReadougGUIPanel::_entry_1_validatecommand --
#
# Legacy command found in callback code. Add user comments inside body.
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::_entry_1_validatecommand args {}

# ReadougGUIPanel::_entry_1_xscrollcommand --
#
# Legacy command found in callback code. Add user comments inside body.
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::_entry_1_xscrollcommand args {}

# ReadougGUIPanel::_entry_4_invalidcommand --
#
# Legacy command found in callback code. Add user comments inside body.
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::_entry_4_invalidcommand args {}

# ReadougGUIPanel::_entry_4_validatecommand --
#
# Legacy command found in callback code. Add user comments inside body.
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::_entry_4_validatecommand args {}

# ReadougGUIPanel::_entry_4_xscrollcommand --
#
# Legacy command found in callback code. Add user comments inside body.
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::_entry_4_xscrollcommand args {}

# ReadougGUIPanel::_spinbox_5_command --
#
# Legacy command found in callback code. Add user comments inside body.
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::_spinbox_5_command args {}

# ReadougGUIPanel::_spinbox_5_invalidcommand --
#
# Legacy command found in callback code. Add user comments inside body.
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::_spinbox_5_invalidcommand args {}

# ReadougGUIPanel::_spinbox_5_validatecommand --
#
# Legacy command found in callback code. Add user comments inside body.
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::_spinbox_5_validatecommand args {}

# ReadougGUIPanel::_spinbox_5_xscrollcommand --
#
# Legacy command found in callback code. Add user comments inside body.
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::_spinbox_5_xscrollcommand args {}

# ReadougGUIPanel::_text_1_xscrollcommand --
#
# Legacy command found in callback code. Add user comments inside body.
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::_text_1_xscrollcommand args {}

# ReadougGUIPanel::_text_2_xscrollcommand --
#
# Legacy command found in callback code. Add user comments inside body.
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::_text_2_xscrollcommand args {}

# ReadougGUIPanel::record_command --
#
# Legacy command found in callback code. Add user comments inside body.
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::record_command args {}

# ReadougGUIPanel::runnumber_validatecommand --
#
# Legacy command found in callback code. Add user comments inside body.
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::runnumber_validatecommand args {

    set    text   $args
    if {[regexp "(^\[0-9\]+\$)" $text]} {
	return 1
    } else {
	return 0
    }

}
# ReadougGUIPanel::scalerparameters_command --
# Invoked by a click onthe Scaler->parameters... menu.
#
# ARGS:
#    <NONE>
#
proc ReadougGUIPanel::scalerparameters_command args {
    set newinfo [ScalerParameterGUI::modal .scalerparameters \
                            $ReadougGUIPanel::ScalerChannels \
                            $ReadougGUIPanel::ScalerPeriod ]
    set ReadougGUIPanel::ScalerChannels [lindex $newinfo 0]
    set ReadougGUIPanel::ScalerPeriod   [lindex $newinfo 1]
}
# ReadougGUIPanel::filenew_command --
#    Invoked by a click on the File->new... menu.
#  Accepts potentially new values for the
#  host/filename.
#
proc ReadougGUIPanel::filenew_command args {
    set newinfo [selectReadoutDialog::createModal .readoutprompt \
                    [ReadougGUIPanel::getHost] [ReadougGUIPanel::getPath]]
    ReadougGUIPanel::setHost [lindex $newinfo 0]
    ReadougGUIPanel::setPath [lindex $newinfo 1]
}

# ReadougGUIPanel::fileexit_command
#     Called when the File->Exit is hit.
#     We confirm the user wants to exit.. If so, the
#     Client's exit callback (if established) is called.
#     and then we exit.
#
proc ReadougGUIPanel::fileexit_command args {
    if {[tk_dialog .confirmexit "Exit confirmation" \
            {Are you sure you want to exit the GUI and readout program?} \
            warning 1 Yes No] == 0} {

        if {$ReadougGUIPanel::exitCallback != ""} {
            eval $ReadougGUIPanel::exitCallback
        }
        exit
    }
}
# ReadougGUIPanel::filestart_command
#    Called by the File->Start command.
#    If the user has a callback established, we call it.
#
proc ReadougGUIPanel::filestart_command args {
    if {$ReadougGUIPanel::startCallback != ""} {
        eval $ReadougGUIPanel::startCallback
    }
}
# ReadougGUIPanel::filerestart_command
#     Called in response to the File->Restart menu entry.
#
proc ReadougGUIPanel::filerestart_command args {
    if {$ReadougGUIPanel::restartCallback != "" } {
        eval $ReadougGUIPanel::restartCallback
    }
}
# ReadougGUIPanel::filesource_command
#     Called in response to the File->Source menu entry.
#
proc ReadougGUIPanel::filesource_command args {
    set file [tk_getOpenFile -initialdir ReadougGUIPanel::sourceDefaultPath \
              -defaultextension .tcl \
              -filetypes {
                 {{TCL Scripts} {.tcl}           }
                 {{TK scripts}  {.tk}            }
                 {{All files}    *               }
               }
             ]
    # If the user handed us a file, invoke our callback to the app.

    if {($file != "") &&($ReadougGUIPanel::sourceCallback != "")} {
        $ReadougGUIPanel::sourceCallback $file
    }
}
# END CALLBACK CODE

# ReadougGUIPanel::init --
#
#   Call the optional userinit and initialize the dialog.
#   DO NOT EDIT THIS PROCEDURE.
#
# Arguments:
#   root   the root window to load this dialog into
#
# Results:
#   dialog will be created, or a background error will be thrown
#
proc ReadougGUIPanel::init {root args} {
    # Catch this in case the user didn't define it
    catch {ReadougGUIPanel::userinit}
    if {[info exists embed_args]} {
	# we are running in the plugin
	ReadougGUIPanel::ui $root
    } elseif {$::argv0 == [info script]} {
	# we are running in stand-alone mode
	wm title $root ReadougGUIPanel
	if {[catch {
	    # Create the UI
	    ReadougGUIPanel::ui  $root
	} err]} {
	    bgerror $err ; exit 1
	}
    }
    if {$root == "."} {
        set ReadougGUIPanel::ROOT {}
    }
    catch {ReadougGUIPanel::run $root}
}

