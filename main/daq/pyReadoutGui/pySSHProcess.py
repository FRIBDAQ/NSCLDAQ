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

from PyQt6.QtCore import QProcess, QObject



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
        
        pass

    def spawnRemote(self, remote : str, command : str) -> None:
        '''
        Spawn a program into the remote system.  After the program is run,
        Writes and reads etc. can be used to communicate with the program.
        
        @param remote :str - the system the program will run in. DNS name or dotted IP.
        @param command :str - The command to run.
        @note This is intended for short-lived commands.
        '''
        
        pass
    
    # Utilities
    
    def _ssh(self, host : str) -> None:
        #
        #   This actually set up and starts the ssh pipeline to the host.
        
        
        self.setProgram('ssh')
        self.setArguments([host,])
        self.setProcessChannelMode(QProcess.ProcessChannelMode.MergedChannels)
        self.start()
        if self._Write(f'cd {os.getcwd()}\n') < 0:
            raise RuntimeError('Unable to write the cd command to a subprocess')
        

    def _reconstructContainer(self) -> None:
        # If we are running in a containerized environment,
        # the following envrionment variables will be set:
        #
        # APPTAINER_CONTAINER - Full path to the container.
        # APPTAINER_SHELL     - shell to start in the container.
        # 
        if 'APPTAINER_CONTAINER' in os.environ:
            print('reconstruct container')
            container_image = os.environ['APPTAINER_CONTAINER']
            # Shell might or might not exist:

            if 'APPTAINER_SHELL' in os.environ:
                shell_spec = f'--shell {os.environ["APPTAINER_SHELL"]}'
            else:
                shell_spec = ''
            bindings_spec = self._getBindings()
            
            # Push the container startup command down the pipe:
            command = f'apptainer shell {shell_spec} {bindings_spec} {container_image}\n'
            
            self._Write(command)
            
    def _getBindings(self):
        # The bindings are either specified in an environment variable:
        # CONTAINER_BINDINGS if they exist or in the file ~/.singularity_bindpoints
        # if it exists or, no extra bindings will be done:
        
        if 'CONTAINER_BINDINGS' in os.environ:
            return f'--bind {os.environ["CONTAINER_BINDINGS"]}'
        else:
            bindpoints_file = pathlib.Path('~/.singularity_bindpoints').expand_user()
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
    
    def _Write(self, msg : str) -> int:
        # Needed because the pyqt bindings for QIODevice don't let us
        # write strings without first converting them to byte arrays and, well
        # let's face it, I'm too lazy to not encapsulate that  here:
        print("Sendinng", msg)
        return self.write(bytearray(msg, 'utf-8'))