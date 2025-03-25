#!/usr/bin/env python3
'''
Provides pidToCommandLine which takes a PID and turns it into an iterable 
of command line words.

NSCLDAQ should also install this as $DAQBIN/pidtocommand so that
remote PIDs can be interrogated e.g.

nsclda.ssh(remotehost $DAQBIN/pidtocommand somepid)

'''
import psutil
import sys

def pidToCommand(pid):
    '''
       Given a PID returns an iterable that contans the
       command words of the process designated by the pid.
       Note this works only on a local system.
       
       An exception will be thrown if the PID does not exist (I think).
    '''
    try:
        process = psutil.Process(pid)
        return process.cmdline()
    except:
        raise KeyError(f'No such process pid={pid}')


###
#  If this is run as a standalone python program it will output the
#  command line to stdout... if an exception was thrown it will
#  output that to stderr.
#

if __name__  == "__main__":
    pid = int(sys.argv[1])
    try:
        command = pidToCommand(pid)
        print(' '.join(command))
        sys.exit(0)
    except KeyError as e:
        sys.stderr.write(str(e)+"\n");
        sys.exit(-1)