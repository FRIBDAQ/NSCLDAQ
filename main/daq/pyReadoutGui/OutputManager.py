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
                      will be highlighted with a red foreground.
                      This should be used _only_ for really
                      high priority events, like an unexpected program exit.
        setRecording - Turn the recording backgroun on/off for the texts we manage.
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
        
        self._nonRecordingStyleSheet = self._main.styleSheet()
        
        self._recordingStyleSheet    = 'QTextEdit { background-color : green; color : white; font-size : 15pt;}'
        self.addTab(self._main, 'Main')
        
        # Since it looks like there's no way to look up a 
        # tab's widget by index, we'll also maintain
        # the tab output windows as a name/widget dict.
        
        
        self._outputs : dict[str, OutputWindow.OutputWindow] = {}
        self.setRecording(False)
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
        newWindow.setStyleSheet('QTextEdit { font-size : 15pt;}')
        self.addTab(newWindow, name)
        self._outputs[name] = newWindow
        
        return newWindow
        
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
        
        fullText = '<span style="color: #800020;">' + msg + '</span>'

        #  See if we should add  the text to the current tab:
        
        currentTabText = self.tabText(self.currentIndex())
        if currentTabText in self._outputs:
            #  The current tab is one of our output widgets.
            #  Won't match if 'Main' either.
            widget = self._outputs[currentTabText]
            widget.append(fullText)
        
        # No matter what, add it to the main output widget:
        
        self._main.append(fullText)
        
    def setRecording(self, state : bool) -> None:
        '''
            Set the default background/foregroung colors to those
            appropriate to the recording state.
            
            @param state : bool  - True for reconding colors, False for non recording
        '''
        style = self._recordingStyleSheet if state else self._nonRecordingStyleSheet
        self._main.setStyleSheet(style)
        
        if not state:
            self._main.setStyleSheet('QTextEdit {font-size :  15pt;}')    
        for output in self._outputs.values():
            output.setStyleSheet(style)
            if not state:
                self._main.setStyleSheet('QTextEdit {font-size :  15pt;}')  
            
# Test code.

if __name__ == '__main__':

    import sys
    from PyQt6.QtWidgets import QApplication
    from PyQt6.QtCore    import QTimer

    # Slots:
    
    def output(widget :OutputWindow.OutputWindow) -> None:
        #  Standard output.
        
        widget.append('standard text')
    
    def toMain(win : OutputManager) -> None:
        # Some normal text to the main window:
        
        win.addToMain('main standard text')
    
    def emergency(win : OutputManager) -> None:
        #  With apologies to "The Russians are Coming" emergency text:
        
        win.emergencyMessage('Emergency, emergency, everyone to come out from the streets!')
    
    # Flip colors:
    
    recording = True
    def flip(win : OutputManager) -> None:
        global recording
        
        win.setRecording( recording)
        recording = not recording
        
    # entry

    app = QApplication(sys.argv)
    win = OutputManager()
    
    tab1 = win.addOutput('Tab1')
    tab2 = win.addOutput('tab2')
    
    # Timers to do output:
    #   To specific tabs
    t1Timer = QTimer()
    t1Timer.setInterval(2000)    # output every 2 secs.
    t1Timer.setSingleShot(False)
    t1Timer.timeout.connect(lambda : output(tab1))
    t1Timer.start()
    
    t2Timer = QTimer()
    t2Timer.setInterval(3000)   # Every 3 seconds.
    t2Timer.setSingleShot(False)
    t2Timer.timeout.connect(lambda : output(tab2))
    t2Timer.start()
    
    tmTimer = QTimer()
    tmTimer.setInterval(5000)
    tmTimer.setSingleShot(False)
    tmTimer.timeout.connect(lambda : toMain(win))
    tmTimer.start()
    
    
    #  Now emergency messages.
    
    teTimer = QTimer()
    teTimer.setInterval(30*1000)    # every 30 seconds.
    teTimer.setSingleShot(False)
    teTimer.timeout.connect(lambda : emergency(win))
    teTimer.start()
    
    # Colorization:
    
    rTimer = QTimer()
    rTimer.setInterval(60*1000)  # Flip every min.
    rTimer.setSingleShot(False)
    rTimer.timeout.connect(lambda : flip(win))
    rTimer.start()
    
    
    win.show()
    sys.exit(app.exec())

        
        


