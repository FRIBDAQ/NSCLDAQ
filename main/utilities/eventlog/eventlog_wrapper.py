#!/usr/bin/env python3

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

##
# @file   eventlog_wrapper.py
# @brief  Wraps eventlog instances for the manager.
# @author Ron Fox <fox@nscl.msu.edu>
#

'''
  This script wraps the event logger when its used by the program manager.
  The expectation is that:
   daqsetup has been run and therefore all env. variables relevant to that
   have been defined (specfically DAQBIN).
   In addition the following script variables have been defined and are in env:
   -   RECORD_PARTIAL - This is a boolean which is true if the events should just
                        be recorded into a directory that is a soup of event
                        files... like the 11.x multilogger e.g.  If false,
                        the event files will be managed as they were for
                        the 'normal' event file logger.

   -   RECORD_DEST   - Where the event files will be recorded.  If RECORD_PARTIAL
           is true, this is the directory in which event file soup is made.
           if not, this is the top level of the directory tree that
           holds the managed event files.
   -   RECORD_SRC  - the URL from which the ring items are logged.
   -   RUN_NUMBER  - expected run number.
   -   DAQBIN - must have been defined (e.g. by a daqsetup.bash).

 @note This script only records a single run.
'''
import os
import sys
import pathlib
import datetime
from PyQt5.QtCore import QProcess, QTimer, QIODevice
from PyQt5.QtWidgets import QApplication

##
# _eventlog_path
#   Return a string that points to the event logger
#   program (suitable for command)
#
# @return str
#
def _eventlog_path():
    global daqbin
    return str(pathlib.Path(daqbin, 'eventlog'))
    
#
#  Construct the filename for segment 0 of a run
#  Return a string:
#
def _event_filename(run) :
    return f'run-{run:04d}-00.evt'

# Get the run number from an event filename:

def _event_file_run(evt):
    return int(evt.split('-')[1])
## 
# _multilog
#   Start a multilogger.
#  
# @param  src - the URI of the ring buffer to log from.
# @param  dest - the diretory in which to log.
# @return QProcess that's running the event logger.
#
def _multilog(src, dest):
    global runNumber
    global SEGMENT_SIZE
    
    # Make sure the directory exists,  if it does not:
    
    os.makedirs(dest, exist_ok=True, mode=0o750)
    
    #  Figure out the prefix of the event filename.
    #  Used to uniquify it across identical run numbers:
    
    now = datetime.datetime.today()
    prefix = now.strftime('%d-%b-%y-%H:%M:%S-') + str(runNumber)
    
    command = _eventlog_path()
    command_args = [
        f'--source={src}', f'--path={dest}',
        f'--segmentsize={SEGMENT_SIZE}',
        f'--prefix={prefix}', '--oneshot'
    ]
    
    # Make the process, set it up and return it:
    
    process = QProcess()
    process.setProgram(command)
    process.setArguments(command_args)
    
    return process


##
# Finalize a run.
# This is called when the event logger exits, no matter the
# reason... thus we'll finalize runs if the event logger crashes too.
# Note that the finalization is only done if record_partial is false.
#
# Lots to do if not partial:
#   - remove the two symlinks we made in 
#   - Make a new link to the event file in {destination}/complete.
#   - Tar up everything in {destination}/experiment/current, dereferencing
#     links to the {destination}/experiment/run{run} directory.
#   - chmod -R  in {destination}/experiment/run{run} to 0550
#     to prevent accidental deletion of event data and 
#     associated data.
#       
#
def _finalize_run(destination, run):
    global partial
    if partial:
        return
    
    #  Ok not a partial run so we have to do stuff.
    
    filename = _event_filename(run)
    
    # Handle the links and stuff:
    
    pathlib.Path(destination, 'experiment', 'current', filename).unlink(missing_ok=True)
    pathlib.Path(destination, 'current', filename).unlink(missing_ok=True)
    file_path = pathlib.Path(destination, 'experiment', f'run{run}', filename)
    new_link  = pathlib.Path(destination, 'complete', filename)
    os.symlink(str(file_path), str(new_link))
    
    # the rest of the stuff is done by issuing a bunch of os.system operations
    # since it's simpler with shell commands:
    
    # Construct the tar command we ignore errors fromt he commands.
    
    tar_command = f'(cd {destination}/experiment/current; tar czf - --dereference .) | (cd {destination}/experiment/run{run}; tar xzpf - --warning=no-timestamp)'
    
    try:
        os.system(tar_command)
    except:
        pass

    # now the chmod we need to remove write access from the run directory contents and it.
    
    chmod_command = f'chmod -R u-w {destination}/experiment/run{run}'
    print("CHMOD: ", chmod_command)
    try:
        os.system(chmod_command)
    except:
        print("Exception")
        pass
##
# Wait for the actual event file to appear then
# Create the link to it in {destination}/experiment/current
# and {destination}/current.
# This is done by doing a check and, if we don't see it,
# scheduling QTimer to all us back after a second...
# continuing that nonensense until the file appears.
#
# destination - top level directory.
# run         - run number.
#
#  We expect destination/expermient/run{run}/run-{run}-00evt
# where the run number in the filename is zero filled on the left to at l
# least four digits (e.g. in C:  %04d format),
# We assume that there will only be one segment number because the
# segment size is so large.
#
def _schedule_link_in_current(destination, run):
    global link_timer
    filename = _event_filename(run)
    
    # now the path:
    
    path = pathlib.Path(destination, 'experiment', f'run{run}', filename)
    if path.is_file():
        #  The file exists, so make the two links below:
        link1 = pathlib.Path(destination, 'current', filename)
        link2 = pathlib.Path(destination, 'experiment', 'current', filename)
        os.symlink(str(path), str(link1))
        os.symlink(str(path), str(link2))
        
        # Done so don't reschedule.
        
    else:
        # The event file does not exits... reschedule the check
        #  FILE_POLL_INTERVAL seconds from now:
        link_timer = QTimer()
        link_timer.setInterval(FILE_POLL_INTERVAL * 1000)
        link_timer.setSingleShot(True)
        link_timer.timeout.connect(lambda :_schedule_link_in_current(destination, run))
        link_timer.start()
        pass
        
##
# Make the directory tree used by a full recording:
#    destination
#      +--- complete
#      +--- current
#      +--- experiment
#              +---- current.
#
#  destination is the top leve of that directory tree and is also
#  made if required.  It is not an error for bits or even all of this
#  tree to already exist.
#  The modes for the directory allow (depending on the umask rwxr-x---)
#  that is the user has full access, the group has read and traversal while
#  The rest of the world has no access.
#
#  These modes get modified at the end of the run by finalize.
#
def _make_directory_tree(destination):
    os.makedirs(destination, exist_ok=True, mode=0o750)
    os.makedirs(f'{destination}/complete', exist_ok=True, mode=0o750)
    os.makedirs(f'{destination}/current', exist_ok=True, mode=0o750)
    os.makedirs(f'{destination}/experiment', exist_ok=True, mode=0o750)
    os.makedirs(f'{destination}/experiment/current', exist_ok=True, mode=0o750)

##
#  If a previous run exited badly, there could be orphaned
#  links in the various current directories.
#  These links are removed and, if there is are event files
#  in the corresponding run diretories:
#   -  They are linked to the complete directory
#   -  a file named 'run_improperly_ended' is put in that directory.
#   -  The directory is protected.
#
# Note that since we don't know the related files in experiment/current 
# are actually relevant to this run we don't do the tar.
def _clean_orphans(destination):
    runs = []     # Will be orpaned run numbers
    current_paths = list(pathlib.Path(destination, 'current').glob('*.evt'))
    exp_paths     = list(pathlib.Path(destination, 'experiment', 'current').glob('*.evt'))
    
    #  Remove those links and construct a list of the unique run numbers:
    
    for path in set(current_paths) | set(exp_paths):
        run = _event_file_run(path.name)
        runs.append(run)
        path.unlink()    # remove the symlink.
        
    
        
    runs = list(set(runs))    #  Now unique in run numbers.
    
    for number in runs:
        filename = _event_filename(number)
        target   = pathlib.Path(destination, 'experiment', f'run{number}', filename)
        link     = pathlib.Path(destination, 'complete', filename)
        os.symlink(target, link)
        
        # Note thta we ignore failures to create the run_improperly_ended though
        # our finalize order should make it always possible.
        marker  = pathlib.Path(destination, 'experiment', f'run{number}', 'run_improperly_ended')
        try:
            marker.touch(exist_ok=True)
        except:
            print(f'Unable to create improper end marker {str(marker)}', file=sys.stderr)
        # Try to write protect the directory... if we can't we ignore the error too:
        
        try:
            os.system(f'chmod -R u-w {str(marker.parent)}')
        except:
            pass
##
# _onExit
#    The eventlog exited....
#
# @param exitcode - process exit code.
# @param status   - Qt exit status.

def _onExit(exitcode, status):
    global app
    global runNumber
    global destination
    print(f'Eventlog exited with code {exitcode}', file=sys.stderr)
    ourstatus = 0
    if status == QProcess.CrashExit:
        print('Abormal exit according to Qt', file=sys.stderr)
        ourstatus = -1
    _finalize_run(destination, runNumber)
    app.exit(ourstatus)          # Kill the app with this return code.



##
# _fulllog
#   Entry point for full logging.  This maintains the NSCLDAQ
#   event directory structures in the selected destination.
#   Note that We run the event log as a QProcess with input
#   and exit handlers.
#
# @param source URI of the ring buffer to point the event logger
#        at for data to log.
# @param destination top level directory of the directory tree we will
#        manage.
# @param run The run number we are expecting fromt he manager.f
# @note we point the event logger at {destination}/experiment/current
#      when it exits, we will move that to {destination}/experiment/run{run}
#      along with an associated data as well as  making an entry in {destination}/complete
#      linking back to the file.
# @return QProcess that's running the event logger.
#
def _fullylog(source, destination, run):
    global SEGMENT_SIZE
    
    destdir = pathlib.Path(destination, 'experiment', f'run{run}')  #event log dir.
    os.makedirs(str(destdir), exist_ok=False, mode=0o750)    # The run directory must not exist yet.
    command = _eventlog_path()
    command_args = [
        f'--source={source}', f'--path={destdir}', f'--segmentsize={SEGMENT_SIZE}',
        '--oneshot'
    ]
    # Make and run the proces..
    # We'll connect to the stdin,stderr ready to read and exit signals in the main code.
    # 
    
    process = QProcess()
    process.setProgram(str(command))
    process.setArguments(command_args)
    
    _schedule_link_in_current(destination, run)
    
    return process
    
#---------------------- Entry point 
# Note, the global definitions will be important for some utility methods defined 
# above.
           
# Doing things this way _might_? provide support for unit testing:

try:
    partial     = int(os.environ['RECORD_PARTIAL'])
    destination = os.environ['RECORD_DEST']
    source      = os.environ['RECORD_SRC']
    runNumber   = int(os.environ['RUN_NUMBER'])
    daqbin      = os.environ['DAQBIN']
except KeyError as e:
    print(f'The environment has not been properly set for eventlog_wrapper: {e}', file=sys.stderr)
    exit(-1)
    
# Some global definitions:

SEGMENT_SIZE="1000000g"    # Effectively unlimited segment size.
FILE_POLL_INTERVAL=1     # Seconds between polls for event file existence.
done        = False      # Set True when managed logging is finished.
app         = QApplication(sys.argv)
eventlog_process = None  # QProcess running the event logger
link_timer  = None       # Will be QTimer to poll for event file to exist.


if __name__ == '__main__':

    #   Figure out what the environment says we should do:


    if not partial:
        _make_directory_tree(destination)
        _clean_orphans(destination)


    if partial:
        eventlog_process = _multilog(source, destination)
    else:
        eventlog_process = _fullylog(source, destination, runNumber)

    # Connect slots to the eventlog process signals we care about
    #  

    eventlog_process.setProcessChannelMode(QProcess.ForwardedChannels)   # stderr/stdout forward to our output.
    eventlog_process.finished.connect(_onExit)                           # handle exit.

    eventlog_process.start(QIODevice.ReadOnly)   # Start the logger....

    # Run the event loop.

    sys.exit(app.exec())