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
@file Slave.py
@brief Support enslavement of the program for remote control.
@author Ron Fox

This file implements the slave side of the remote control
protocol  In order to interoperate with the Tcl/Tk 
readout shell, we implement the same protocol as documented 
in the reference page named "Readout GUI Remote control protocol"
in the FRIB/NSCLDAQ documnetation.  This specifically provides
the following operations:

set slave 0/1 | run <run number> | title "<some title>" | recording 0/1 | destination <path>
begin
end
masterTransition tostate
init
get state | slave

Note that set slave destination is recognized but ignored since it has no
useful meaning in the pyReadoutGUI given that it supports multiple full
event loggers and that none of them are actually the 'main' logger.
'''
import socket

from nscldaq.portmanager.PortManager import PortManager
from nscldaq.readoutgui.ReadoutGuiView import mainWindow
from PyQt6.QtCore import QByteArray, Qt
from PyQt6.QtNetwork import QTcpServer, QTcpSocket

# This is vibe coded from gemini:

def _parse_tcl_list(tcl_str: str):
    """
    Parses a Tcl list string into a nested Python list without using tkinter.
    Handles nested braces and basic backslash escaping.
    """
    # Remove leading and trailing whitespace
    tcl_str = tcl_str.strip()
    
    stack = [[]]
    current_token = []
    in_braces_depth = 0
    escaped = False
    
    i = 0
    while i < len(tcl_str):
        char = tcl_str[i]
        
        if escaped:
            current_token.append(char)
            escaped = False
            i += 1
            continue
            
        if char == '\\':
            escaped = True
            i += 1
            continue
            
        if char == '{':
            # If we are entering a new nested list
            if in_braces_depth == 0 and not current_token:
                new_list = []
                stack[-1].append(new_list)
                stack.append(new_list)
            else:
                # Treat as literal character if inside an unclosed token
                current_token.append(char)
            in_braces_depth += 1
            
        elif char == '}':
            in_braces_depth -= 1
            if in_braces_depth == 0:
                if current_token:
                    stack[-1].append("".join(current_token))
                    current_token = []
                if len(stack) > 1:
                    stack.pop()
            else:
                if in_braces_depth < 0:
                    raise ValueError("Unmatched closing brace '}' in Tcl list.")
                current_token.append(char)
                
        elif char.isspace():
            if in_braces_depth > 0:
                # Keep whitespace if we are deep inside a braced element
                current_token.append(char)
            else:
                # Whitespace acts as a delimiter for the current item
                if current_token:
                    stack[-1].append("".join(current_token))
                    current_token = []
        else:
            current_token.append(char)
            
        i += 1
        
    if in_braces_depth != 0:
        raise ValueError("Unmatched opening brace '{' in Tcl list.")
        
    if current_token:
        stack[-1].append("".join(current_token))
        
    return stack[0]

class Slave:
    '''
    Encapsulates the listener and connection components of
    the slave subsystem Note that we are constructed
    with two bits of configuration information:
    - A service name (advertised with the port manager).
    - Enable which can be turned on/off.  disabling will
      refuse subsequent connections.
    - Once connected, this operates autonomously, accepting 
    requests and responding to them. See the docstrings in the
    methods that handle requests for details of the request/response.
    See the reference described in the module docstrings as well for
    a full protocol description.
    '''
    
    def __init__(self, configuration : dict[str, object]):
        '''
        @param configuration - the server configuration.  This dict has the following
            key/values:
            - 'service' [str]  - The name of the service to be advertised.
            - 'enabled' [bool] - If true, the server starts up, otherwise, it just configures
                                 and waits for a later enbable.
            @note - there is no interaction with the port manager to advertise the port until
                   the server is able to accept connections.
                   
        '''
        self._serviceName = configuration['service']
        self._enabled     = configuration['enabled']
        self._listener    : QTcpServer | None = QTcpServer() # Will be the listener
        self._server      : QTcpSocket | None = None      # Will be the connection.
        self._portManager : PortManager | None = None     # The port allocator.
        self._port        : int | None  = None            # THe port the manager gave us (persistent).
        self._isEnslaved    : bool  = False
        
        if self._enabled:
            self._startListening()                     # make and start listening if needed.
        
        # When a connection comes in, process it:
        
        self._listener.newConnection.connect(self._connect)
        
    def __del__(self):
        ''' If we are enabled shut everything down'''
        self.disable()
    
    # Public methods:
    
    def enable(self) -> None:
        '''
        Enable the server.  If it is already enabled, this is a no-op.
        If the server is being enabled, it starts listening for connections
        and making a server on the first one received.
        '''
        
        if not self._enabled:
            # Do nothing if already enabled.
            self._enabled = True
            self._startListening()
        
    def disable(self) -> None:
        '''
        Disable the server. If it is already disabled, this is a no-op.
        If the server is active it stops listening for connections
        and any active server is shutdown without processing any
        pening requests.
        '''
        
        if self._enabled:
            # Do nothing if already disabled.
            
            self._enabled = False
            
            if self._listener:
                self._listener.close()   # Will need to listen again.
                
            # If there is a connection, shut it down now
            
            if self._server:
                self._server.abort()     # Hard stop flushing queues.
                self._server = None      # Kill it off.
    
    def getConfig(self) -> dict[str, object]:
        '''
        Return the configuration map.  This is normally used
        to save the configuration so that it can be restored on startup.
        '''
        return {
            'service': self._serviceName,
            'enabled': self._enabled
        }
    #  Private Methods
    
    def _startListening(self) -> None:
        # Start listening for connections
        
        # If I don't have a port yet for my service, get one:
        
        if not self._portManager:
            self._portManager = PortManager('localhost')     # Need to maintain the connection.
            self._port        = self._portManager.getPort(self._serviceName) # since a port was allocated.
        
        # Start the server listening.  We are alredy connected.
        
        self._listener.listen(port = self._port)
    # Private methods:
    
    def _reqToWords(self, request : str) -> list[str]:
        # Note the function below does the right thing for 
        # {} but does not honor "" quoting therefore a 
        # set title "this is a    title"
        #  Will appear as list elements [... '"this', 'is', 'a' 'title"]
        #  Which loses the internal extra blanks and
        # requires stripping the "'s off the first and last word."
        # I have a hunch the tcl version has similar issues
        # stippling extra '  's.
        #  
        return _parse_tcl_list(request)
    
    def _makeTitle(self, words : list[str]) -> str:
        # Build the title back. Note that if the first and last
        # letters are '"' assume they were quoting letter and remove them.
        # There are actually two possibilities depending on the quoting
        #  set title "this is a title" produces additional words, one per title word.
        #  set title {this is a title} produces a nested list:
        
        if isinstance(words[0], list):
            title = ' '.join(words[0])  # {} quoted case.
        else:    
            title = ' '.join(words)     # "" quoted case.
            title.strip('"')
        return title
                        
    def _doSet(self, tail) -> str:
        # Note that anything but set slave requires us to be enslaved.
        
        # There must be at least two elements in the tail:  a thing to set
        # and a value - title is a bit more complex as we'll see.
        
        # Return value is the response to the client - without the trailing newline.
        
        if len(tail) < 2:
            return 'ERROR - The set command requires something to set and a to set it to.'
        
        what = tail.pop(0)
        if what != 'slave' and not self._isEnslaved:
            return f'ERROR - set {what} requires enslavement but we are emancipated.'
        
        #  Only title can have more than one remaining tail element:
        if what != 'title' and len(tail) > 1:
            return f'ERROR - set {what} can only have a single additional word'
        match what:
            case 'slave':
                # set slave 1|0  - require the appropriate integer:
                try:
                    which = int(tail[0])
                    if which not in [1,0]:
                        return 'ERROR - set slave requires 1 or 0 but got {which}'
                    
                    if which:
                        self._enslave()
                    else:
                        self._emancipate()
                    return 'OK'
                except ValueError:
                    return f'ERROR - set slave value must be 1 or 0 was {tail[0]}'
            case 'run':
                #  Similar restrictions to set slave, but any whole number:
                try:
                    run = int(tail[0])
                    if run < 0:
                        raise ValueError('bad value')
                    # Got a valid run number -- set it and return OK.
                    
                    mainWindow().centralWidget().RunParameters().setRun(run)
                    return 'OK'
                except ValueError:
                    return f'ERROR run number must be a counting number, was {tail[0]}'
            case 'title':
                # Title requires some processing of the tail to produce a title string.
                # see the comments in _reqToWords.
    
                title = self._makeTitle(tail)
                mainWindow().centralWidget().RunParameters().setTitle(title)
                return 'OK'
            case 'recording': 
                # Same restrictions on the value that set slave has:
                try: 
                    which = int(tail[0])
                    if which not in [1,0]:
                        raise ValueError('bad value')
                    state = Qt.CheckState.Checked if which else Qt.CheckState.Unchecked
                    mainWindow().centralWidget().Recording().setCheckState(state)
                    return 'OK'
                except ValueError:
                    return f'ERROR - set recording requires a 1 or 0 got {tail[0]}'
            case 'destination':
                # This is supported but a no-op for protocol completeness.  Our
                # local recording configuration is quite a bit more complex.
                return 'OK'
            case _:
                return f'ERROR - set {what} is not legal must set slave|run|title|recording|destination'
    
    def _doRequest(self, request : str) -> str:
        #
        #  This is the beast that actually responds
        #  to requests, performing them and returning
        #  the string to send back to the client
        #  Note that if we are not a slave but get anything other
        #  than set slave 1 we fail.
        request_words = self._reqToWords(request)
        print('Request words are:', request_words)
        verb = request_words.pop(0)
        
        if verb != 'set' and not self._isEnslaved:
            return f'ERROR - The {verb} request requires enslavement and we are emancipated'
        
        match verb:
            case 'set':
                return self._doSet(request_words)
            case 'begin':
                return self._doBegin(request_words)
            case 'end':
                return self._doEnd(request_words)
            case 'masterTransition':
                return self._doTransition(request_words)
            case 'get':
                return self._doGet(request_words)
            case _:
                return f'ERROR - {verb} is not a recognized request!'

    # Internal (private) slots:
    
    def _connect(self) -> None:
        # A master is connecting. It appears that the act of connecting implicitly
        # enslaves us.
        
        self._server = self._listener.nextPendingConnection()
        
        # Set the signal handling for the connection.
        
        self._server.disconnected.connect(self._lostConnection)    # Unenslaved on connection lost.
        self._server.readyRead.connect(self._request)
        
        # set the GUI to show we are enslaved and by whom.
        
        self._enslave()
    
    def _lostConnection(self) -> None:
        # Lost connection to the client:
        self._emancipate()   # 12 years a slave is enough :-P
        
        # Clean up the server.
        
        self._server.abort()
        self._server = None
    
    def _enslave(self) -> None:
        # Enslave ourselves:
        
        self._isEnslaved = True    
        mw  = mainWindow()        
        gui = mw.centralWidget()
        gui.enslave()                               #  disable widgets.
        
        # Now a status message:
        
        peer = self._server.peerAddress().toString()  

        # Try to get a host name:
        try:
            peer = socket.gethostbyaddr(peer)
        except socket.herror:
            pass    # No translation leaves the peer as a text address.
        except OSError:
            pass   # If address family not supported.
        mw.setStatusMessage(f'Enslaved by {peer}')
        
        
    def _emancipate(self) -> None:
        # No longer a slave
        
        self._isEnslaved = False
        mw = mainWindow()
        gui = mw.centralWidget()
        gui.emancipate()
        mw.setStatusMessage('No longer enslaved')
        
        
    def _request(self) -> None:
        #  A request came in from a client that has enslaved us
        
        # Get the request and turn it into a string:
        
        lineArray = self._server.readLine()   
        if lineArray:
            line = lineArray.data().decode('utf-8')
            response = self._doRequest(line) + '\n'
            qresponse = QByteArray(response.encode('utf-8'))
            self._server.write(qresponse)
        else: 
            # Likely we lost the connection...
            # Eventually _lostConnection will fire so just ignore
            pass
        
    
