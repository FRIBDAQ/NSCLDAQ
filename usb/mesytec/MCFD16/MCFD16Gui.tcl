#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#    Jeromy Tompkins
#	   NSCL
#	   Michigan State University
#	   East Lansing, MI 48824-1321



package provide mcfd16gui 1.0

package require mcfd16usb
package require snit
package require Tk
#package require FrameSwitcher
package require FrameManager
package require scriptheadergenerator
package require BlockCompleter
package require mcfd16channelnames


## "Base" type for MCFD16View ################
#
# The MCFD16CommonView and MCFD16IndividualView both delegate some methods to
# this snit::type. It holds the variables that their widgets target as
# -textvariables. The only thing they do not manage is the name values. 
#
snit::type MCFD16View {

  option -presenter -default {} ;# logical controller of device

  variable _mcfd ;# basic array of values

  ## @brief Construct and initialize options
  #
  constructor {args} {
    $self configurelist $args

    $self InitArray 
  }


  ## @brief Retrieve the fully qualified name of the array
  #
  # @returns fully qualified name of _mcfd array for use outside of the
  # snit::type
  method mcfd {} {
    return [$self info vars _mcfd]
  }

  ## @brief Check whether a channel name contains non-whitespace characters
  # 
  # This is called when a channel entry loses focus
  #
  # @param name   candidate string 
  #
  # @return boolean
  # @retval 0 - string was empty or all whitespace
  # @retval 1 - otherwise
  method ValidateName {name} {
    set name [string trim $name]
    set good [expr [string length $name]!=0]

    return $good
  }

  ## @brief Reset channel to a simple string
  #
  # Typically called with ValidateName returns false. It will set the string
  # to "Ch#".
  #
  # @returns ""
  method ResetChannelName {widget} {
    set str [$widget cget -textvariable]
    regexp {^.*(\d+)$} $widget match ch
    set $str "Ch$ch"
  }

  ## @brief Initialize all elements of the array for each setting
  #
  method InitArray {} { 

    # set the channel valus
    for {set i 0} {$i < 16} {incr i} {

      set _mcfd(th$i) 127
      set _mcfd(mo$i) 4 

      if {($i%2)==0} {
        set pair [expr $i/2]
        set _mcfd(po$pair) neg
        set _mcfd(ga$pair) 1
        set _mcfd(wi$pair) 100 
        set _mcfd(dt$pair) 100 
        set _mcfd(dl$pair) 1
        set _mcfd(fr$pair) 20
      }
    }

    # set the common channels
    set _mcfd(po8) neg
    set _mcfd(ga8) 1
    set _mcfd(wi8) 50
    set _mcfd(dt8) 50
    set _mcfd(dl8) 1
    set _mcfd(fr8) 20
    set _mcfd(th16) 127
  }

  # ------ Setters and getters
  
  ## @brief Retrieve the threshold value for a certain channel
  # 
  # There is no range checking for the channel and it is the caller's
  # responsibility to check that ch is in the range [0,16].
  #
  # @param ch   the channel index
  #
  # @returns value of threshold
  method GetThreshold {ch} {  return $_mcfd(th$ch) }

  ## @brief Set the threshold value for a certain channel
  # 
  # There is no range checking for the channel and it is the caller's
  # responsibility to check that ch is in the range [0,16].
  #
  # @param ch   the channel index
  # @param val  value to set
  method SetThreshold {ch val} { set _mcfd(th$ch) $val }

  ## @brief Retrieve the polarity value for a certain channel
  # 
  # There is no range checking for the channel and it is the caller's
  # responsibility to check that ch is in the range [0,8], where ch=8
  # corresponds to the common value.
  #
  # @param ch   the channel index
  #
  # @returns value of threshold
  method GetPolarity {ch} {  return $_mcfd(po$ch) }

  ## @brief Set the polarity value for a certain channel
  # 
  # There is no range checking for the channel and it is the caller's
  # responsibility to check that ch is in the range [0,8].
  #
  # @param ch   the channel index
  # @param pol  value to set
  method SetPolarity {ch pol} {  set _mcfd(po$ch) $pol }

  ## @brief Get the gain value for a certain channel
  # 
  # There is no range checking for the channel and it is the caller's
  # responsibility to check that ch is in the range [0,8].
  #
  # @param ch   the channel index
  method GetGain {ch} {  return $_mcfd(ga$ch) }

  ## @brief Set the gain value for a certain channel
  # 
  # There is no range checking for the channel and it is the caller's
  # responsibility to check that ch is in the range [0,8].
  #
  # @param ch   the channel index
  # @param gain  value to set
  method SetGain {ch gain} {  set _mcfd(ga$ch) $gain }

  ## @brief Get the width value for a certain channel
  # 
  # There is no range checking for the channel and it is the caller's
  # responsibility to check that ch is in the range [0,8].
  #
  # @param ch   the channel index
  method GetWidth {ch} {  return $_mcfd(wi$ch) }

  ## @brief Set the width value for a certain channel
  # 
  # There is no range checking for the channel and it is the caller's
  # responsibility to check that ch is in the range [0,8]. there is also no
  # check for the value passed in as an argument.
  #
  # @param ch     the channel index
  # @param width  value to set
  method SetWidth {ch width} {  set _mcfd(wi$ch) $width }

  ## @brief Get the deadtime value for a certain channel
  # 
  # There is no range checking for the channel and it is the caller's
  # responsibility to check that ch is in the range [0,8].
  #
  # @param ch   the channel index
  method GetDeadtime {ch} {  return $_mcfd(dt$ch) }

  ## @brief Set the deadtime value for a certain channel
  # 
  # There is no range checking for the channel and it is the caller's
  # responsibility to check that ch is in the range [0,8].
  #
  # @param ch   the channel index
  # @param time value to set
  method SetDeadtime {ch time} {  set _mcfd(dt$ch) $time }

  ## @brief Get the delay value for a certain channel
  # 
  # There is no range checking for the channel and it is the caller's
  # responsibility to check that ch is in the range [0,8].
  #
  # @param ch   the channel index
  method GetDelay {ch} {  return $_mcfd(dl$ch) }

  ## @brief Set the delay value for a certain channel
  # 
  # There is no range checking for the channel and it is the caller's
  # responsibility to check that ch is in the range [0,8].
  #
  # @param ch   the channel index
  # @param time value to set
  method SetDelay {ch time} {  set _mcfd(dl$ch) $time }

  ## @brief Retrieve the fraction value for a certain channel
  # 
  # There is no range checking for the channel and it is the caller's
  # responsibility to check that ch is in the range [0,8].
  #
  # @param ch   the channel index
  method GetFraction {ch} {  return $_mcfd(fr$ch) }

  ## @brief Set the fraction value for a certain channel
  # 
  # There is no range checking for the channel and it is the caller's
  # responsibility to check that ch is in the range [0,8].
  #
  # @param ch       the channel index
  # @param fraction value to set
  method SetFraction {ch frac} {  set _mcfd(fr$ch) $frac }


  ##  All MCFD16View-like types simple pass events to their presenter
  #
  method Commit {} {
    $options(-presenter) Commit
  }

}; # end of the View


###############################################################################
#
# MCFD16IndividualView 
#
#  A widget for controlling the MCFD16 channels individually. This creates a 
#  16-row table with a header. There is a row for each of the channels in the
#  device and also they are grouped into pairs to make the configuration simpler
#  to understand. This provides no mechanism for committing the values and is 
#  actually just a widget to display the various controls. It provides controls
#  over: threshold, gain, fraction,  delay, width, polarity, and deadtime. It
#  also displays a name column that contains user-definable labels for the
#  individual channels.
#
# @important The MCFD16IndividualPresenter is the only snit::type designed to
# work with this.
snit::widget MCFD16IndividualView {

  component m_base ;# the MCFD16View instance

  delegate method * to m_base
  delegate option * to m_base

  ## Constructs the gui and parses options
  #
  # @param args   option-value pairs (see MCFD16View for supported options)
  constructor {args} {
    install m_base using MCFD16View %AUTO%

    $self configurelist $args 

    $self BuildGUI

  }

  ## @brief Assemble the widgets
  #
  method BuildGUI {} {

    $self BuildHeader $win.header
    $self BuildTable $win.table

    grid $win.header -sticky nsew -padx 2 -pady 2
    grid $win.table -sticky nsew -padx 2 -pady 2

    # allow the columns to resize
    grid columnconfigure $win {0} -weight 1
    grid rowconfigure $win {0} -weight 1
  }


  ## Construct the header row
  #
  # @param name   the name of the frame that will hold the row
  method BuildHeader {name} {

    set w $name
    ttk::frame $w -style "Header.TFrame"

    ttk::label $w.na -text Name -style "Header.TLabel" -padding "3 3 3 0" \
                                -width 8     
    ttk::label $w.ch -text Channel  -style "Header.TLabel" -padding "3 3 3 0" 


    ttk::label $w.po -text Polarity  -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.ga -text Gain  -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.th -text Threshold  -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.wi -text Width -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.dt -text Deadtime  -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.dl -text Delayline -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.fr -text Fraction -style "Header.TLabel" -padding "3 3 3 0"

    grid $w.na $w.th $w.po $w.ga $w.wi $w.dt $w.dl $w.fr \
      -sticky sew -padx 4 -pady 4
    grid columnconfigure $w {1 2 3 4 5 6 7} -weight 1 -uniform a
    grid columnconfigure $w {0} -weight 2 -uniform a
    grid rowconfigure $w {0} -weight 1
  }

  ## Construct the 16 rows of the table
  #
  # @param name   the name of the frame that will hold the row
  #
  method BuildTable {name } {
    set w $name
    ttk::frame $w 

    set rows [list]
    for {set ch 0} {$ch < 16} {incr ch 2} { 
      set style Grouped
      set name "$w.rows[expr $ch/2]"
      $self BuildGroupedRows $name $ch $style
      lappend rows $name
    }

    foreach row $rows {
      grid $row -sticky nsew -padx 2 -pady 2
    }
    grid columnconfigure $w {0} -weight 1
    grid rowconfigure $w {0 1 2 3 } -weight 1
  }

  ## Constructs a pair of rows in the table
  #
  # @param name   name of the frame that will be filled 
  #
  method BuildGroupedRows {name ch style} {
    set w $name
    ttk::frame $w -style $style.TFrame

    # construct first row
    ttk::entry $w.na$ch -width 8 -textvariable MCFD16ChannelNames::chan$ch \
                        -style "$style.TEntry" \
                                -validate focus -validatecommand [mymethod ValidateName %P] \
                                -invalidcommand [mymethod ResetChannelName %W]

    ttk::spinbox $w.th$ch -textvariable "[$self mcfd](th$ch)" -width 4 \
                        -style "$style.TSpinbox" -from 0 -to 255 \
                        -state readonly
    set pair [expr $ch/2]
    ttk::radiobutton $w.poneg$pair -variable "[$self mcfd](po$pair)" \
                                   -value neg -text neg \
                                   -style "$style.TRadiobutton"

    ttk::spinbox $w.ga$pair -textvariable "[$self mcfd](ga$pair)" \
      -style "$style.TSpinbox" -width 4 \
      -values {0 1 2} -state readonly

    ttk::spinbox $w.wi$pair -textvariable "[$self mcfd](wi$pair)" \
      -style "$style.TSpinbox" -width 4 \
      -from 16 -to 222 -state readonly

    ttk::spinbox $w.dt$pair -textvariable "[$self mcfd](dt$pair)" \
      -style "$style.TSpinbox" -width 4 \
      -from 27 -to 222 -state readonly

    ttk::spinbox $w.dl$pair -textvariable "[$self mcfd](dl$pair)" \
      -style "$style.TSpinbox" -width 4 \
      -from 0 -to 4 -state readonly

    ttk::radiobutton $w.fr${pair}20 -text "20%" -variable "[$self mcfd](fr$pair)" \
                                    -value 20  -style "$style.TRadiobutton"

    grid $w.na$ch $w.th$ch $w.poneg$pair $w.ga$pair $w.wi$pair \
      $w.dt$pair $w.dl$pair $w.fr${pair}20 \
      -sticky news -padx 4 -pady 4
    grid configure $w.ga$pair $w.wi$pair $w.dt$pair \
                    $w.dl$pair -rowspan 2

    # construct second row 
    incr ch
    ttk::entry $w.na$ch -width 8 -textvariable MCFD16ChannelNames::chan$ch \
      -style "$style.TEntry" \
      -validate focus -validatecommand [mymethod ValidateName %P] \
                      -invalidcommand [mymethod ResetChannelName %W]
    ttk::spinbox $w.th$ch -textvariable "[$self mcfd](th$ch)" -width 4 \
      -style "$style.TSpinbox" -from 0 -to 255 \
      -state readonly

    ttk::radiobutton $w.popos$pair -variable "[$self mcfd](po$pair)" \
                                   -value pos -text pos \
                                   -style "$style.TRadiobutton"
    ttk::radiobutton $w.fr${pair}40 -text "40%" -variable "[$self mcfd](fr$pair)" \
                                    -value 40  -style "$style.TRadiobutton"
    grid $w.na$ch $w.th$ch $w.popos$pair x x x x $w.fr${pair}40 -sticky news -padx 4 -pady 4

    # allow the columns to resize
    grid columnconfigure $w {1 2 3 4 5 6 7} -weight 1 -uniform a
    grid columnconfigure $w {0} -weight 2 -uniform a
    grid rowconfigure $w {0 1} -weight 1
  }
}

###############################################################################
#
# MCFD16CommonView
#
#  A widget for controlling common channel values in MCFD16. This is just one 
#  row of controls that will set the parameters for common mode. It provides
#  control over the same settings as are displyed in the MCFD16IndividualView
#  for those common settings. One stark difference between this and the
#  individual view is that there is no way to configure the names. Instead, the
#  name is fixed as "Common". LIke the individual view, there is no real
#  functionality built into this snit::widget other than displaying some widgets
#  and then providing a means for accessing their values.
#
# @important The MCFD16CommonPresenter is intended for use with this
# snit::widget.
snit::widget MCFD16CommonView {

  component m_base ;# the MCFD16View

  delegate method * to m_base
  delegate option * to m_base

  ## Construct the megawidget and process options
  #
  # @param args   option-value pairs (see MCFD16View for valid options)
  constructor {args} {
    install m_base using MCFD16View %AUTO%

    $self configurelist $args 

    $self BuildGUI
  }

  ## Construct the header and the row of controls
  method BuildGUI {} {

    $self BuildHeader $win.header
    $self BuildCommonControls $win.common


    grid $win.header -sticky nsew -padx 2 -pady 2
    grid $win.common -sticky nsew -padx 2 -pady 2

    # allow the columns to resize
    grid columnconfigure $win {0} -weight 1
    grid rowconfigure $win {0} -weight 1
  }

  ## Build the header
  # 
  # @param name    the name of container frame
  #
  method BuildHeader {name} {

    set w $name
    ttk::frame $w -style "Header.TFrame"

    ttk::label $w.na -text Name -style "Header.TLabel" -padding "3 3 3 0" \
                                -width 8     
    ttk::label $w.ch -text Channel  -style "Header.TLabel" -padding "3 3 3 0" 


    ttk::label $w.po -text Polarity  -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.ga -text Gain  -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.th -text Threshold  -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.wi -text Width -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.dt -text Deadtime  -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.dl -text Delayline -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.fr -text Fraction -style "Header.TLabel" -padding "3 3 3 0"

    grid $w.na $w.th $w.po $w.ga $w.wi $w.dt $w.dl $w.fr \
      -sticky sew -padx 4 -pady 4
    grid columnconfigure $w {1 2 3 4 5 6 7} -weight 1 -uniform a
    grid columnconfigure $w {0} -weight 2 -uniform a
    grid rowconfigure $w {0} -weight 1
  }

  ## Build the controls
  # 
  # @param name    the name of container frame
  #
  method BuildCommonControls {name} {
    set w $name

    ttk::frame $w -style "Grouped.TFrame"

    ttk::label $w.name -text Common  -style "Grouped.TLabel"

    ttk::radiobutton $w.poneg8 -text "neg" -variable [$self mcfd](po8) \
                               -value neg -style "Grouped.TRadiobutton"
    ttk::radiobutton $w.popos8 -text "pos" -variable [$self mcfd](po8) \
                               -value pos -style "Grouped.TRadiobutton"
    ttk::spinbox $w.ga8 -textvariable [$self mcfd](ga8) -width 4 \
                               -values {0 1 2} -style "Grouped.TSpinbox" \
                               -state readonly
    ttk::spinbox $w.wi8 -textvariable [$self mcfd](wi8) -width 4 \
                               -from 16 -to 222 -style "Grouped.TSpinbox" \
                               -state readonly
    ttk::spinbox $w.dt8 -textvariable [$self mcfd](dt8) -width 4 -from 27 \
                               -to 222 -style "Grouped.TSpinbox" -state readonly
    ttk::spinbox $w.dl8 -textvariable [$self mcfd](dl8) -width 4 -from 0 \
                               -to 4 -style "Grouped.TSpinbox" -state readonly
    ttk::radiobutton $w.fr820 -text "20%" -variable [$self mcfd](fr8) \
                              -value 20 -style "Grouped.TRadiobutton"
    ttk::radiobutton $w.fr840 -text "40%" -variable [$self mcfd](fr8) -value 40\
                              -style "Grouped.TRadiobutton"
    ttk::spinbox $w.th16 -textvariable [$self mcfd](th16) -width 4 -from 0 \
                              -to 255 -style "Grouped.TSpinbox" -state readonly

    grid $w.name $w.th16 $w.poneg8 $w.ga8 $w.wi8 $w.dt8 $w.dl8 $w.fr820 -sticky news -padx 4 -pady 4
    grid ^       ^       $w.popos8 ^      ^      ^      ^      $w.fr840 -sticky news -padx 4 -pady 4
    grid columnconfigure $w {1 2 3 4 5 6 7} -weight 1 -uniform a
    grid columnconfigure $w {0} -weight 2 -uniform a
  }
}


###############################################################################
###############################################################################
###############################################################################

# PRESENTERS For the views


## Common functionality for MCFD16CommonPresenter and MCFD16IndividualPresenter
#
# This snit::type provides some convenience methods that are mutually useful to
# the more specialized presenters. Both of those specialized presenters delegate
# to this snit::type.
#
snit::type MCFD16Presenter {

  option -widgetname -default "" ;# name of the view
  option -handle -default ""     ;# device handle (a low-level driver)
  
  option -view -default ""       ;# view instance (owned by this)

  ## Parse options
  #
  constructor {args} {
    $self configurelist $args
  }

  ## Destroy the view
  destructor {
    catch {destroy $options(-view)}
  }


  ## Syncronization utility method for writing view parameter to device
  # 
  # @param param    name of parameter
  # @param index    index of parameter  (i.e. channel index)
  #
  method CommitViewToParam {param index} {
    set value [[$self cget -view] Get$param $index]
    # if we loaded a file, missing information, the missing info will show
    # up as "NA". We will catch when/if someone tries to write these values
    if {$value ne "NA"} {
      [$self cget -handle] Set$param $index $value
    }
  }

  ## Syncronization utility for looping CommitViewToParam
  #
  # @param param    name of parameter
  # @param begin    first index to pass to CommiteViewToParam
  # @param end      first invalid index that signifies completion
  #
  method LoopCommitView {param begin end} {
    for {set ch $begin} {$ch<$end} {incr ch} {
      $self CommitViewToParam $param $ch
    }
  }

  ## Syncronization utility for setting view state based on device state
  #
  # @param param    name of the parameter
  # @param index    index of parameter (i.e. channel index)
  method UpdateParamFromView {param index} {
    set code [catch {
      [$self cget -view] Set$param $index [[$self cget -handle] Get$param $index]
    } result]
    if {$code} {
      set msg "Failed while reading $param (ch=$index) with message: $result"
      tk_messageBox -icon error -message $msg
    }
  }

  ## Sychronization utility for loop UpdateParamFromView
  #
  # @param param  name of the parameter
  # @param begin  first index to pass to UpdateParamFromView
  # @param end    first invalid index that signifies completion
  method LoopUpdateView {param begin end} {
    for {set ch $begin} {$ch<$end} {incr ch} {
      $self UpdateParamFromView $param $ch
    }
  }

}

#------------------------------------------------------------------------------
#
# Preseneter for the MCFD16IndividualView
#
#  Servers as the synchronization logic of the MCFD16IndividualView and the
#  device. It considers the device (aka. -handle) the model that it intends to
#  manage. 
#
snit::type MCFD16IndividualPresenter {

  component m_base ;#  MCFD16Presenter instance

  delegate method * to m_base
  delegate option * to m_base

  ## Build the view and synchronize with module
  constructor {args} {
    install m_base using MCFD16Presenter %AUTO% 

    $self configurelist $args

    $self configure -view [MCFD16IndividualView [$self cget -widgetname] \
                                                  -presenter $self]

    $self UpdateViewFromModel
  }

  destructor {
    $m_base destroy
  }

  ## Method for committing state of the view to the device
  #
  # It write the state of the module and then reads back to make sure
  # that the view is properly representing the state of the device.
  #
  method Commit {} {
    $self UpdateModelFromView ;# write state
    $self UpdateViewFromModel ;# read back state
  }

  ## Set the state of the view given the model
  #
  method UpdateViewFromModel {} {

    $self UpdateViewThresholds 
    update
    $self UpdateViewPolarities
    update
    $self UpdateViewGains
    update
    $self UpdateViewWidths
    update
    $self UpdateViewDeadtimes
    update
    $self UpdateViewDelays
    update
    $self UpdateViewFractions
    update
  }


  # Various helper methods
  method UpdateViewThresholds {} { $self LoopUpdateView Threshold 0 16 }
  method UpdateViewPolarities {} { $self LoopUpdateView Polarity 0 8 }
  method UpdateViewGains {} { $self LoopUpdateView Gain 0 8 }
  method UpdateViewWidths {} { $self LoopUpdateView Width 0 8 }
  method UpdateViewDeadtimes {} { $self LoopUpdateView Deadtime 0 8 }
  method UpdateViewDelays {} { $self LoopUpdateView Delay 0 8 }
  method UpdateViewFractions {} { $self LoopUpdateView Fraction 0 8 }

  ## Write the state of the view to the device
  #
  # This performs the synchronization for each parameter type at a time.
  # It does so by calling various helper methods. The helper methods only
  # iterate over non-common settings so that for threshold it writes values for
  # channels [0,15] and for all others it write values for channels [0,7].
  #
  method UpdateModelFromView {} {

    $self CommitViewThresholds 
    update
    $self CommitViewPolarities
    update
    $self CommitViewGains
    update
    $self CommitViewWidths
    update
    $self CommitViewDeadtimes
    update
    $self CommitViewDelays
    update
    $self CommitViewFractions
  }

  ## Commit data to module
  method CommitViewThresholds {} { $self LoopCommitView Threshold 0 16 }
  method CommitViewPolarities {} { $self LoopCommitView Polarity 0 8 }
  method CommitViewGains {} { $self LoopCommitView Gain 0 8 }
  method CommitViewWidths {} { $self LoopCommitView Width 0 8 }
  method CommitViewDeadtimes {} { $self LoopCommitView Deadtime 0 8 }
  method CommitViewDelays {} { $self LoopCommitView Delay 0 8 }
  method CommitViewFractions {} { $self LoopCommitView Fraction 0 8 }
}


# -----------------------------------------------------------------------------
#
# Presenter for the MCFD16CommonView
#
# This is essentially the same thing as the MCFD16IndividualPresenter except
# that it only sets the common parameters. It is expected that the device is
# already in a common mode for writing. If it isn't, it is not gauranteed that
# the write operations will succeed.
#
snit::type MCFD16CommonPresenter {

  component m_base ;# MCFD16Presenter instance

  delegate option * to m_base
  delegate method * to m_base

  ## Set up the view and synchronize view to the device
  #
  # @param  args  the option-value pairs
  constructor {args} {
    install m_base using MCFD16Presenter %AUTO% 

    $self configurelist $args

    $self configure -view [MCFD16CommonView [$self cget -widgetname] -presenter $self]

    $self UpdateViewFromModel
  }

  ## @brief Clean up
  destructor {
    $m_base destroy
  }

  ## @brief Write the view state to the device, then read back new device state
  #
  method Commit {} {
    $self UpdateModelFromView
    update
    $self UpdateViewFromModel
  }

  ## @brief Write the state of the parameters in the view to the device
  #
  method UpdateViewFromModel {} {

    # not sure if the names is something we need to record
    $self UpdateViewThresholds 
    update
    $self UpdateViewPolarities
    update
    $self UpdateViewGains
    update
    $self UpdateViewWidths
    update
    $self UpdateViewDeadtimes
    update
    $self UpdateViewDelays
    update
    $self UpdateViewFractions
    update
  }

  ## Write the state of the device to the view
  #
  # Uses a whole bunch of helper methods that perform the 
  # synchronization for only a single parameter at a time.
  #
  method UpdateModelFromView {} {
    # make sure the mode is set first
    # becuase subsequent writes may depend on it

    $self CommitViewThresholds 
    update
    $self CommitViewPolarities
    update
    $self CommitViewGains
    update
    $self CommitViewWidths
    update
    $self CommitViewDeadtimes
    update
    $self CommitViewDelays
    update
    $self CommitViewFractions
  }


  # ---- Utility methods 
  # for synchronizing the view state to the device state
  method UpdateViewThresholds {} { $self UpdateParamFromView Threshold 16 }
  method UpdateViewPolarities {} { $self UpdateParamFromView Polarity 8 }
  method UpdateViewGains {} { $self UpdateParamFromView Gain 8 }
  method UpdateViewWidths {} { $self UpdateParamFromView Width 8 }
  method UpdateViewDeadtimes {} { $self UpdateParamFromView Deadtime 8 }
  method UpdateViewDelays {} { $self UpdateParamFromView Delay 8 }
  method UpdateViewFractions {} { $self UpdateParamFromView Fraction 8 }


  ## Commit data to module
  method CommitViewThresholds {} { $self CommitViewToParam Threshold 16 }
  method CommitViewPolarities {} { $self CommitViewToParam Polarity 8 }
  method CommitViewGains {} { $self CommitViewToParam Gain 8 }
  method CommitViewWidths {} { $self CommitViewToParam Width 8 }
  method CommitViewDeadtimes {} { $self CommitViewToParam Deadtime 8 }
  method CommitViewDelays {} { $self CommitViewToParam Delay 8 }
  method CommitViewFractions {} { $self CommitViewToParam Fraction 8 }
}


########## Unified Controls ####################
#
# This is the core configuration widget component of the application. It provides a
# megawidget that holds a MCFD16Common* and MCFD16Individual* set of controls.
# The views for these controls are in a FrameSwitcher widget that allows for
# simple switching back and forth between the visible frame. The main
# responsibility of this widget is to manage what is visible and also to
# coordinate events between the various controls it manages. There is also a
# PulserPresenter that forms a single row of controls at the bottom of the
# widget.
#
# The read may notice that the separation of view and presenter is not present
# for this widget. It could be made to follow the paradigm but I will leave that
# as a task for the future.
#
snit::widget MCFD16ControlPanel {

  component m_frames        ;# the frame switcher
  component m_comPrsntr     ;# instance of MCFD16CommonPresenter
  component m_indPrsntr     ;# instance of MCFD16IndividualPresenter
  component m_pulserPrsntr  ;# instance of PulserPresenter

  option -handle -default {} -configuremethod {SetHandle} ;# the device driver
  variable m_mode  ;# current config mode of the device (selects visible frame)
  variable m_current

  ## Create all presenters and build the gui
  #
  # This reads from the device the configuration mode and then displays the
  # appropriate controls for the response it receives. 
  #
  # @param  args  option-value pairs (really should only be -handle)
  # 
  # @throws error if no handle is provided.
  constructor {args} {
    $self configurelist $args

    if {[$self cget -handle] eq ""} {
      return -code error "MCFD16ControlPanel must be provided -handle option."
    }

    #  install m_frames using FrameSwitcher $win.frames
    install m_frames using FrameManager $win.frames
    install m_comPrsntr using \
      MCFD16CommonPresenter %AUTO% -widgetname $win.com \
                                   -handle [$self cget -handle]

    install m_indPrsntr using \
      MCFD16IndividualPresenter %AUTO% -widgetname $win.ind \
                                   -handle [$self cget -handle]

    install m_pulserPrsntr using \
      PulserPresenter %AUTO% [PulserView $win.plsr] \
                                   [$self cget -handle]

    set m_mode [[$self cget -handle] GetMode]
    trace add variable [myvar m_mode] write [mymethod OnModeChange]

    $self BuildGUI

    # sets the m_current and updates the frame switcher
    $self OnModeChange {} {} {}

  }

  ## @brief Create the widget and then install them into the megawidget.
  #
  method BuildGUI {} {
    # add the frames
    $m_frames add common $win.com
    $m_frames add individual $win.ind

    ## construct the frame containing mode elements
    set mode $win.mode
    ttk::frame $mode -style Mode.TFrame
    ttk::label $mode.modeLbl -text "Configuration Mode" -style Mode.TLabel
    ttk::radiobutton $mode.modeCom -text "common" -variable [myvar m_mode] \
                                  -value common -style Mode.TRadiobutton
    ttk::radiobutton $mode.modeInd -text "individual" -variable [myvar m_mode] \
                                  -value individual -style Mode.TRadiobutton
    grid $mode.modeLbl $mode.modeCom $mode.modeInd -sticky new
    grid columnconfigure $mode {0 1 2} -weight 1

    ttk::button $win.commit -text "Commit to Device" -command [mymethod Commit] \
                                  -style "Commit.TButton"
    ttk::button $win.update -text "Update from Device" -command [mymethod Update] \
                                  -style "Commit.TButton"

    grid $win.mode - -sticky new
    grid $m_frames - -sticky nsew

    grid $win.commit  $win.update  -sticky ew -pady 4
    grid $win.plsr - -sticky sew -pady 4

    grid rowconfigure $win 1 -weight 1
    grid columnconfigure $win {0 1 2} -weight 1
  }


  ## Display the appropriate controls for the current mode
  #
  # This gets called every time the m_mode variable is written to
  # and invokes select in the FrameSwitcher to keep the displayed state correct.
  # 
  # @param name0  name of variable
  # @param name1  subname of variable
  # @param op     operation
  #
  method OnModeChange {name0 name1 op} {
    if {$m_mode eq "common"} {
      $m_frames select common
      set m_current $m_comPrsntr
    } else {
      $m_frames select individual 
      set m_current $m_indPrsntr
    } 
  }


  ## Command triggered when "commit to device" button is pressed
  #
  # Calls commit for the current presenter object. This is a more efficient
  # way of operating than writing every parameter to the device every time.
  # Rather, this will only set the common values if that is the desired state.
  #
  method Commit {} {
    [$self cget -handle] SetMode $m_mode ;# set mode first to make sure that
                                         ;# subsequent writes succeed.
    $m_current Commit
  }

  ## @brief Commit the channel settings and also the pulser state
  #
  # By default, the Commit method will only call Commit on the currently
  # displayed presenter. This is because the commmon settings don't necessarily
  # stick if the device is not in common configuration modes.
  #
  method CommitAll {} {
    [$self cget -handle] SetMode $m_mode ;# set mode first to make sure that
                                         ;# subsequent writes succeed.
    $m_current Commit
    $m_pulserPrsntr CommitViewToModelNoTransition

  }

  ## Call each of the presenters' UpdateViewFromModel methods
  #
  # Basically, this resynchronizes the state of the view to module.
  # It is called by the "Update from Device"
  method Update {} {
    # first read from the device the mode 
    set m_mode [[$self cget -handle] GetMode]

    # for each of the modes, read in the modules values
    $m_comPrsntr UpdateViewFromModel 
    $m_indPrsntr UpdateViewFromModel 
    $m_pulserPrsntr UpdateViewFromModel
  }

  ## Retrieve the current controls presenter
  #
  method GetCurrent {} { return $m_current }

  ## @brief Callback for when a new handle is passed in
  #
  # This is called when "$self configure -handle val" is evaluated.
  # It makes sure that all of the presenters it manages get passed the new
  # handle as well.  It also is responsible for actually storing the new option
  # value.
  #
  # @param  opt   the option name (should only be -handle)
  # @param  val   name of a device driver instance
  #
  method SetHandle {opt val} {
    # pass the handle around to all of the sub-presenters
    if {$m_comPrsntr ne ""} {
      $m_comPrsntr configure -handle $val
    }

    if {$m_indPrsntr ne ""} {
       $m_indPrsntr configure -handle $val
    }
    if {$m_pulserPrsntr ne ""} {
      $m_pulserPrsntr SetHandle $val
    }

    # set the option
    set options($opt) $val
  }
}

#######
#

## @brief A widget for saving state
#
# This is the widget that the user interacts with when they try to save the
# state  of the Gui to a file. It is not implemented using the MVP design
# because the logic is mingled with the actual view code. However, it is a
# pretty simple task that it must achieve, so it was chosen for simplicity.
# 
# Besides provides the mechanism for browsing to a file, this provides the
# mechanism for actually writing the state to file. It delegates some
# responsibilities to other snit::types like ScriptHeaderGenerator, but
# otherwise swaps in an MCFD16CommandLogger to the MCFD16ControlPanel and calls
# Commit. In this way, the same exact code executed to write the Gui state to an
# MCFD16 device is executed for writing the file. At the end of this, the real
# device handle is put back into place for use.
#
# The file produced by this is a standard tcl script that should have no problem
# being executed as "tclsh < savedFile.tcl" as long as the proper directories
# are included in the TCLLIBPATH.
snit::widget SaveToFileForm {

  variable _theApp ;# the MCFD16GuiApp
  variable _path   ;# file path to write the state to.
  variable _displayPath

  ## @brief Pass in the state
  #
  # @param app    the name of the MCFD16GuiApp instance
  # @param args   option-value pairs for the potential future expansion (not
  # used at the moment.
  constructor {app args} {
    set _theApp $app
    # this sets up the path and the display path
    $self setPath ""

    $self BuildGUI
  }

  ## @brief Assemble the widgets into a unified megawidget
  #
  method BuildGUI {} {
    ttk::label  $win.pathLbl -text "Output file name"
    ttk::entry  $win.pathEntry -textvariable [myvar _path] -width 24 
    ttk::button $win.browse -text "Browse"  -command [mymethod Browse] -width 8
    ttk::button $win.save -text "Save"  -command [mymethod Save] -width 8

    grid  $win.pathLbl $win.pathEntry $win.browse -sticky ew -padx 4 -pady 4
    grid  $win.save - - -sticky ew -padx 4 -pady 4
    grid columnconfigure $win 1 -weight 1
  }

  ## @brief Launch a tk_getSaveFile dialogue when Browse button is pressed
  #
  # when a non-empty path is returned from the browser, the path is updated.
  method Browse {} {
    set path [tk_getSaveFile -confirmoverwrite 1 -defaultextension ".tcl" \
                    -title {Save as} ] 
    if {$path ne ""} {
      $self setPath $path  
    }
  }


  ## @brief The logic for saving
  #
  # The logic is described in the description of this snit::widget so I will
  # just add the description inline.
  #
  method Save {} {
    if {$_path eq ""} {
      tk_messageBox -icon error -message "User must specify file name."
      return
    }

    if {[catch {open $_path w} logFile]} {
      tk_messageBox -icon error -message "Failed to open $_path for writing."
      return
    }

    set options [$_theApp GetOptions]
    
    # write the header portion that instantiates a device driver
    ScriptHeaderGenerator gen $options
    set lines [gen generateHeader]
    foreach line $lines {
      chan puts $logFile $line
    }
    gen destroy

    # generate the lines in the file make device driver calls
    MCFD16Factory factory $options
    set logger [factory create cmdlogger $logFile]
    set realHandle [$_theApp GetHandle]
    set control [$_theApp GetControlPresenter]

    # replace real driver with logging driver
    $control configure -handle $logger

    # commit (this causes all of the gui state to be written to file)
    $control CommitAll
    $logger Flush ;# flush buffers to make sure it all gets into the file

    # replace the logging driver with the real driver and update the view
    $control configure -handle $realHandle
    $control Update

    # write the name of the channels to the logfile
    $self WriteChannelNames $logFile MCFD16ChannelNames

    # cleanup
    $logger destroy
    factory destroy

    # close the file channel because nothing owns it.
    close $logFile
  }

  ## @param Write the name of the channels to the log file
  #
  # @param logFile      the file channel to write to
  # @param nmspaceName  name of namespace describing channels
  #
  method WriteChannelNames {logFile nmspaceName} {
    for {set ch 0} {$ch<16} {incr ch} {
      chan puts $logFile "set ${nmspaceName}::chan$ch [set ${nmspaceName}::chan$ch]"
    }
  }

  ## @brief Update path
  #
  # @param  path    new path string
  method setPath path {
    set _path $path
    set _displayPath [file tail $path]
  }
}


## @brief The snit::widget responsible for loading state from a .tcl file
#
# This follows the MVP style where the logic is separated from the view. In this
# case, the view is kept very humble by not implementing any callbacks other
# than to forward events to the presenter.
#
# Unlike most other MVP designs, this will not blow up if there is no presenter
# defined. Instead, events are dropped on the floor and not passed on in the
# absence of the presenter. 
snit::widget LoadFromFileForm {

  option -presenter -default {} 

  variable _path ;# the path to the file to load

  ## @brief Parse options and build GUI
  #
  # @param   args   the option-value pairs
  #
  constructor {args} {
    $self configurelist $args

    $self BuildGUI
  }

  ## @brief Assemble the widgets into a megawidget
  #
  method BuildGUI {} {
    ttk::label  $win.pathLbl -text "Input file name"
    ttk::entry  $win.pathEntry -textvariable [myvar _path] -width 24 
    ttk::button $win.browse -text "Browse"  -command [mymethod Browse] -width 8
    ttk::button $win.load -text "Load"  -command [mymethod Load] -width 8

    grid $win.pathLbl $win.pathEntry $win.browse -sticky ew -padx 4 -pady 4
    grid $win.load - - -sticky ew -padx 4 -pady 4
    grid columnconfigure $win 1 -weight 1
  }

  ## @brief Forward event to browse to the presenter
  #
  method Browse {} {
    if {[$self cget -presenter] eq {} } {
      return
    } else {
      [$self cget -presenter] Browse 
    }
  }

  ## @brief forware event to load to the presenter
  #
  method Load {} {
    if {[$self cget -presenter] eq {} } {
      return
    } else {
      [$self cget -presenter] Load $_path
    }
  }

  ## @brief Setter for the path
  #
  method SetPath {path} {
    set _path $path
  }
}

## @brief Logic for the LoadFromFileView
#
# The logic for this operation is more complicated than in most parts of this
# gui in order to ensure that loading from a file is safe. The main
# functionality of this is to respond to the events generated by the view,
# react, and the update the view. The complexity lies in the fact that it parses
# the file to load for only the allowed API calls. In this way, the method
# prevents the execution of lines like: exec rm *. The logic does not go so far
# as to totally prevent such a thing from being executed but does reduce the
# risk. It also writes the names to the file. One could argue that some of that
# logic should live in a separate snit::type, but for the moment it is the
# responsibility of this.
#
# It is possible to construct this snit::type without an associated view to
# control. This makes it easier to test instances.
#
snit::type LoadFromFilePresenter {

  option -view -default {} -configuremethod SetView
  variable _contentFr


  ## @brief Construct and set up the state
  #
  # @param contentFr    an MCFD16ControlPanel instance
  # @param args         option-value pairs
  #
  constructor {contentFr args} {
    set _contentFr $contentFr 

    $self configurelist $args
  }
  
  ## @brief Callback for "configure -view"
  #
  #  Stores the new -view but also passes $self to the view as the -presenter
  #  option.
  #   
  #  @param opt   option name (must be -view)
  #  @param val   name of LoadFromFileForm
  #
  method SetView {opt val} {
    $val configure -presenter $self
    set options($opt) $val
  }

  ## @brief Launch a file chooser dialogue
  #
  # Any non-empty path returned by the file chooser dialogue is used to set the
  # path displayed by the view.
  #
  method Browse {} {
    set path [tk_getOpenFile -defaultextension ".tcl" \
                    -title {Choose file to load} ] 
                  
    if {($path ne "") && ([$self cget -view] ne {})} {
      [$self cget -view] SetPath $path  
    }
    
  }

  ## @brief Load a file into memory and use it to set the state of the Gui
  #
  # Comments are provided inline to facilitate the understanding of the logic.
  # FOr the most part, this reads evaluable chunks of a file into a list,
  # removes all chunks that are not relevant, recognized MCFD16 driver API calls
  # or lines that set the name from the list, creates an MCFD16Memorizer that
  # will be operated on by the list elements, swaps in an MCFD16Memorizer to the
  # MCFD16ControlPanel, updates the MCFD16ControlPanel using the memorizer, and
  # then replaces the memorizer with the real driver.
  #
  # @param  path    the path to a file containing state 
  method Load {path} {
    if {![file exists $path]} {
      set msg "Cannot load from $path, because file does not exist."
      tk_messageBox -icon error -message $msg
    } elseif {! [file readable $path]} { 
      set msg "Cannot load from $path, because file is not readable."
      tk_messageBox -icon error -message $msg
    }

    # split the file into complete chunks that can be passed to eval
    set rawLines [$self TokenizeFile $path]

    # find the lines we can safely execute
    set executableLines [$self FilterOutNonAPICalls $rawLines]

    # determine the first
    set devName [$self ExtractDeviceName [lindex $executableLines 0]]
    if {[llength [info commands $devName]]>0} {
      if {[$_contentFr cget -handle] ne $devName} {
        rename $devName {}
      } else {
        set msg "Device driver instance in load file shares same name "
        append msg "as current driver ($devName). User must use a different "
        append msg "instance name in the load file."
        tk_messageBox -icon error -message $msg
        return
      }
    }

    set fakeHandle [MCFD16Memorizer $devName]

    # load state into device
    $self EvaluateAPILines $executableLines

    # update the actual content
    set realHandle [$self SwapInHandle $fakeHandle]
    $_contentFr Update
    set fakeHandle [$self SwapInHandle $realHandle]

    $fakeHandle destroy
    catch {close $loadFile}
  }

  ## @brief Split the file into fully executable chunks
  #
  # This uses the BlockCompleter snit::type to find all complete blocks (i.e. at
  # the end of a line the number of left and right curly braces are equal). The
  # list of complete blocks are returned.
  #
  # @param path   the path to the file to parse
  #
  # @returns  the list of complete blocks 
  method TokenizeFile {path} {
    # if here, the file exists and can be updated
    set loadFile [open $path r]

    set blocks [list]
    BlockCompleter bc -left "{" -right "}"
 
    while {![chan eof $loadFile]} {
      bc appendText [chan gets $loadFile]
      while {![bc isComplete] && ![chan eof $loadFile]} {
        bc appendText "\n[chan gets $loadFile]"
      }
      lappend blocks [bc getText]
      bc Reset
    }

    bc destroy
    return $blocks
  }

  ## @brief Replace current handle with a new handle
  #
  # @param  newHandle name of the instance to pass into the MCFD16ControlPanel
  #
  # @returns the name of the previous instance managed by the MCFD16ControlPanel
  method SwapInHandle {newHandle} {
    set oldHandle [$_contentFr cget -handle]
    $_contentFr configure -handle $newHandle

    return $oldHandle
  }

  ## @brief Check to see if second element is an API call
  #
  # In a well-formed call to a device driver, the first element will be the name
  # of the device driver instance and the second element of the line will be the
  # actual method name. If the second element is not a string that is understood
  # to be a valid method name, then we return false. That is only if that line
  # is also not recognized as a command to manipulate the names.
  # 
  # @param  lines   the list of lines to filter
  #
  # @returns a list of lines that only contain  valid api calls or name
  # manipulation code
  method FilterOutNonAPICalls lines {
    set validLines [list]
    foreach line $lines {
      if {[$self IsValidAPICall $line] || [$self IsNameLine $line]} {
        lappend validLines $line
      }
    }
    return $validLines
  }

  ## @brief Retrieve name of the device driver from a line
  #
  # Given a line that is a driver call, the first element is the driver name.
  # This first element is extracted. It is up to the caller to ensure that the
  # line is actually an API call.
  #
  # @param line   the line of code to look in.
  #
  # @returns a device name specified as existing at the global scope
  method ExtractDeviceName {line} {
    set tokens [split $line " "]
    set name [lindex $tokens 0]
    if {[string first "::" $name] != 0} {
      set name "::$name"
    }
    return $name
  }

  ## @brief Check whether the second element of a line is a valid api call
  #
  # This snit::type provides a list of strings that it considers valid API
  # calls. If the second element of the line is in this list, then it is
  # considered an api call.
  #
  # @param line   line of code to check
  #
  # @returns boolean
  # @retval 0 - second element is not in list of valid calls
  # @retval 1 - otherwise
  method IsValidAPICall {line} {
    # we are only treating simple lines where the second element is the verb
    # this is valid for all 
    set verb [lindex $line 1]
    return [expr {$verb in $_validAPICalls}]
  }

  method EvaluateAPILines {lines} {
    foreach line $lines {
      #puts $line
      uplevel #0 eval $line
    }
  }

  ## @brief Check whether a line looks like it will set a channel name
  #
  # @returns boolean
  # @retval 0 - doesn't satisfy regular expression
  # @retval 1 - otherwise
  method IsNameLine {line} {
    set expression {^\s*set\s+(::)*MCFD16ChannelNames::chan.*$}
    return [regexp $expression $line]

  }

  # Type data .... 
  typevariable _validAPICalls ;# list of calls consider valid API calls

  ## @brief Populate the list of valid api calls
  #
  typeconstructor {
    set _validAPICalls [list]
    lappend _validAPICalls "SetThreshold"
    lappend _validAPICalls "GetThreshold"
    lappend _validAPICalls "SetGain"
    lappend _validAPICalls "GetGain"
    lappend _validAPICalls "SetWidth"
    lappend _validAPICalls "GetWidth"
    lappend _validAPICalls "SetDeadtime"
    lappend _validAPICalls "GetDeadtime"
    lappend _validAPICalls "SetDelay"
    lappend _validAPICalls "GetDelay"
    lappend _validAPICalls "SetFraction"
    lappend _validAPICalls "GetFraction"
    lappend _validAPICalls "SetPolarity"
    lappend _validAPICalls "GetPolarity"
    lappend _validAPICalls "SetMode"
    lappend _validAPICalls "GetMode"
    lappend _validAPICalls "EnableRC"
    lappend _validAPICalls "RCEnabled"
    lappend _validAPICalls "SetChannelMask"
    lappend _validAPICalls "GetChannelMask"
    lappend _validAPICalls "EnablePulser"
    lappend _validAPICalls "DisablePulser"
    lappend _validAPICalls "PulserEnabled"
  }
}

## @brief The widgets for enabling and disabling channels
# 
# This is intended for controlling the channel mask parameter. It is reached by
# the menu bar. This particular snit::widget does not actually do anything other
# than maintain a set a widgets and a means to access or set the values
# controlled by them. 
#
# It is intended to be controlled by the ChannelEnableDisablePresenter.
#
#
snit::widget ChannelEnableDisableView {

  option -presenter -default {} 
  variable _bit0
  variable _bit1
  variable _bit2
  variable _bit3
  variable _bit4
  variable _bit5
  variable _bit6
  variable _bit7

  variable _rows

  ## @brief Build the GUI and set the state
  #
  constructor {args} {
    $self configurelist $args

    for {set ch 0} {$ch < 8} {incr ch} {
      set _bit$ch 0
    }

    set _rows [list]

    $self BuildGUI
  }

  ## @brief Buil
  #
  method BuildGUI {} {
    $self BuildHeader $win.header

    for {set ch 0} {$ch < 16} {incr ch 2} {
      set row [expr $ch/2]
      $self BuildGroupedRows $win.row$row $ch
      lappend _rows $win.row$row
    }

    $self BuildButtons $win.buttons

    grid $win.header -sticky nsew -padx 4 -pady 4
    foreach row $_rows {
      grid $row -sticky ew -padx 4 -pady 4
    }
    grid $win.buttons -sticky nsew -padx 4 -pady 4


    grid rowconfigure $win [Utils::sequence 0 [expr [llength $_rows]+1] 1] -weight 1 
    grid columnconfigure $win 0 -weight 1
  }

  method BuildHeader {name} {
    ttk::frame $name
    ttk::label $name.title -text "Enable / Disable channels" -style "Title.TLabel"

    grid $name.title -sticky nsew -padx 4 -pady 4
  }

  #
  method BuildGroupedRows {name ch} {
    set style Grouped

    set pair [expr $ch/2]

    set w $name
    ttk::frame $w -style $style.TFrame

    # construct first row
    ttk::entry $w.na$ch -width 8 -textvariable MCFD16ChannelNames::chan$ch \
                        -style "$style.TEntry" \
                                -validate focus -validatecommand [mymethod ValidateName %P] \
                                -invalidcommand [mymethod ResetChannelName %W]

    ttk::radiobutton $w.off$pair -variable [myvar _bit$pair] -value 1 \
                        -style "$style.TRadiobutton" -text "disable"

    ttk::radiobutton $w.on$pair -variable [myvar _bit$pair] -value 0 \
                        -style "$style.TRadiobutton" -text "enable"

    grid $w.na$ch $w.off$pair $w.on$pair -sticky news -padx 4 -pady 4

    # construct second row 
    incr ch
    ttk::entry $w.na$ch -width 8 -textvariable MCFD16ChannelNames::chan$ch \
      -style "$style.TEntry" \
      -validate focus -validatecommand [mymethod ValidateName %P] \
                      -invalidcommand [mymethod ResetChannelName %W]
    grid $w.na$ch x x -sticky news -padx 4 -pady 4

    # allow the columns to resize
    grid columnconfigure $w {1 2} -weight 1 -uniform a
    grid columnconfigure $w {0} -weight 2 -uniform a
    grid rowconfigure $w {0 1} -weight 1 
  }

  method BuildButtons {name} {
    ttk::frame $name 
    ttk::button $name.commit -text "Commit" -style "Commit.TButton" \
                            -command [mymethod Commit]
    
    ttk::button $name.update -text "Update" -style "Commit.TButton" \
                            -command [mymethod Update] 

    grid $name.commit $name.update -sticky nsew -padx 4 -pady 4
    grid columnconfigure $name {0 1} -weight 1
  }

  ## Check whether a channel name contains non-whitespace characters
  # 
  # This is called when a channel entry loses focus
  #
  # @param name   candidate string 
  #
  # @return boolean
  # @retval 0 - string was empty or all whitespace
  # @retval 1 - otherwise
  method ValidateName {name} {
    set name [string trim $name]
    return [expr [string length $name]!=0]
  }

  ## Reset channel to a simple string
  #
  # Typically called with ValidateName returns false
  #
  # @returns ""
  method ResetChannelName {widget} {
    set str [$widget cget -textvariable]
    regexp {^.*(\d+)$} $widget match ch
    set $str "Ch$ch"
  }

  ## @brief Set a specific bit to a value
  #
  # This does not check for valid parameter values and leaves it to the caller
  # to make sure that they are reasonable.
  #
  # @param index  bit index (must be in range [0,7])
  # @param val    value  (must be either 0 or 1)
  method SetBit {index val} {
    set _bit$index $val
  }

  ## @brief Retrieve the index-th bit value
  # 
  # There is no parameter checking in this so it is up to the caller to pass in
  # a good bit index.
  #
  # @param index  index of bit (must be in range [0,7])
  #
  # @returns value of bit
  method GetBit {index} {
    return [set _bit$index]
  }

  ## @brief Forward commit event to the presenter
  #
  # If no presenter exists, then nothing is done.
  #
  method Commit {} {
    set presenter [$self cget -presenter]
    if {$presenter ne {}} {
      $presenter Commit
    }
  }

  ## @brief Forward an update event to the presenter
  #
  # If no presenter exists, then nothing is done.
  #
  method Update {} {
    set presenter [$self cget -presenter]
    if {$presenter ne {}} {
      $presenter Update
    }
  }
}


##############################################################################

## @brief The logic for the ChannelEnableDisableView
#
snit::type ChannelEnableDisablePresenter {

  option -view -default {} -configuremethod SetView
  option -handle -default {} -configuremethod SetHandle

  ## @brief Parse options and construct
  #
  constructor {args} {
    $self configurelist $args
  }

  ## @brief Handler for when the view state is to be transmitted to device
  #
  # First commits the mask and then reads the state back from the device
  #
  method Commit {} {
    $self CommitMask
    $self UpdateViewFromModel
  }

  ## @brief Read the state from the device and synchronize to the view
  #
  method Update {} {
    $self UpdateViewFromModel
  }

  ## @brief Read the state from the device and synchronize to the view
  #
  # @throws error if no handle exists
  # @throws error if no view exists
  # @throws error if communication fails
  method UpdateViewFromModel {} {
    # verify that first there is a device to communicate with
    set handle [$self cget -handle]
    if {$handle eq {}} {
      set msg {ChannelEnableDisablePresenter::UpdateViewFromModel }
      append msg {Cannot access model because it does not exist.}
      return -code error $msg
    }

    # verify that first there is a view to communicate with
    set view [$self cget -view]
    if {$view eq {}} {
      set msg {ChannelEnableDisablePresenter::UpdateViewFromModel }
      append msg {Cannot update view because it does not exist.}
      return -code error $msg
     }

    # try to read the channel mask
    if {[catch {$handle GetChannelMask} mask]} {
      set msg "Failure while reading channel mask from device with message : "
      append msg "$mask"
      tk_messageBox -icon error -message $msg
    } else {

      # split the mask into a list of bits
      set bits [$self DecodeMaskIntoBits $mask]

      # update the view
      for {set bit 0} {$bit < 8} {incr bit} {
        $view SetBit $bit [lindex $bits $bit]
      }
    }
  }

  ## @brief Write the state of the view to the device
  #
  # @throws error if no handle exists
  # @throws error if no view exists
  #
  method CommitMask {} {
    # check for the presence of a handle
    set handle [$self cget -handle]
    if {$handle eq {}} {
      set msg {ChannelEnableDisablePresenter::CommitMask }
      append msg {Cannot access model because it does not exist.}
      return -code error $msg
    }

    # check for the presence of a view
    set view [$self cget -view]
    if {$view eq {}} {
      set msg {ChannelEnableDisablePresenter::CommitMask }
      append msg {Cannot update view because it does not exist.}
      return -code error $msg
    }

    # get the bits from the view and load them into the state
    set bits [list]
    for {set index 0} {$index < 8} {incr index} {
      lappend bits [$view GetBit $index]
    }

    # turn list of bits into a number
    set mask [$self EncodeMaskIntoBits $bits]

    # write the mask
    $handle SetChannelMask $mask
  }

  # UTILITY METHODS

  ## @brief Split an integer into a list of bits
  #
  # Given an integer, convert it to a list of 0s and 1s that represent it. Split
  # the bits up and form a list. For example, passing 100 (0x64) into this method, the
  # result will be {0 0 1 0 0 1 1 0}
  #
  # @returns list of 8 bits (least significant bit first)
  method DecodeMaskIntoBits {mask} {
    set bits [list]

    # interpret mask as an actual byte
    set byteValue [binary format s1 $mask]

    # convert byte into representation of bits as a string
    set count [binary scan $byteValue b8 binRep]

    # split each character up to form a list of bits
    return [split $binRep {}]
  }

  ## @brief Turn a list of bits into an equivalent integer
  #
  # This is the opposite operation of the DecodeMaskIntoBits. Given a list i
  # {0 0 1 0 0 1 1 0}, the method will return a value of 100.
  #
  # @param list of bits (least significant bit first)
  #
  # @returns an integer
  method EncodeMaskIntoBits {bits} {
    set mask 0

    # collapse list of bits into a single word by removing spaces
    set binRepStr [join $bits {}]

    # convert string representation of bits into an actual byte
    set binByte [binary format b8 $binRepStr]

    # interpret the byte as an 8-bit signed number
    set count [binary scan $binByte c1 mask]

    # becuase the number if signed and padded, we mask out upper bits
    set mask [expr {$mask & 0xff}]

    return $mask
  } 

  ## @brief Callback for a "configure -view" operation
  # 
  # Performs the handshake required when the view is set. A new view is passed
  # $self as the value to its -presenter option. If a handle exists, it is
  # appropriate to update the state of the view from it.
  #
  # @param opt    option name (should be -view)
  # @param val    value (name of view)
  method SetView {opt val} {
    # store the new view (opt="-view", val = new view name)
    set options($opt) $val
    $val configure -presenter $self

    # we have a handle already, then update!
    set handle [$self cget -handle]
    if {$handle ne {}} {
      $self UpdateViewFromModel
    }
  }

  ## @brief Callback for a "configure -handle" operation
  #
  # Sets the -handle option to the new value and also updates the view from it
  # if the -view option is set.
  #
  # @param opt  option name (should be -handle)
  # @param val  value (name of handle)
  #
  method SetHandle {opt val} {
    # store the new handle (opt="-handle", val = new handle name)
    set options($opt) $val

    # we have a view already, then update!
    set view [$self cget -view]
    if {$view ne {}} {
      $self UpdateViewFromModel
    }
  }
}



################################################################################

## @brief Megawidget for controlling the pulser state
#
# The PulserView is just a collection of widgets. It assembles them and then
# forwards any events to the widgets to a presenter object if one exists. It is
# intended to work with a PulserPresenter only.
# 
#
snit::widget PulserView {
  hulltype ttk::frame ;# force the hulltype to ensure we can set the style
  option -pulserid   -default 1 ;# value of pulser (1 or 2)
  option -enabled    -default 0 ;# whether state of buttons are enabled
  option -buttontext -default {Enable}  ;# 
  option -radiobuttonstate -default 0 -configuremethod RadiobuttonStateChange

  option -presenter -default {} ;# option presenter name

  delegate option * to hull

  ## @brief Construct the widget and parse the options
  #
  constructor {args} {
    $self configurelist $args

    $self BuildGUI
  }

  ## @brief Assemble the widgets
  #
  method BuildGUI {} {
    $self configure -style Pulser.TFrame

    ttk::label $win.lbl -text "Test Pulser" -style "Pulser.TLabel"

    ttk::radiobutton $win.mHzPulser -text "2.5 MHz" \
                                    -variable [myvar options(-pulserid)] \
                                    -value 1 -style "Pulser.TRadiobutton"
    ttk::radiobutton $win.kHzPulser -text "1.22 kHz" \
                                    -variable [myvar options(-pulserid)] \
                                    -value 2 -style "Pulser.TRadiobutton"

    ttk::button $win.onoff -textvariable [myvar options(-buttontext)]\
                           -command [mymethod OnPress] \
                                          -style "Pulser.TButton"

    grid $win.lbl $win.kHzPulser $win.mHzPulser $win.onoff -sticky sew \
                                                -padx 4 -pady 4
    grid columnconfigure $win {0 1 2 3} -weight 1
  }

  ## @brief Forwards a button press event to the presenter
  #
  # If no presenter exists, then this is a no op.
  #
  method OnPress {} {
    set presenter [$self cget -presenter] 
    if {$presenter ne {}} {
      $presenter OnPress
    }
    # it is not an error if there is no presenter... it is instead a noop
  }

  ## @brief Callback for when the state changes
  #
  # This allows for a single call of (configure -radiobuttonstate ...) to set
  # the state for both of the radiobuttons.
  #
  # @param option   name of option (should only be -radiobuttonstate)
  # @param val      state to set it to (0 or 1)
  #
  method RadiobuttonStateChange {option val} {
    if {$val == 0} {
      $win.mHzPulser state disabled
      $win.kHzPulser state disabled
    } else {
      $win.mHzPulser state !disabled
      $win.kHzPulser state !disabled
    }
    set options($option) $val
  }
}

###############################################################################

## @brief Logic behind the PulserView
#
# The logic for this is designed around a state transition. When the user
# presses the Enable or Disable button, they are actually requesting a state
# transition. For this reason, this transitions the state and then updates the
# view to reflect that state. 
#
snit::type PulserPresenter {

  variable _handle ;# name of the handle instance
  variable _view ;# name of the view
  
  ## @brief Introduce the view to the presenter, setup
  #
  # Because we cannot know a priori the state of the pulser, there needs to be a
  # known state. To accomplish this we disable the pulser on construction.
  #
  # Once set up, the view is synchronized to the state of the device.
  # 
  # @param view   name of the PulserView instance
  # @param handle name of the device driver
  constructor {view handle} {
    set _view $view
    set _handle $handle

    # tell the view that this is its presenter
    $_view configure -presenter $self

    # we don't know which pulser state exists, so we will disable by default
    $_handle DisablePulser 

    # synchronize
    $self UpdateViewFromModel

  }

  ## @brief Perform appropriate transition and synchronize
  #
  method OnPress {} {
    $self TransitionAndCommitViewToModel

    # update the view state to reflect the next state
    $self UpdateViewFromModel
  }

  ## @brief Synchronize the view to the state of the device
  #
  # In this case, the only thing that can be done is to check whether or not the
  # pulser is enabled or disabled.
  #
  # Once that state is known, the view is updated to reflect it.
  method UpdateViewFromModel {} {
    # get state of the device
    set state [$_handle PulserEnabled]
    if {$state == 0} {
      set state inactive
    } else {
      set state active
    }

    # update the view given the state of device
    $self UpdateButtonText $state
    $self UpdateRadiobuttonState $state
  }

  ## @brief Transition the state of device
  #
  # If the button originally displays, "Enable", then we must transition to
  # an active state. If displying "Disable" we transition to inactive.
  # Transitioning to active means enabling the pulser and to inactive means
  # disabling the pulser. This assumes that the device and the view are in sync
  # already.
  #
  method TransitionAndCommitViewToModel {} {
    # get state of the view (determined by button text)
    set trans [$self GetTransitionType]

    # commit the state to the device
    $self TransitionPulser $trans

  }

  ## @brief Synchronize device to the view state
  #
  # This is fundamentally different than transitioning. It does not assume that
  # the device and the view are in the same state and forces the device to
  # conform to the state of the view. Doing so does not introduce a transition
  # of the over system, though it might change the state of the device.
  #
  method CommitViewToModelNoTransition {} {
    set buttontext [$_view cget -buttontext]

    if {$buttontext eq "Enable"} {
      # if button prompts to enable, then the actual state
      # is disabled
      $_handle DisablePulser
    } else {
      $_handle EnablePulser [$_view cget -pulserid]
    }
  }

  ## @brief Check whether button text is "Enable"
  #
  # In the logic, the button text is actually the flag used to determine the
  # state of the view. It is a boolean value so it is perfectly suitable.
  #
  # @returns boolean
  # @retval 1 - button text on view shows "Enable"
  # @retval 0 - otherwise
  #
  method GetViewEnabled {} {
    set text [$_view cget -buttontext]
    if {$text eq "Enable"} {
      return 0 ; # the pulser is disabled
    } else {
      return 1 ; # pulser is enabled
    }
  }

  ## @brief Write to device according to transition type
  #
  # The transition type determines whether the pulser will be enabled or
  # disabled. If the transition enables the device, the -pulserid is retrieved
  # from the view to select which pulser should be enabled.
  #  
  # @warning No parameter checking occurs and it is caller's responsibility to
  # verify that either enable or disable is passed in. 
  #
  # @param transType  either "enable" or "disable"
  #
  method TransitionPulser {transType} {
    if {$transType == "enable"} {
      # user wants to enable...
      $_handle EnablePulser [$_view cget -pulserid]
    } else {
      $_handle DisablePulser
    }
    
  }

  ## @brief Determine type of transition from the button text
  #
  # @returns string
  # @retval enable - button displays "Enable"
  # @retval disable - otherwise
  method GetTransitionType {} {
    set buttontext [$_view cget -buttontext]
    if {$buttontext eq "Enable"} {
      return enable
    } else {
      return disable
    }
  }

  ## @brief Set the button text according to the state
  #
  # @param state  state name (should be "active" or "inactive")
  #
  method UpdateButtonText {state} {
    if {$state eq "active"} {
      # user has set it into enabled
      $_view configure -buttontext "Disable"
    } else {
      $_view configure -buttontext "Enable"
    }
  }

  ## @brief Set the radiobutton state according to state type
  #
  # @param state  state name (should be active or inactive)
  #
  method UpdateRadiobuttonState {state} {
    if {$state eq "active"} {
      $_view configure -radiobuttonstate 0
    } else {
      $_view configure -radiobuttonstate 1
    }
  }

  ## @brief Pass in a new handle
  # 
  # @param handle   name of new MCFD16 driver instance to use
  #
  method SetHandle {handle} {
    set _handle $handle
  }

  ## @brief Retrieve name of currently used handle instance
  #
  # @returns name of handle
  method GetHandle {} {
    return $_handle
  }
}


