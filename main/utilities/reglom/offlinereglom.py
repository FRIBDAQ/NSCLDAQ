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

'''
@file offlinereglom.py
@brief Drive offline regloming of event built event files.
@author Ron Fox
@note  Replacement for offlinereglom.tcl - Issue #510
'''


import glob
import os
import pathlib
import sys
from collections import namedtuple
from enum import Enum

import parse
from nscldaq.mg_configutils import OkDialog
from nscldaq.OutputWindow import OutputWindow
from PyQt6.QtCore import QObject, QProcess, Qt, QTimer, QUrl, pyqtSignal
from PyQt6.QtWidgets import (
    QApplication,
    QDialog,
    QFileDialog,
    QFrame,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QMessageBox,
    QPushButton,
    QRadioButton,
    QSpinBox,
    QTableWidget,
    QTableWidgetItem,
    QVBoxLayout,
    QWidget,
)

constants = namedtuple('constants', ['MEGABYTE', 'POLL_INTERVAL'])
Constants = constants(
    MEGABYTE = 1024*1024, 
    POLL_INTERVAL = 2000
)
class TimestampPolicy(Enum):
    earliest = 1
    latest   = 2
    average  = 3

class TsPolicySelector(QWidget):
    ''' 
    Select a timestamp policy:
        Attributes:
            policy - one of values of the TimestampPolicy enum.
        Signals:
            changed - the policy changed.
    '''
    changed = pyqtSignal()
    
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
        self._layout = QHBoxLayout()
        self.setLayout(self._layout)
        
        self._layout.addWidget(QLabel('Timestamp Policy:', self))
        
        self._earliest = QRadioButton('Earliest', self)
        self._layout.addWidget(self._earliest)

        self._latest   = QRadioButton('Latest', self)
        self._latest.setChecked(True)   # Default to latest.
        self._layout.addWidget(self._latest)
        
        self._average = QRadioButton ('Average', self)
        self._layout.addWidget(self._average)
        
        
        #  Route the button clicked signals to the common changed signal:
        
        self._earliest.clicked.connect(self.changed)
        self._latest.clicked.connect(self.changed)
        self._average.clicked.connect(self.changed)
        
    #  Attribute implementation:

    def policy(self) -> TimestampPolicy:
        if self._earliest.isChecked():
            return TimestampPolicy.earliest
        elif self._latest.isChecked():
            return TimestampPolicy.latest
        elif self._average.isChecked():
            return TimestampPolicy.average
        else:
            raise RuntimeError('No valid timestamp policy is checked!!')
    def setPolicy(self, policy: TimestampPolicy) -> None:
        TimestampPolicy(policy)     # Throws an exception for bad value.
        match policy:
            case TimestampPolicy.earliest:
                widget = self._earliest
            case TimestampPolicy.latest:
                widget = self._latest
            case TimestampPolicy.average:
                widget = self._average
        
        widget.setChecked(True)
    
class ReGlomControls(QWidget):
    '''
    The actual full reglom controls.
    
    Attributes:
      dt   - Glom assembly window.
      sourceid - Output Source ID.
      tspolicy - Timestamp policy.
      infile   - Input file template (segment e.g)
      outfile  - Output file.
      
    '''
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
        # The widget is a bunch of horizontal strips:
        
        self._layout = QVBoxLayout()
        self.setLayout(self._layout)
        
        #  The Glom timee window and sid:
        
        glomlayout = QHBoxLayout()
        
        glomlayout.addWidget(QLabel('Glom interval (dt)', self))
        self._dt = QSpinBox(self)
        self._dt.setMinimum(1)
        self._dt.setMaximum(0x7fffffff)    # Maxint for i32
        glomlayout.addWidget(self._dt)
        
        glomlayout.addWidget(QLabel('Ouput SID', self))
        self._sid = QSpinBox(self)
        self._sid.setMinimum(0)
        self._sid.setMaximum(0x7fffffff)
        glomlayout.addWidget(self._sid)
        
        self._layout.addLayout(glomlayout)
        
        # Timestamp policy:
        
        self._tsPolicy = TsPolicySelector(self)
        self._layout.addWidget(self._tsPolicy)
        
        # Input file:
        
        infile = QHBoxLayout()
        infile.addWidget(QLabel('Input File:', self))
        self._infile = QLineEdit(self)
        infile.addWidget(self._infile)
        self._browseinfile = QPushButton('Browse...')
        self._browseinfile.clicked.connect(self._browseInputFile)
        infile.addWidget(self._browseinfile)
        
        self._layout.addLayout(infile)
        self._layout.addWidget(QLabel('For multisegment event file choose any segment', self))
    
        # Outfile:
        
        outfile = QHBoxLayout()
        outfile.addWidget(QLabel('Output File', self))
        self._outfile = QLineEdit(self)
        outfile.addWidget(self._outfile)
        self._browseoutfile = QPushButton('Browse...', self)
        self._browseoutfile.clicked.connect(self._browseOutputFile)
        outfile.addWidget(self._browseoutfile)
        
        self._layout.addLayout(outfile)
        self._layout.addWidget(QLabel('Output files are not segmented', self))
   
    # Implement attributes:
    
    def dt(self) -> int:
        ''' @return int - the ticks in the Glom build interval'''
        return self._dt.value()
    def setDt(self, dt : int) -> None:
        ''' @param dt : int - the new value for the glom dt.'''    
        self._dt.setValue(dt)
        
    def sourceid(self) -> int:
        ''' @return int - Current source id value'''
        return self._sid.value()
    def setSourceid(self, sid : int) -> None:
        ''' @param sid : int - new value of the sourcde id.'''
        self._sid.setValue(sid)
    
    def tspolicy(self) -> TimestampPolicy:
        '''  @return TimestampPolicy - The currently selected timestamp policy.'''
        return self._tsPolicy.policy()
    def setTsPolicy(self, policy : TimestampPolicy) -> None:
        self._tsPolicy.setPolicy(policy)
        
    def infile(self) -> str:
        ''' @return str - the input file selected. '''
        return self._infile.text()
    def setInfile(self, file : str) -> None:
        ''' @param file : str - new input file.'''
        self._infile.setText(file)
        
    def outfile(self) -> str:
        ''' @return str: Output file selected. '''
        return self._outfile.text()
    def setOutfile(self, file : str) -> None:
        ''' @param file : str - New output file path.'''
        self._outfile.setText(file)
    #  Internal slots: 
    
    def _browseInputFile(self) -> None:
        file, _ = QFileDialog.getOpenFileName(
            self, 'Input File (segment)', '.', 'Event Files (*.evt);; All Files (*)', '*.evt'
        )
        if file.strip():
            self._infile.setText(file)
    
    def _browseOutputFile(self) -> None:
        file, _ = QFileDialog.getSaveFileName(
                    self, 'Reglommed File', '.', 'Event Files (*.evt);; All Files (*)', '*.evt'
                )
        if file.strip():
            self._outfile.setText(file)

class ReGlomConfiguration(OkDialog):
    def __init__(self, parent : QObject | None = None):
        super().__init__(ReGlomControls(), parent)    
 # tests for now:
 
class UnglomFiles(QWidget):
    '''
        This is a widget that will display the status of the unglom
        part of the reglom operation.  The top of this
        widget is a QTableWidget which is a two column table containing
        the names of the unglom files and their sizes in MB.
        
        The bottom contains a total size of the unglom files in MB.
        
        Attributes:
        
        files  - Contains a list of 2 item tuples containing the filenames and sizes
                e.g.:  [(sid_1, 1234), (sid_2, 5555)]  where sizes are in bytes to make thing
                simpler for the caller.
                calling setFiles also updates the total field which cannot be retrieved or set.
    '''
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
        self._layout = QVBoxLayout()  # Stack the table on top of the labels:
        self.setLayout(self._layout)
        
        
        self._table = QTableWidget(self)
        self._table.setColumnCount(2)
        self._table.setHorizontalHeaderLabels(['File', 'Size (MB)'])
        self._table.setShowGrid(True)
        self._layout.addWidget(self._table)
        
        # Below the table the total size is just a pair of labels:
        
        totalLayout = QHBoxLayout()
        totalLayout.addWidget(QLabel('Total Size (MB):', self))
        self._totalSize = QLabel('0.0', self)
        totalLayout.addWidget(self._totalSize)
        
        self._layout.addLayout(totalLayout)
        
    # Implement attributes:
    
    def files(self) -> list[tuple[str, int]]:
        '''
            @return list[tuple[str, int]] - 
        '''
        result = []
        for row in range(self._table.rowCount()):
            filename = self._table.item(row, 0).text()
            size     = int(float(self._table.item(row, 1).text()) * Constants.MEGABYTE)
            result.append((filename, size))
        return result

    def setFiles(self, fileInfo : list[tuple[str, int]]) -> None:
        '''
        @param fileInfo a list of tuples where:
            [0]  - is a filename.
            [1]  - is its size in integer bytes.
        '''
        total = 0.0
        names = []
        for (name, size ) in fileInfo:
            names.append(name)        # So we know if we need to delete rows.
            # If the file is in the table, just update its size otherwise
            # add a row:
            matches = self._table.findItems(name, Qt.MatchFlag.MatchExactly)
            if matches:
                row = matches[0].row()
            else:
                self._table.setRowCount(self._table.rowCount() + 1)   # Add a row:
                row = self._table.rowCount() - 1   # Rows number from 0.
                self._table.setItem(row, 0, QTableWidgetItem(name))
            # Common code to set the count:
            
            mb = float(size)/Constants.MEGABYTE
            total += mb
            self._table.setItem(row, 1, QTableWidgetItem(f'{mb:.2f}'))
        
        # Update the total:
        
        self._totalSize.setText(f'{total:.2f} MB')
        
        
        # Now we need to remove rows whose files are gone (not in names).
        # We work from the back so we don't need to worry about adjusting
        # row numbers:
        
        for row in reversed(range(self._table.rowCount())):
            name = self._table.item(row, 0).text()
            if name not in names:
                self._table.removeRow(row)
        
class OutputProgress(QWidget):
    '''
    Widget to show the progress of generating the output file.
    Attributes:
     name - filename.
     size - size in bytes. Note that it will be displayed as  mbytes.
    '''
    
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        self._layout = QHBoxLayout()     # Side by side labels.
        self.setLayout(self._layout)
        
        self._layout.addWidget(QLabel('Output progress', self))
        self._filename = QLabel('                ', self)
        self._layout.addWidget(self._filename)
        
        self._size = QLabel('0.0 MB', self)
        self._layout.addWidget(self._size)
    
    # Implement the attributes:
    
    def name(self) -> str:
        ''' @return str - the name of the file displayed'''
        
        return self._filename.text()

    def setName(self, name : str) -> None:
        ''' @param name : str - name of the file to set in the label'''
        
        self._filename.setText(name)
        
    def size(self) -> int:
        ''' @return int # bytes equivalent to the MB displayed'''
        
        mb = float(self._sizes.text())
        bytes = mb * Constants.MEGABYTE
        return int(bytes)
    
    def setSize(self, size : int) -> None:
        ''' @param size : int - size of the file in bytes, 
            converted to MB and displayed'''    
            
        mb = float(size)/Constants.MEGABYTE
        self._size.setText(f'{mb:.2f}')   # Two digits of precision.



class MonitorReglom(QWidget):
    '''
      Thisi s just a megawidget with the UnglomFiles and
      OutputProgress widget along with an OutputWindow to show the stdout/stderr 
      of programs.
    '''
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
        # Put the progress widgets side by side and the
        # output window below both.
        
        self._layout = QVBoxLayout()
        self.setLayout(self._layout)
        
        self._layout.addWidget(QLabel('Progress', self))
        
        # Progress widgets:
        
        progressLayout = QHBoxLayout()
        self._unglom = UnglomFiles(self)
        progressLayout.addWidget(self._unglom)
        vline = QFrame(self)
        vline.setFrameShape(QFrame.Shape.VLine)
        vline.setFrameShadow(QFrame.Shadow.Sunken)
        progressLayout.addWidget(vline)
        self._glom   = OutputProgress(self)
        progressLayout.addWidget(self._glom)
        
        self._layout.addLayout(progressLayout)
        
        line = QFrame(self)
        line.setFrameShape(QFrame.Shape.HLine)
        line.setFrameShadow(QFrame.Shadow.Sunken)
        
        self._layout.addWidget(line)
        
        self._layout.addWidget(QLabel('Program Output', self))
        
        self._output = OutputWindow(self)
        self._layout.addWidget(self._output)
        
    
    def addOutput(self, output : str) -> None:
        '''
            Add output to the output window.
            @param output : str - the outut to add to the window.
        '''
        self._output.append(output)
    
    def updateSidFiles(self, info : list[tuple[str, int]])  -> None:
        '''
            Update the unglom files status part of the widget.
            @param info - List of file informatino suitable for its
                    setFiles method
        '''
        self._unglom.setFiles(info)
        
    def setOutFileInfo(self, name: str, size: int) -> None:
        '''
        Update the output file information.
        @param name - name of the output file.
        @param size - file size.
        
        '''
        self._glom.setName(name)
        self._glom.setSize(size)
        

def getFileList(eventFile : str) -> list[str]:
    # Return the actual file list.  If the name of the file
    # is of the form run-nnnn-mm.evt then this could be a segment
    # of a multisegment event file and we need to get all segments.
    
    result = parse.parse("run-{}-{}.{}", eventFile)
    if not result :
        # Just a single file not in that format:
        return [eventFile,]
    else:
        pattern = 'run-' + result[0] + '-*.'+result[2]  # Replace segment# with *
        files = glob.glob(pattern)
    
        return files.sort()        # To preserve segment order.

#--------------------------------------------------------------------------------------------------------
# Regloming methods:

def  reportDone(code : int, status : QProcess.ExitStatus, monitor: MonitorReglom) -> None:
    monitor.addOutput(f"reglom completed with code {code} status {status}")
    monitor.addOutput("Removing temporary files:\n")
    
    for f in glob.glob('sid-*'):
        os.remove(f)
        monitor.addOutput(f"Removed {f}")
    # Just returning leaves the UI up.

def conditionallyStartReglom(
    code : int, status : QProcess.ExitStatus,
    monitor : MonitorReglom, dt : int, sid: int, tsPolicy: TimestampPolicy, outfile: str) -> None:
    
    monitor.addOutput(f"Unglom exited with code {code}, status {status}")
    
    # For this we don't need the bash trick, and we already know $DAQBIN is defined.
    
    program = pathlib.Path(os.environ['DAQBIN']) / 'reglom'
    
    options = [
        f'--dt={dt}',
        f'--timestamp-policy={tsPolicy.name}',
        f'--sourceid={sid}',
        f'--output={outfile}'
    ]
    for infile in glob.glob('sid-*'):
        full_path = pathlib.Path(pathlib.Path.cwd())/infile
        options.append(QUrl.fromLocalFile(str(full_path)).toString())
    
    process = QProcess(monitor)      #  Making mnonitor the parent prevent del (I hope).
    process.setProcessChannelMode(QProcess.ProcessChannelMode.MergedChannels)    # Both stdout and stderr are the process.
    process.readyReadStandardError.connect(lambda : readProcess(process, monitor))    # Not sure I need this....
    process.readyReadStandardOutput.connect(lambda : readProcess(process, monitor))
    process.finished.connect(lambda code, status: reportDone(code, status, monitor))
    
    
    process.start(str(program), options)
    
#-------------------------------------------------------------------------------------------------------
# Unglomging methods.

def readProcess(process :QProcess, monitor : MonitorReglom) -> None:
    while process.canReadLine():
        lineBytes = process.readLine()
        monitor.addOutput(lineBytes.data().decode('utf-8'))

def monitorFiles(monitor : MonitorReglom, outfile :str):
    # Monitor the sid_* files in the wd and 
    # outfile as well:  This is called periodically from a timer.
    #
    source_files = glob.glob('sid-*')
    fileinfo = []
    for file in source_files:
        fpath = pathlib.Path(file)
        size  = fpath.stat().st_size
        fileinfo.append((file, size))
    
    monitor.updateSidFiles(fileinfo)
    
    outPath = pathlib.Path(outfile)
    if outPath.is_file():
        # It exists:
        
        monitor.setOutFileInfo(str(outPath), outPath.stat().st_size)
    
    

#
#  Start the Unglom and monitor it:
#  monitor - the status monitor object.
#  files   - The list of input files.
#  dt      - The dt for glom.
#  sid     - output sourceid.
#  tsPolicy- The policy to assign timestamps to the output items.
#  outfile - The output file for the reglommed data:
#
# Note that we start the unglom and monitor of the output sid files.
# but we setup a handler for that process exit that can start the glom itself.
#
def startUnGlom(monitor : MonitorReglom, files : list[str], dt : int, sid : int, tsPolicy : TimestampPolicy, outfile : str) -> None:
    
    if 'DAQBIN' not in os.environ:
        print('A Version of NSCLDAQ must be setup to run this (daqsetup.bash script sourced)', file=sys.stderr)
        exit(-1)
    dir = os.environ['DAQBIN']
    program = pathlib.Path(dir) / 'Unglom'
    
    
    process = QProcess(monitor)
    process.setProcessChannelMode(QProcess.ProcessChannelMode.MergedChannels)    # Both stdout and stderr are the process.
    process.readyReadStandardError.connect(lambda : readProcess(process, monitor))    # Not sure I need this....
    process.readyReadStandardOutput.connect(lambda : readProcess(process, monitor))
    
    # Also need a timer to monitor the sizes of the sid_* files
    
    timer = QTimer(monitor)                 # Making it owned by monitor I think prevents deletion.
    timer.setSingleShot(False)
    timer.setInterval(Constants.POLL_INTERVAL)
    timer.timeout.connect(lambda : monitorFiles(monitor, outfile))
    timer.start()
    
    # Set up on completion to start the next step:
    process.finished.connect(lambda code, status: conditionallyStartReglom(code, status, monitor, dt, sid, tsPolicy, outfile))
    
    # We want to start up bash so we have piping..so construct the args. 
    
    args = ["-c",]    # Cat the files to 
    cargs = ['cat']
    cargs.extend(files)
    cargs.append('|')
    cargs.append(str(program))  # Unglom.
    cargs.append('-')
    args.append(' '.join(cargs))
                  
    process.start("/bin/bash", args)
    return process
    
#
#  Entry point:
#  pop up the ReGlomConfiguration dialog, the do the unglom/reglom
#  With the status stuff following. End with a message box to prompt the
# exit.
def main() -> int:
    app = QApplication(sys.argv)
    
    #  Prompt for the reglom parameters:
    
    prompt = ReGlomConfiguration()
    prompt.show()
    while True:
        status = prompt.exec()
        if status == QDialog.DialogCode.Rejected:
            return 0                 # They don't want to go on
    
        # Get the parameters.. note that we need an input and output file.
        params = prompt.workarea()
        infile = params.infile()
        outfile = params.outfile()
        if not infile.strip() or not outfile.strip():
            QMessageBox.warning(
                prompt, 'Missing files', 
                'Both the input and output files must be specified!', 
                QMessageBox.StandardButton.Ok)
            continue                            # Try again.
        dt = params.dt()
        sid = params.sourceid()
        tsPolicy = params.tspolicy()
        break
    
    del prompt
    
    
    
    # Pop up our progress thingy and fire off the programs.
    
    monitor = MonitorReglom()
    monitor.show()
    
    files = getFileList(infile)     # Get the list of inputs...could be multiseg file.
        
    _unglom = startUnGlom(monitor, files, dt, sid, tsPolicy, outfile)   # Don't let the process get dropped.
    
    return app.exec()


if __name__ == "__main__":

    sys.exit(main())