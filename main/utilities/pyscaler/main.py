#!/usr/bin/env python3

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

# I think this has to come before ScalerPlot to ensure that
# matplotlib used in that selects the qt5 bindings not qt6.
# of course we could just flip our code to pyqt6.

from   PyQt5.QtWidgets import (QApplication, QMainWindow, QMessageBox, 
                               QWidget, QVBoxLayout)
from   PyQt5.QtCore    import QObject


import nscldaq.pyscaler.ScalerPlot as ScalerPlot
import daqformat

import time
import csv
import tabulate

import nscldaq.pyscaler.configfile as configfile
import nscldaq.pyscaler.ScalerGui  as ScalerGui
import nscldaq.pyscaler.source_manager as source_manager
import nscldaq.pyscaler.channel  as channel
from pathlib import  Path
import os








## 
# usage:
# Output program usage.
def usage() -> None:
    print("Usage:" , file=stderr)
    print(f'   {argv[0]}  configfile', file=stderr)
    print( 'Where', file=stderr)
    print('    configfile - is a pyScaler configuration file.', file=stderr)

##
#  Given a scaler name and source generate the qualified name:
#
def qualify_name(source: str, name: str) -> str :
    return f'{source}.{name}'

##
#  unqualify a name into a tpule containing the source and the rest of the
#  name.
def unqualify_name(fully_qualified : str) -> tuple[str, str] :
      split_name = fully_qualified.split('.')
      return (split_name[0], '.'.join(split_name[1:]))
# factor out making a plotline name for a ratio:
def make_ratioName(scalers):
    name = f'{scalers[0]}/{scalers[1]}'
    return name
##
#  clear_scalers
#     
def clear_scalers(scalers : dict) -> None:
    for srcname in scalers:
        for scalername in scalers[srcname]:
            scalers[srcname][scalername].clear()


##
#  write_csv
#    Write the end run CSV file:
#
#  Parameters:
#   output_basename - The output file (full path) without the .csv extension.
#   title      - the run title.
#   run_number - the run number.
#   start      - date/time string for the run start.
#   end        - date/time string for the run end.
#   duration   - Number of active seconds
#   scalers    - The dict of scaler dicts.
# 
def write_csv(
    output_basename : str,   title : str,  run_number : int,
    start           : str,   end   : str,  duration    : float,
    scalers : dict
) -> None:
    filename = f'{output_basename}.csv'
    with open(filename, 'w', newline='') as csvfile:
        csvwriter = csv.writer(csvfile)
        # Write the header line that identifies the run:
        
        csvwriter.writerow(
            [run_number, title, start, end,  duration]
        )
        # Now write the scaler lines with fully qualified names:
        
        for srcname in scalers:
            for sclname in scalers[srcname]:
                full_name = qualify_name(srcname, sclname)
                ch = scalers[srcname][sclname]
                csvwriter.writerow(
                    [full_name, ch.total(), ch.averageRate(), ch.rateStdDev()]
                )
            
        

##
# write_report
#    Same as write_csv but writes a human readable report file.
#
#
#  Parameters:
#   output_basename - The output file (full path) without the .csv extension.
#   title      - the run title.
#   run_number - the run number.
#   start      - date/time string for the run start.
#   end        - date/time string for the run end.
#   duration   - Number of active seconds
#   scalers    - The dict of scaler dicts.
# 
def write_report(
    output_basename : str,   title : str,  run_number : int,
    start           : str,   end   : str,  duration    : float,
    scalers : dict
) -> None:
    filename = f'{output_basename}.report'
    
    
    #  Figure out the duration string:
    
    secs = duration % 60
    minutes = int(duration/60)
    mins = minutes % 60
    hours = int(minutes/60)
    hrs   = hours %24
    days  = int(hours/24)
    
    time_string = f'{days} {hrs:02d}:{mins:02d}:{secs:02.2f}'
        
    with open(filename, 'w') as report :
        print(f'Run        : {run_number}', file=report)
        print(f'Title      : {title}', file=report)
        print(f'Started    : {start}', file=report)
        print(f'Ended      : {end}', file=report)
        print(f'Elapsed    : {time_string}', file=report)
        print("", file=report)
        
        # Compute the tabulated report and print that as well.
        
        report_data = []
        for srcname in scalers:
            for sclname in scalers[srcname]:
                full_name = qualify_name(srcname, sclname)
                ch = scalers[srcname][sclname]
                report_data.append(
                    [full_name, ch.total(), f'{ch.averageRate():.2f}', f'{ch.rateStdDev():.2f}']
                )
        
        formatted_report = tabulate.tabulate(
            report_data,
            headers = ['Name', 'Total', 'Average Rate', 'Rate Std Dev'],
            tablefmt='pipe'
        )
        print(formatted_report, file=report)
    
    
##
# write_endRun
#    Write scalers at the end of run.
#
# Parameters:
#   output_dir - Directory in which to write the files.
#   title      - the run title.
#   run_number - the run number.
#   start      - time since Epoch when the run started.
#   end        - time since the Epoch when the run ended.
#   duration   - Number of active seconds
#   scalers    - The dict of scaler dicts.
#
def write_endRun(
    output_dir  : str, title : str,
    run_number  : int, start : int, end : int, duration : float, 
    scalers     : dict
) -> None:
    
    # We need to have a valid start time to write the reports:
    
    if start is not None:
        # May as well stringify the times here:
        # And make the base filename with path:
        # (DRY). 
        start_str = time.ctime(start)
        end_str   = time.ctime(end)
        
        output_basename = f'{output_dir}/run{run_number:04d}'
        
        # We write two files:  A CSV file in case someone wants to
        # process with a program (e.g. Excel) and a human readable nice
        # report file.
        
        write_csv(output_basename, title, run_number, start_str, end_str, duration, scalers)
        write_report(output_basename, title, run_number, start_str, end_str, duration, scalers)

##
# write_plots  
#   Save the plots to file:
# path - directory in which to write the plot file.
# run  - run number.
# plots- Plot object.
#
def write_plots(path: str, run : int, plots : ScalerPlot.ScalerStripChart)   -> None:
    filename = f'{path}/run{run:04d}.png'    # Png format.
    
    # Note the matplolib tosses an exception if the file already exists 
    # so destroy it here if that's the case:
    fpath  = Path(filename)
    if fpath.is_file():
        os.remove(filename)
        
    plots.save(filename)
##
#  configure_display
#
#  Configure the tabs etc. in the GUI:
# Parameters:
#    display - the ScalerDisplay widget to configure.
#    configuration - the processed configuration.
#
def configure_display(display : ScalerGui.ScalerDisplay, configuration: configfile.Configuration) -> None :
    #
    # Add the notebook pages:
    
    pages = configuration.pages()
    for page in pages:
        display.addPage(page)

# factor out making a plotline name for a ratio:
def make_ratioName(scalers):
    name = f'{scalers[0]}/{scalers[1]}'
    return name
##
# configure_stripcharts
#    Add the plotlines configured in the config file:
#  plotter - the ScalerStripChart widget.
#  config  - The result of the plots() call on a configuration.
#
# Note the caller must ensure that the config isn ot empty.
#
def configure_stripcharts(plotter, config):
    #
    #  If requested, set the window and decimation threshold.
    #
    if 'window' in config.keys():
        plotter.setWindow(config['window'])
    if 'trim_to' in config.keys():
        plotter.setMaxPoints(config['trim_to'])
        
    # Add the plotlines for single scalers:
    
    for scaler in config['single']:              # The key is gauranteed.
        plotter.add_plotline(scaler)
        
    # Add the plotlines for scaler ratios.
    
    for scalers in config['ratio']:
        plotter.add_plotline(make_ratioName(scalers))
    
        
        
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

StartTime = None
def runStarted(item : daqformat.statechangeitem, display: ScalerGui.ScalerDisplay, scalers : dict) -> None:
    global StartTime
    state_change(item, 'Active', display)
    clear_scalers(scalers)
    StartTime = item.getTime()
    
# Plots is the plot widget, if any.
def runEnded(item : daqformat.statechangeitem, display: ScalerGui.ScalerDisplay, 
             config : configfile.Configuration,  scalers: dict, plots: ScalerPlot.ScalerStripChart) -> None:
    global StartTime
    state_change(item, 'Halted', display)
    write_endRun(
        config.output_path(), item.getTitle(), item.getRunNumber(), 
        StartTime, item.getTime(), item.getElapsedTime(),
                  scalers
    )
    if plots is not None:
        write_plots(config.output_path(), item.getRunNumber(), plots)

def runPaused(item : daqformat.statechangeitem, display: ScalerGui.ScalerDisplay) -> None:
    state_change(item, 'Paused', display)

def runResumed(item : daqformat.statechangeitem, display: ScalerGui.ScalerDisplay) -> None:
    state_change(item, 'Active', display)


#
#  Update all the displays associated with the counters in a data source
#  from the current internal data:
#
#  scalers - the current scalers and rate values.
#  display - The scaler display widget.
#
#  Note the scaler names in scalers are not qualified by the source name.
# 
#    If that ever becomes a performance problem...we can fix that later.
def updateDisplay(scalers: dict, display: ScalerGui.ScalerDisplay) -> None:

    for page in display.pageNames():
        definition = display.lineDefinition(page)
        model      = display.lineModel(page)
        
        # Note the scalers here are fully qualified.
        
        for line_def in definition['lines']:
            line_no = line_def['number'] - 1 # Zero based in model.
            type = line_def['type']
            # Every non empty line has a first scaler and its rates:
            if type != 'empty':
                first_name = line_def['scalers'][0]
                (source, uqname) = unqualify_name(first_name)
                counts = [scalers[source][uqname].total(), ]
                rates  = [scalers[source][uqname].rate(), ]
                    
                
                #  pair and ratio have a second scaler name:
                
                if type in {'pair', 'ratio'} :
                    second_name = line_def['scalers'][1]
                    source = second_name.split('.')[0]
                    uqname = '.'.join(second_name.split('.')[1:])
                    counts.append(scalers[source][uqname].total())
                    rates.append(scalers[source][uqname].rate())
                
                # Update the line:

                model.update_line(line_no, counts, rates)
            
            

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
    
    counters = item.getScalers()       # Ok we have all the counters.
    sources = configuration.datasources()
    
    # fInd the named source:
    
    sourceinfo = None
    for s in sources:
        if s['name'] == name:
            sourceinfo = s
            break
    if sourceinfo is None:
        raise AssertionError(f'**BUG** No such data source {name} in {sources} report to DAQ software group, provide your configuration file too please.')
    
    for index, scaler_name in enumerate(sourceinfo['scalers']):
        scalers[name][scaler_name].update(item.startTime(), item.endTime(), counters[index])


    # Our scalers and their rates are now fully updated.
    
    

##
#  Look at all of the scalers that have alamrs configured and 
#  Colorize the appropriate cells and tabs in the display.
#
#  @param colors - a dict keyed by the alarm type 
#                 whose value are the colors for that alarm.
#                 See configfile.Configuration.alarm_colors().
#  @param scalers - The dict of counters.
#  @param display - The tabbed notebook containing all displays.
#           
def updateAlarms(colors : dict, scalers : dict, display : ScalerGui.ScalerDisplay) -> None:
    for page in display.pageNames():
        definition = display.lineDefinition(page)
        model      = display.lineModel(page)
        alarms = set()                            # So we can figure out the tab color:
        for line_def in definition['lines']:
            line_no = line_def['number'] - 1
            type = line_def['type']
            if type != 'empty' :
                name = line_def['scalers'][0]
                (source, uqname) = unqualify_name(name)
                
                # is this scaler alarmed:
                
                if scalers[source][uqname].isLowAlarm():
                    color = colors['lowalarm']
                    alarms.add('low')
                elif scalers[source][uqname].isHighAlarm():
                    color = colors['highalarm']
                    alarms.add('high')
                else:
                    color = colors['noalarm']
            
                model.set_line_color(line_no, 1, color)
                
                # If there's a second scaler do it too:
                
                if type in ('pair', 'ratio'):
                    name = line_def['scalers'][1]
                    (source, uqname) = unqualify_name(name)
                
                    # is this scaler alarmed:
                    
                    if scalers[source][uqname].isLowAlarm():
                        alarms.add('low')
                        color = colors['lowalarm']
                    elif scalers[source][uqname].isHighAlarm():
                        alarms.add('high')
                        color = colors['highalarm']
                    else:
                        color = colors['noalarm']
                    model.set_line_color(line_no, 2, color)
        # Now set the page's tab color depending on the alarms:
        
        tab_color = colors['noalarm']
        if len(alarms) == 2:
            tab_color = colors['bothalarms']
        elif 'low' in alarms:
            tab_color = colors['lowalarm']
        elif 'high' in alarms:
            tab_color = colors['highalarm']
        display.setTabTextColor(page, tab_color)

##
# Update the strip charts... we've determined there are some:
#
#  name          - Name of the datasource that updated.
#  time          - End time for the scaler readout (used to tag the point times).
#  configuration - The plot configuration.
#  scalers       - The map of scale robjects  after they've been updated.
#  plots         _ the ScalerPlot widget in which to draw the strip charts.
#
# Note the scalers in the configuration are fully qualified names.
#

def updateStripCharts(name, time, configuration, scalers, plots):
    t = float(time)
    # Singles:
    # Note, here we only need to do something for scalers in our data source.
    
    for scaler in configuration['single']:
        source, uqname = unqualify_name(scaler)
        if source == name:
            rate = scalers[source][uqname].rate()
            plots.add_point(scaler, t, rate)
    #
    # Since the ratios may span boundaries,
    # We just update all of them though that might make the rates
    # look a bit spikey.
    
    for scaler_pair in configuration['ratio']:
        plot_name = make_ratioName(scaler_pair)
        numerator, denominator = scaler_pair
        numsource, numname = unqualify_name(numerator)        
        densource, denname = unqualify_name(denominator)
        
        ratio = scalers[numsource][numname].rate()/scalers[densource][denname].rate()
        plots.add_point(plot_name, t, ratio)
            
    plots.update()
##
#  update.
#   name - name of that source.
#   item - the ring item we got.
#   configuration - our configuration.
#   display  - the scaler display widget.
#   scalers  - our cumulative counters and rates.
#   plots    - The widget in which to display strip charts or "None'
#              if there aren't any.
# Note the scaler names in the scalers hashe are not fully qualified.
def update(
        name: str, item: daqformat.ringitem,
        configuration: configfile.Configuration, display: ScalerGui.ScalerDisplay, 
        scalers: dict, plots: ScalerPlot.ScalerStripChart
    ) -> None:
    # We care about state transitions, and scalers...what did we get:
    
    match item.type():
        case daqformat.BEGIN_RUN:
            runStarted(item, display, scalers)
            if plots is not None:
                plots.clear()                          # Clear any strip charts.
        case daqformat.END_RUN:
            runEnded(item, display, configuration, scalers, plots)
        case daqformat.PAUSE_RUN:
            runPaused(item, display)
        case daqformat.RESUME_RUN:
            runResumed(item, display)
        case daqformat.PERIODIC_SCALERS | daqformat.INCREMENTAL_SCALERS | daqformat. TIMESTAMPED_NONINCR_SCALERS:
            updateCounters(name, item, configuration, display, scalers)
            if plots is not None:
                updateStripCharts(name, item.endTime() , configuration.plots(), scalers, plots)

    # Set any alarm colors:
    
    updateAlarms(configuration.alarm_colors(), scalers, display)
    updateDisplay(scalers, display)
##
#  sourceExited
#     Called when one of the data sources failed.  We popup
#     a dialog giving the user the opportunity to exit or
#     continue with the reduced set of sources.
#   name - the name of the source that exited.
#
def sourceExited(name : str) -> None:
   
    answer = QMessageBox.question(None, 'Data Source Exited', f'Data source {name} exited continue?')
    if answer == QMessageBox.No:
        exit(-1)

## kill_sources
#  Called as the app is exiting, kill off all data sources:
#      
# manager - the source manager:
#

def kill_sources(manager) -> None:
    global source_exit_signal
    QObject.disconnect(source_exit_signal)  # Disconnect the signal handler.
    for name in manager.sourceNames():
        manager.killSource(name)

      
        
##
#  entry point.
def main() -> None:
    global source_exit_signal
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
    
    display           = QWidget(main_window)
    scaler_counts     = ScalerGui.ScalerDisplay(display)
    layout            = QVBoxLayout()
    layout.addWidget(scaler_counts)
    vsize = 700
    configured_plots = configuration.plots()
    #
    # Add the stipr chart if there's at least one thing to plot
    if len(configured_plots['single']) > 0 or len(configured_plots['ratio']) > 0:
        plots         = ScalerPlot.ScalerStripChart(display)
        layout.addWidget(plots)
        configure_stripcharts(plots, configured_plots)
        vsize += 300
    else:
        plots = None
    display.setLayout(layout)
    configure_display(scaler_counts, configuration)
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
    alarms = configuration.alarms()
    scalers = dict()
    for source in sources:
        name = source['name']
        isIncremental = source['incremental']   # So we make the right type of scaler.
        source_scalers = dict()
        for scaler in source['scalers']:
            source_scalers[scaler] =  channel.Channel(incremental=isIncremental)
            # Is the scaler alarmed:
            
            fqname = qualify_name(name, scaler)

            #  If alarms are set on the scaler, apply them here.
            
            if fqname in alarms.keys():
                source_scalers[scaler].setLowAlarm(alarms[fqname]['low'])
                source_scalers[scaler].setHighAlarm(alarms[fqname]['high'])
        scalers[name] = source_scalers
        
    
    
    # Set up the data sources.
    # For each data sourcde we add it to a data source manager.
    # 
    # We connect the newData signal to our update slot
    # and the sourceExit to sourceExited slot.
    #
    #  We use the lambda trick to pass additional
    #  parameters we need to those slots.
    
    source_mgr = source_manager.DataSourceManager(scaler_counts)
    for source in sources:
        source_mgr.addSource(source['name'], source['url'], format=source['version'])
    
    source_mgr.newData.connect(
        lambda name, item: update(name, item, configuration, scaler_counts, scalers, plots)
    )
    source_exit_signal = source_mgr.sourceExited.connect(sourceExited)
        
    
    # Run the QtApp.
    main_window.show()
    # Set the size to 850 x 700 which works on WSL/Windows.
    # Though Y size needs some thought.
    main_window.resize(850, vsize)
    application.aboutToQuit.connect(lambda: kill_sources(source_mgr))    
    exit(application.exec())


if __name__ == '__main__':
    # Note that doing things this way supports unit testing.
    
    main()
