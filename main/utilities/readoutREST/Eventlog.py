'''
    This file contains some model/view combinations for event loggin.
    In addition to doing an overall enable/disable, individual
    loggers can be enabled/disabled.   So we provide:
    
    LoggerConfig  - View for configuring loggers.
    LoggerConfigModel - Model for logger configuration 
    
    Logger        - View with a toggle for turning on logging.
    LoggerModel   - Containst the data for turning on logging.
    
    Note that controllers that link these together in some
    coherent way are assumed to be external to this file so that
    the things we actually control are not bound to the managed environment
    (necessarily).
    
    @file Eventlog.py
    @brief Models and views for controlling event loggers.
    @author Ron Fox
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
from typing import Self
from PyQt6.QtCore import pyqtSignal, QObject, Qt
from PyQt6.QtWidgets import QWidget, QTableWidget, QTableWidgetItem, QCheckBox


# Models.
class LoggerConfigModel(QObject):
    
    '''
        Provides the model for loggers.  For each logger, we'll store
        -  The ringbuffer (URI) from which the logging is being done.
        -  The host running thelogger.
        -  The destination to which the  logger is logging.
        -  Flag that's true if this as a partial logger.
        -  FLag that's true if the logger is enabled.
        -  Flag that's true if the logger is critical.
        
        Note that once a logger has been entered into the model, the only thing
        that can be modified is its enable state.  If the set of loggers is being
        monitored externally and maintained here in the presence of configuration, 
        the controller should just kill off and re-define loggers if they change in
        other ways.
        
        Attributes:
          - loggers (readonly)
        
        Methods:
           - addLogger - add a new logger.
           - deleteLogger - delete an existing logger.
           - enableLogger - turn a logger on.
           - disableLogger - disable a logger.
        Signals:
           loggerAdded(dict) - A new logger was added to the list.
           loggerDeleted(str) - A logger was deleted.
           loggerEnabled(str)  - A logger wsa enabled.
           loggerDisabled(str) -  A logger was disbaled.
           
        In the signals:
           str - is the destination of a logger.  That unambiguously identifies a logger.
           dict - is a dict with the following keys:
            *  ring - source of logger data.
            *  host - Host running the logger.
            *  destination - where the logger is recording data.
            *  partial - true if the logger is partial.
            *  enabled - True if the logger is enabled.
            *  critical - True if an unexpected logger exit shuts down the experiment.
            
        
          
    '''
    loggerAdded = pyqtSignal(dict)
    loggerDeleted = pyqtSignal(str)
    loggerEnabled = pyqtSignal(str)
    loggerDisabled = pyqtSignal(str);
    
    def __init__(self, parent=None):
        super().__init__(parent)
        
        # Our internal data is are just a set of the loggers:
        # The set, when populated is a set of dict describedi n the signals section
        # of the class documentation.
         
        self._loggers = set()
        
    # Attributes:
    
    def loggers(self) -> set[dict]:
        return self._loggers

    #  Methods:
    
    def addLogger(self, logger: dict) -> Self:
        '''
            @param logger  a llogger dict as descrsibed above.
            
            The addLogger signal is emitted with the logger passed
            as its value.  
        '''
        self._loggers.add(logger)
        self.addLogger.emit(logger)
        return self
    
    def deleteLogger(self, dest : str) -> Self:
        '''
            Removes the logger that logs to the specified destination.
            @param dest - the destination of the logger.
            
            - On success, loggerDeleted is signalled with the destination passed.
            - If the logger is not in the set, IndexError is raised.
            
        '''
        logger = self._findLogger(dest)
        self._loggers.remove(logger)
        self.loggerDeleted.emit(dest)
        return self
        
    
    
    def enableLogger(self, dest: str) -> Self:
        logger = self._findLogger(dest)
        logger['enabled'] = True
        self.loggerEnabled.emit(dest)
        
    def disableLogger(self, dest :str) -> Self:
        logger = self._findLogger(dest)
        logger['enabled'] = False
        self.loggerDisabled.emit(dest)
        
    # Internal methods.
    
    def _findLogger(self, dest: str) -> Self:
        # Find a logger or raise if the logger does not exist:
        
        logger = [l for l in self._loggers if l['destination'] == dest]
        if len(logger > 0):
            return logger[0]    # It's a  list.
        else:
            raise IndexError(f'No logger to {dest}')
        


class LoggerModel(QObject):
    '''
        Provides the model which backs up the state of logging.
        
        Attributes:
           enabled - True/False depending on the state.
        
        Signals:
            changed(bool)  - change of logging state new state is the parameter.
                This is only emitted if setEnabled actually changed the state
                e.g. the sequencde:
                setEnabled(True)
                SetEnabled(True)
                
                will emit at most one signal (only if the state was not already enabled.)
        
    '''
    changed = pyqtSignal(bool)
    def __init__(self, parent=None, state=False):
        '''
            @param parent - parent object, if there is one.
            @param state  - initial state of the enable.
        '''
        super().__init__(state)
        self._state = state
        
    # Attributes:
    
    def enabled(self) -> bool:
        return self._state
    
    def setEnabled(self, state: bool) -> Self:
        self._state = state
        self.changed.emit(state)
        return self
    
    
# views:

class LoggerConfig(QWidget):
    '''
        This is a view that's intended for use with the LoggerConfigModel.  We, sadly,
        can't use a QTableView because it's a pain to put a widget in it and we want
        checkbuttons for the logger enables.  Therefore, we have code manually deal with
        signals from the 'model' to maintain the table and add enable checkbuttons in the
        right hand column which we then have callbacks on to let the world know they've been
        changed.
          
        Signals:
          enableChanged(str, bool) - An enabled was clicked - str is the destination of the logger and the
                                     bool the new enabled state.
        Slots:
           addLogger - add a new logger.
           deleteLogger - Remove a logger.
           enableLogger - mark a logger enabled.
           disableLogger - mark a logger disabled.
           _enableChanged - (private) - the enable checkbutton was clicked for some item.
           
        Attributes:
            model   - (readonly) - the model we're attached to - a LoggerConfigModel. 
    '''
    enableChanged = pyqtSignal(str, bool)
    
    def __init__(self, parent=None):
        super().__init__(parent)
        self._model =LoggerConfigModel(self)
        
        # Make the table widget and set its headings:
        
        self._view = QTableWidget(self)
        self._view.setRowCount(5)
        self._view.setHorizontalHeaderLabels(
            ['Ring', 'Destination', 'P', 'C', 'Enabled']
        )
        
        # Connect to the model's signals so we can maintain the view:
        
        self._model.loggerAdded.connect(self._addLogger)
        self._model.loggerDeleted.connect(self._deleteLogger)
        self._model.loggerEnable.connect(self._enableLogger)
        self._model.loggerDisable.connect(self._disableLogger)
        
    
    ### attributes:
    
    def model(self) -> LoggerConfigModel:
        '''
            @return LoggerConfigModel - the model associated with this view.
        '''
        return self._model
        
    ### Slots:
    
    def addLogger(self, logger: dict) -> None:
        '''
            Append a logger to the table.  Note that column 4 is a checkbutton that both shows and
            controls the enable state of the logger on that row.
            
            @param logger - a dict describing the logger.  See LoggerConfigModel for the list of keys.
        '''
        # Add a new row saving the index of the new row (they count from 0).
        
        rowIndex = self._view.rowCount()
        self._view.setRowCount(rowIndex+1)
        
        self._view.setItem(rowIndex, 0, QTableWidgetItem(logger['ring']))
        self._view.setItem(rowIndex, 1, QTableWidgetItem(logger['destination']))
        self._view.setItem(rowIndex, 2, QTableWidgetItem('X' if logger['partial'] else ' '))
        self._view.setItem(rowIndex, 3, QTableWidgetItem('X' if logger['critical'] else ' '))
        
        # Now the checkbutton:
        
        widget = QCheckBox(self._view)
        widget.setCheckState(Qt.CheckState.Checked if logger['enabled'] else Qt.CheckState.Unchecked)
        widget.checkStateChanged.connect(self._enableCHanged)
        self._view.setCellWidget(rowIndex, 4, widget)  
        
    def deleteLogger(self, dest: str) -> None:
        '''
            Delete the logger with the specified destination from the table.
            
            @param dest - the logger destination to find.
            @throws IndexError if there's no such logger.
        '''
        row = self._findLoggerRow(dest)
        self._view.removeRow(row)
        
    def enableLogger(self, dest: str) -> None:
        '''
            Check the enable check box for a logger.  
            
            @param dest - logger destination
        '''
        row = self._findLoggerRow(dest)
        widget = self._view.cellWidget(row, 4)
        widget.setCheckState(Qt.CheckState.Checked)
    
    def disableLogger(self, dest: str) -> None:
        '''
            uncheck the check box for a logger.
            
            @param dest -the logger destination.
            
        '''
        row = self._findLoggerRow(dest)
        widget = self._view.cellWidget(row, 4)
        widget.setCheckState(Qt.CheckState.Unchecked)
        
    def _enableChanged(self, state: int) -> None:
        
        # called when a checkbox has signalled a change.  We need to figure out
        # which checkbox it is, get the logger destination, figure out the new state
        # and emit the enableChanged signal:
        
        row = self._view.currentRow()
        destination = self._view.item(row, 1).text()
        enabled     = True if state == Qt.CheckState.CHecked else False

        self.enableChanged.emit(destination, enabled)
        
    
    def _findLoggerRow(self, dest: str) -> int:
        
        matches = self._view.findItem(dest, 0)
        if len(matches) == 0:
            raise IndexError(f'There is no logger with the destination {dest}')
        
        row = matches[0].row()
        return row
        
        
        
        
    
        
        