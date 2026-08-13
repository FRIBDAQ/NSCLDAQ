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


import sys
from enum import Enum
from collections import namedtuple
from nscldaq.mg_configutils import OkDialog
from PyQt6.QtCore import QObject, QTimer, pyqtSignal, Qt
from PyQt6.QtWidgets import (
    QApplication,
    QFileDialog,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QPushButton,
    QRadioButton,
    QSpinBox,
    QTableWidget,
    QTableWidgetItem,
    QVBoxLayout,
    QWidget,
)


constants = namedtuple('constants', ['MEGABYTE',])
Constants = constants(MEGABYTE = 1024*1024)
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
            self._table.setItem(row, 1, QTableWidgetItem(str(mb)))
        
        # Update the total:
        
        self._totalSize.setText(str(total))
        
        
        # Now we need to remove rows whose files are gone (not in names).
        # We work from the back so we don't need to worry about adjusting
        # row numbers:
        
        for row in reversed(range(self._table.rowCount())):
            name = self._table.item(row, 0).text()
            if name not in names:
                self._table.removeRow(row)
        
        
if __name__ == "__main__":

    files = [['sid_1', 100* Constants.MEGABYTE], 
             ['sid_2', 50*Constants.MEGABYTE],
             ['sid_3', 25*Constants.MEGABYTE]
        ]    
    findex = 1
    
    def tick():
        global files
        global findex
        
        filesToSet = files[0:findex]
        progress.setFiles(filesToSet)
        
        findex += 1

        # So it's dynamically changing.
        
        for t in files:
            t[1] += Constants.MEGABYTE
        
        print(progress.files())
    
    app = QApplication(sys.argv)
    w   = ReGlomConfiguration()
   
    w.show()
    w.exec()
    
    # Dump the configuration to stdout:
    wa = w.workarea()
    print('Dt: ', wa.dt())
    print('sourceid ', wa.sourceid())
    print('policy', wa.tspolicy().name)
    print('infile', wa.infile())
    print('outfile', wa.outfile())
    
    progress = UnglomFiles()
    progress.show()
    
    # Set up a timer to run the update of the table:
    
    t = QTimer()
    t.setInterval(2000)    # Couple of seconds...
    t.setSingleShot(False)
    
    t.timeout.connect(tick)
    t.start()
    
    sys.exit(app.exec())
    
