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
@file pySSHProcess.py
@brief Execute programs in remote systems via SSH - re-creating container env.
@author Ron Fox
'''
import os
import pathlib

from PyQt6.QtCore import QObject, QProcess


class SSHProcess(QProcess):
    '''
        Runs a proces in a remote system over SSH.  To do this requires that the
        remote system have our public keys installed in ~/.ssh/authorized_keys
        so that it is not necessary to provide a password to do the login.
        
        Some Definitions:
        
        Host - the host/system that creates this objecdt 
        Remote - The system in which the program will be run.
        
        If the Host is running a containerized environment, we attempt to 
        duplicate this environment in the Remote system.  This requires some help,
        as the information available about the container environment is not sufficient
        to restore the bind points.  The bind point targets are available in 
        APPTAINER_BIND, but the sources of each target are not...and typically, in our
        environment some arbitrary tree gets bound to /usr/opt.  That help comes in the form
        of a file ~/.singularity_bindpoints.  This is overidden by the environment
        variable CONTAINER_BINDINGS if defined.
        
        ~/.singulazrity_bindpoints - contains bindings one per line
        CONTAINER_BINDINGS contains a string that is suitable for the --bind option.
        
        
        We assume that the Host is running Apptainer but we don't make that assumption of
        the Remote.  Therefore, the container is started in the remote with
        "singularity shell" rather than the more modern "apptainer shell"
        
        Key points:
        -  We derive from QProcess, its signals should be used to know when to read
           the stdout/stderr of the process.
        -  We configure the QProcess to a merged output so the stdout and stderr
           are availble on as common QIODevice.
        -  The cwd of the remote is set to the same as ours -- the assumption is a unified
           file system between Host and Remote.
        -  The remote is assumed to run a Unixy shell, specifically sh derived  (not csh)
        
        The key methods we add as value to QProcess:
        
        runRemote - Runs a non-interactive program in the remote, waits for it to
            complete and returns the stdout/stderr to the caller.
        spawnRemote - Runs a, potentially, interactive program in the remote
            and returns immediately.  This allows the program to be interacted with
            via writes to it.
            
        Note in constructing commands, the caller is responsible for appropriate quoting
        of the command and the command will be executed in a shell (in the container
        if appropriate).
    '''
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
    def runRemote(self, remote : str, command : str) -> str:
        '''
        Runs a command in the remote, waiting for it to finish, returning the
        full output/error as a combined string.
        
        @param remote :str - host in which the program will run. DNS name or dotted IP.
        @param command - The command to run.
        '''
        self.spawnRemote(remote, command)
        output = ""
        while self.state() != QProcess.ProcessState.NotRunning:
            self.waitForReadyRead(1000)
            data =  self.ReadAll()
            if data is not None:
                output += data
        
        return output
        
    def spawnRemote(self, remote : str, command : str) -> None:
        '''
        Spawn a program into the remote system.  After the program is run,
        Writes and reads etc. can be used to communicate with the program.
        
        @param remote :str - the system the program will run in. DNS name or dotted IP.
        @param command :str - The command to run.
        @note This is intended for  long-lived commands.
        '''
        
        fullcommand = self._reconstructContainer() + '"' + command + '"'
        self._ssh(remote, fullcommand)

    
    def ReadAll(self) -> str:
        '''
            This utility reads any pending data from the stdout/stderr pipes
            and returns it as a string.
            Much more useful than the stuff provided
            
            @return str - the string read.
            @retval None - there's no pending in put.
        '''
        result = self.readAll()
        if result:
            return result.data().decode('utf-8')
        else:
            return None
    def ReadLine(self) -> str:
        '''
        Reads a line of from the stderr/stdout stream and 
        decodes it into a string.
        
        @return str - the line.
        @retval None - There's no pending input.
        
        '''
        result = self.readLine()
        if result:
            return result.data().decode('utf8')
        else:
            return None
        
    def Write(self, msg : str) -> int:
        ''' Needed because the pyqt bindings for QIODevice don't let us
            write strings without first converting them to byte arrays and, well
            let's face it, I'm too lazy to not encapsulate that  here:
            
            @param msg  - message to write.
            @return  int - number of bytes written...
        ''' 
        return self.write(bytearray(msg, 'utf-8'))
    def WriteLine(self, msg : str) -> int:
        '''
         Same as Write abovbe but just appends a \n to the msg before
         writing it.
         
         @param msg - the message to write.
         @return int - number of bytes written.
         
        '''
        writemsg = msg + '\n'
        return self.Write(writemsg)
    # Utilities
    
    def _ssh(self, host : str, command :str) -> None:
        #
        #   This actually set up and starts the ssh pipeline to the host.
        
        
        self.setProgram('ssh')
        self.setArguments([host, command])
        self.setProcessChannelMode(QProcess.ProcessChannelMode.MergedChannels)
        self.start()
        
        

    def _reconstructContainer(self) -> None:
        # If we are running in a containerized environment,
        # the following envrionment variables will be set:
        #
        # APPTAINER_CONTAINER - Full path to the container.
        # APPTAINER_SHELL     - shell to start in the container.
        # 
        #  IF containerized, a prefix to the actual command is returned.
        #
        if 'APPTAINER_CONTAINER' in os.environ:
            container_image = os.environ['APPTAINER_CONTAINER']
            # Shell might or might not exist:


            bindings_spec = self._getBindings()
            
            # Push the container startup command down the pipe:
            # We use a login script, hoping the user
            # has setup a DAQ e.g.
            
            command = f'apptainer exec {bindings_spec} {container_image} bash -lc '
            
            return command
        else:
            return ''
    def _getBindings(self):
        # The bindings are either specified in an environment variable:
        # CONTAINER_BINDINGS if they exist or in the file ~/.singularity_bindpoints
        # if it exists or, no extra bindings will be done:
        
        if 'CONTAINER_BINDINGS' in os.environ:
            return f'--bind {os.environ["CONTAINER_BINDINGS"]}'
        else:
            bindpoints_file = pathlib.Path('~/.singularity_bindpoints').expanduser()
            if bindpoints_file.exists():
                bindings = ''
                with bindpoints_file.open() as f:
                    line = f.readline()
                    line = line[0:-1]      # Slice off \n.
                    bindings += line + ','
                bindings  = bindings[0:-1]    # Slice off trailing ','.
                return f'--bind {bindings}'
            else:
                return ''
    
    
# Test program

if __name__ == '__main__':
    import sys

    from nscldaq.OutputWindow import OutputWindow
    from PyQt6.QtWidgets import (
        QApplication,
        QLineEdit,
        QPushButton,
        QVBoxLayout,
        QWidget,
    )
    
    
    def runit():
        cmd = command.text()
        p   = SSHProcess()
        out.append(p.runRemote('localhost', cmd))
    
    app = QApplication(sys.argv)
    win = QWidget()
    
    win.setLayout(QVBoxLayout())
    out = OutputWindow(win)
    win.layout().addWidget(out)
    command = QLineEdit(win)
    win.layout().addWidget(command)
    button = QPushButton('Execute', win)
    win.layout().addWidget(button)
    button.clicked.connect(runit)
    
    win.show()
    sys.exit(app.exec())