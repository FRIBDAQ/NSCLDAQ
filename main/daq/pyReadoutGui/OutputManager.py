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
@file OutputManager.py
@brief Tabbed notebook of outputs windows.
@author Ron Fox
'''
from nscldaq import OutputWindow
from PyQt6.QtWidgets import QTabWidget, QWidget


# Exceptions for this module:

class OutputManagerException(Exception):
    '''
    All output manager exceptions are derived from this.
    '''
    def __init__(self, message : str):
        super().__init__(message)
    
class NoSuchTab(OutputManagerException):
    '''
        Tab lookup failed.
    '''
    def __init__(self, name : str) :
        super().__init__(f'There is no tab named {name}')

class DuplicateTabName(OutputManagerException):
    '''
        Tab names must be unqiqe:
    '''
    def __init__(self, name : str):
        super().__init__(f'There is already a tab named {name}')
    

#  The manager:

class OutputManager(QTabWidget):
    '''
        Output manager provides a tabbed notebook of OutputWindow
        widgets.  These can be used by facilities to output
        text to a targeted output window.  A Global 'main'
        output window provides shared output from
        several sources, if desired.  
        
        Methods:
        
        addOutput    - Adds a new tab/output window, returns the output window widget.
        removeOutput - Removes the specified tab/output window.
        mainOutput  - Returns a reference to the main output widget. Note 
                      removeOutput on the 'main' tab is an error abd raises
                      the InvalidTab exception.
        addToMain   - Adds a message to the main tab output widget.
        emergencyMessage - Adds a message to the main tab and, if it's not the
                      currently displayed tab the current tab.  These messages
                      will be highlighted with a red foreground and black
                      background.  This should be used _only_ for really
                      high priority events, like an unexpected program exit.
        
        @note  To support rich text, the clients of the OutputWindow should
               emit the html subset supported by Qt. Note that the output window
               automatically wraps stuff added with addOutput in a <p></p> tag
               pair.
    
        @note The widget is derived from a QTabWidget so it's possible/supported
              for a client to decide they want to put some arbitrary tab contents
              in it and just addTab to this object and do it.
    '''
    def __init__(self, parent : QWidget | None = None):
        super().__init__(parent)
        
        # Now create the 'main' window:
        
        self._main = OutputWindow.OutputWindow(self)
        self.addTab(self._main, 'Main')
        
        # Since it looks like there's no way to look up a 
        # tab's widget by index, we'll also maintain
        # the tab output windows as a name/widget dict.
        
        
        self._outputs : dict[str, OutputWindow.OutputWIndow] = {}
    # Public methods:
    
    def addOutput(self, name : str) -> OutputWindow.OutputWindow:
        '''
        Add another tab containing an output window.
        @param name - Tab text.
        @return OutputWindow.OutputWIndow - The output window.
        @throws DuplicateTabName if name is already a tab name.
        '''
        if name in self._outputs:
            raise DuplicateTabName(name)
        newWindow = OutputWindow.OutputWindow(self)
        self.addTab(newWindow, name)
        self._outputs[name] = newWindow
        
    def removeOutput(self, name: str) -> None:
        '''
        Remove the named tab and output window.
        @param name - name of the tab to remove.
        @throws NoSuchTab if there is no tab with that name.
        
        '''
        # Doing this prevents the removal of the
        # Special 'Main' tab, since we cleverly did not
        # put it in the outputs dict.
        
        if name not in self._outputs:
            raise NoSuchTab(name)    
        
        # Now find the tab and kill it off:
        
        for tab in range(self.count()):
            if self.tabText(tab) == name:
                # match:
                
                self.removeTab(tab)
                self._outputs.pop(name)
                return

        
        
    
    def mainOutput(self) -> OutputWindow.OutputWindow:
        '''
        @return OutputWindow.OutputWindow - The output window of the 'Main' tab
    
        '''
        return self._main
    
    def addToMain(self, msg : str) -> None:
        '''
        Add a normal priority message to the main tab output. 
        @param msg : str  - The message to add.
        '''
        self._main.append(msg)
    
    def emergencyMessage(self, msg : str) -> None:
        '''
            Adds a message in red color with black background to both
            the currently displayed output window and the
            "Main" tab.  note that if the type of the
            widget in the tab is not one of _our_ OutputWindow, the text
            is only added to the main wnidow.   This can happen
            if a client adds tabs we don't know about.
            
        '''
        
        # wrap the text in a div to set the color:
        
        fullText = '<div color="red" bgcolor="black">' + msg + '</div>'
        
        #  See if we should add  the text to the current tab:
        
        currentTabText = self.tabText(self.currentIndex())
        if currentTabText in self._outputs:
            #  The current tab is one of our output widgets.
            widget = self._outputs[currentTabText]
            widget.append(fullText)
        
        # No matter what, add it to the main output widget:
        
        self._main.append(fullText)
        
    
        
        


