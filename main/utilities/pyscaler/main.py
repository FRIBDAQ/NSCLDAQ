'''
  This contains the main line code for the pyScaler python scaler display.
  
  @file main.py
  @brief main pyScaler program.
'''


#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014, 2026
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#	     FRIB
#	     Michigan State University
#	     East Lansing, MI 48824-1321


from sys import argv, exit, stderr, path
#
#  Extend our python seach path to include the unified format
# library and import that too:
from os import environ
if 'DAQROOT' not in environ.keys():
    print('ERROR you must set up a DAQ version first', file=stderr)
    exit(-1)

unified_fmt_dir = f'{environ["DAQROOT"]}/unifiedformat/python'
path.append(unified_fmt_dir)


import configfile
import ScalerGui
import source_manager


import daqformat

from   PyQt5.QtWidgets import QApplication, QMainWindow, QMessageBox




## 
# usage:
# Output program usage.
def usage() -> None:
    print("Usage:" , file=stderr)
    print(f'   {argv[0]}  configfile', file=stderr)
    print( 'Where', file=stderr)
    print('    configfile - is a pyScaler configuration file.', file=stderr)
    
    
##
#  configure_display
#
#  Configure the tabs etc. in the GUI:
# Parameters:
#    display - the ScalerDisplay widget to configure.
#    configuration - the processed configuration.
#
def configure_display(display : ScalerGui.SclalerDisplay, configuration: configfile.Configuration) -> None :
    #
    # Add the notebook pages:
    
    pages = configuration.pages()
    for page in pages:
        display.addPage(page)

##
#  Given a scaler name and source generate the qualified name:
#
def qualify_name(source: str, name: str) -> str :
    return f'{source}.{name}'

##### Handlers for the various ring item types we care about.

#
#  A data source gave us data.  Depending on the type of ring item dispatch
# to the appropriate handler above.
#

#
#  Common state change handling code:
#   item - a state change ringitem
#   newstate - the textual new run state to display.
#   display  - the display page.
#
def state_change(item : daqformat.statechangeitem, newstate :str, display : ScalerGui.ScalerDisplay) -> None:
    display.setRunState(newstate)
    display.setRunTitle(item.getTitle())   
    display.setRunNumber(item.getRunNumber())
    display.setTime(item.getElapsedTime())      # I think fractional times are ok here too.

def runStarted(item : daqformat.statechangeitem, display: ScalerGui.ScalerDisplay) -> None:
    state_change(item, 'Active', display)
    
def runEnded(item : daqformat.statechangeitem, display: ScalerGui.ScalerDisplay) -> None:
    state_change(item, 'Halted', display)

def runPaused(item : daqformat.statechangeitem, display: ScalerGui.ScalerDisplay) -> None:
    state_change(item, 'Paused', display)

def runResumed(item : daqformat.statechangeitem, display: ScalerGui.ScalerDisplay) -> None:
    state_change(item, 'Active', display)

#
#  Update all the displays associated with the counters in a data source
#  from the current internal data:
#
#  name - name of the data source to update.
#  scalers - the current scalers and rate values.
#  display - The scaler display widget.
#

def updateDisplay(name : str, scalers: dict, display, ScalerGui.ScalerDisplay) -> None:
    pass

# new scaler data; upate our internal counters
# and the part of the display that this source
# has any influence over.  Note the display update
# of ratios across two data sources will look a bit odd,
# as first one will update then the other leaving the ratio
# Not well computed until both upates.
# 
#  name - data source name.
#  item - ringitem with scaler data.
#  configuration - parsed configuration file.
#  display- the scaler display widget.
#  scalers - our internal counters and rates.
#
def updateCounters(
        name: str, item: daqformat.ringitem,
        configuration: configfile.Configuration, display: ScalerGui.ScalerDisplay, 
        scalers: dict
    ) -> None:
    display.setTime(item.endTime())     # Update the run time.
    display.setRunState('Active')       # In case we were added after the run started.
    interval = item.endTime() - item.startTime() # for rate computations.
    
    counters = item.getScalers()       # Ok we have all the counters.
    sourceinfo = configuration.datasources[name]
    for index, scaler_name in sourceinfo['scalers']:
        rate = float(counters[index])/interval       # Rate of that scaler.
        scalers[name][scaler_name][0] = counters[index]
        scalers[name][scaler_name][1] = rate

    # Our scalers and their rates are now fully updated.
    
    updateDisplay(name, scalers, display)

#   name - name of that source.
#   item - the ring item we got.
#   configuration - our configuration.
#   display  - the scaler display widget.
#   scalers  - our cumulative counters and rates.
# Note the scaler names in the scalers hashe are not fully qualified.
def update(
        name: str, item: daqformat.ringitem,
        configuration: configfile.Configuration, display: ScalerGui.ScalerDisplay, 
        scalers: dict
    ) -> None:
    # We care about state transitions, and scalers...what did we get:
    
    match item.type():
        case daqformat.BEGIN_RUN:
            runStarted(item, display)
        case daqformat.END_RUN:
            runEnded(item, display)
        case daqformat.PAUSE_RUN:
            runPaused(item, display)
        case daqformat.RESUME_RUN:
            runResumed(item, display)
        case daqformat.PERIODIC_SCALERS | daqformat.INCREMENTAL_SCALERS | daqformat. TIMESTAMPED_NONINCR_SCALERS:
            updateCounters(name, item, configuration, display, scalers)


##
#  sourceExited
#     Called when one of the data sources failed.  We popup
#     a dialog giving the user the opportunity to exit or
#     continue with the reduced set of sources.
#   name - the name of the source that exited.
#
def sourceExited(name : str) -> None:
    answer = QMessageBox.question(None, f'Data source {name} exited continue?')
    if answer == QMessageBox.No:
        exit(-1)
        
        
##
#  entry point.
def main() -> None:
    # There must be exactly one parameter, the configuration file
    # and it must exist:
    
    if len (argv) != 2:
        usage()
        exit(-1)
    
    # 
    # Process the configuration file:
    #
    with open(argv[1], 'r') as f:
        config_text = f.read()
    configuration = configfile.Configuration(config_text)
    warnings = configuration.check()
    if len(warnings) > 0:
        print('The configuration file had warnings:', file=stderr)
        for warning in warnings:
            print(warning, file=stderr)
        print(
            "For now ignoring but you might want to fix them for next time", 
            file=stderr
        )
    
    #  Set up the UI.
    
    application = QApplication(argv)
    main_window = QMainWindow()
    
    
    display     = ScalerGui.ScalerDisplay(main_window)
    configure_display(display, configuration)
    main_window.setCentralWidget(display)
    
    
    
    # Set up the data structures needed for the updates.
    # We'll just keep counters for each scaler for each
    # Data source.  We can then use the configu pages
    # to drive the updates:
    #  We'll make a dict keyed by source name whose
    #  elements are dict keyed by scaler name
    #  and contents of those dicts are an array of
    #  totals and rates.
    
    sources = configuration.datasources()
    scalers = dict()
    for source in sources:
        name = source['name']
        source_scalers = dict()
        for scaler in source['scalers']:
            source_scalers[scaler] = [0, 0.0]  # Totals, rates
        scalers[name] = source_scalers
        
    
    
    # Set up the data sources.
    # For each data sourcde we add it to a data source manager.
    # 
    # We connect the newData signal to our update slot
    # and the sourceExit to sourceExited slot.
    #
    #  We use the lambda trick to pass additional
    #  parameters we need to those slots.
    
    source_mgr = source_manager.DataSourceManager(display)
    for source in sources:
        source_mgr.addSourcde(source['name'], source['url'], format=source['version'])
    
    source_mgr.newData.connect(
        lambda name, item: update(name, item, configuration, display, scalers)
    )
    source_mgr.sourceExited.connect(sourceExited)
        
    
    # Run the QtApp.
    main_window.show()
    # Set the size to 850 x 700 which works on WSL/Windows.
    # Though Y size needs some thought.
    main_window.resize(850, 700)
    
    exit(application.exec())
    
    

if __name__ == '__main__':
    # Note that doing things this way supports unit testing.
    
    main()
