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
@file Log.py
@brief Wrapper around the logging module to log actions of the ReadoutGui.
@author Ron Fox
'''


import logging
from typing import ClassVar, Self
import traceback

from nscldaq.readoutgui.ReadoutGuiView import mainWindow

# Exceptions ths class can throw:

class LogException(Exception):
    def __init__(self, msg : str):
        super().__init__(msg)

class SingletonViolation(LogException):
    def __init__(self, file : str):
        ''' @param file :str- file that attempted to instantiate Log'''
        super().__init__(
            f'{file} attempted to directly instantiate Log rather than using the "instance" method to get the singleton'
        )

#  This class is a singleton wrapper around the logger.
#  It also understands disablilng logging.
#  Log messages are also sent to the main output 
#  window and to all windows if sufficiently
#  high level 

class Log:
    _instance : ClassVar[Self | None] = None
    def __init__(self) -> None:
        '''
        Do not directly instantiate Log. You will get
        a LogSingletonViolation Exception if you do.
        Instead, use the class method instance() to get
        (and possibly construct) the singleton instance.
        '''
        
        # Ensure we are being called from inside this module:
        
        caller = traceback.extract_stack()[-2]
        if caller._filename != __file__:
            raise SingletonViolation(caller.filename)        

        # Setup the instance variables that control logging.

        self._filename = None
        self._enabled = False
        
    @classmethod
    def instance(cls) -> Self:
        if not cls._instance:
            cls._instance = cls()
        return cls._instance
    
    # Operations on a the singleton:
    
    def enable(self) -> None:
        '''
            Enable logging.
        '''
        self._enabled = True
    def disable(self) -> None:
        ''''
            Disable logging:
        '''
        self._enabled = False

    def setLogFile(self, path : str) -> None:
        '''
        Set the log file.  Note this creates a new
        logger  if one does not already exist for us.
        
        @param path : str - Path to the file.
        '''
        logger = logging.getLogger(__name__)
        logger.setLeval(logging.INFO)
        logging.basicConfig(filename=path)
        self._filename = path
        self._enabled = True    # setting a new filename enables.

    # Delegations to log.
    
    def debug(self, msg : str) -> None:
        '''
        Simplilfied delegation for the logger's debug method. 
        The user is responsible for all message formatting
        As this message will also be sent to the main output.
        
        @param the messager with whatever formattting indicators survived.
        '''
        if self._logging():
            logging.getLogger(__name__).debug(msg)
        self._logMain(msg)
        
    def info(self, msg : str) -> None:
        '''
        Simplified delegation for the logger's info method:
        @param msg - message to log.
        '''
        if self._logging():
            logging.getLogger(__name__).info(msg)
        self._logMain(msg)
            
    def warning(self, msg : str) -> None:
        '''
        Simplfied interface to the logger's warning method.
        @param msg - message to log.
        '''
        if self._logging():
            logging.getLogger(__name__).warning(msg)
        self._logMain(msg)
        
    def error(self, msg : str) -> None:
        if self._logging():
            logging.getLogger(__name__).error(msg)
        self._logAll(msg)
        
    def critical(self, msg: str) -> None:
        if self._logging():
            logging.getLogger(__name__).critical(msg)
        self._logAll(msg)
            
    # Utilities
    def _logging(self) ->bool:
        # Return true if we are logging:
        # We need both a filename and enabled to be true.
        return self._enabled and self._filename    
    def _logMain(self, msg: str) -> None:
        #  Log msg to the main window.
        
        mainWindow().Outputs().addToMain(msg)
        
    def _logAll(self, msg : str) -> None:
        # emergency message:
        mainWindow().Outputs().emergencyMessage(msg)