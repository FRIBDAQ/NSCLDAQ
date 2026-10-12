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
@file MVLCDataSource.ply
@brief Data source for the MVLC controller Readout.
@author Ron Fox
@note The program is normally installed with the mesytec-mvlc driver, if at all.

The registration must look for the program fribdaq-readout.  It looks for it in one of the
following places:

As normally installed in FRIBDAQ - /usr/opt/mesytec-mvlc/x.y.z   looking for it in all of the x.y.z directories and choosing
the one with the largest semantic version number e.g. on my dev system I have
0.0.1/ 0.1.0/ 0.1.3/ 0.1.4/ 0.1.5/  all of which were built to enable the installation of fribdaq-readout.
The one in 0.1.5 will be chosen.

If the environment variable MVLC_DIR is defined, that overrides this search and the program
is looked for in $MVLC_DIR/bin.

If the program is found this data source is registerd as 'MVLC'


'''

from nscldaq.readoutgui.FRIBDAQDataSource import FRIBDAQSource
from packaging.version import Version
import os
import pathlib

def _findExecutable() -> str | None:
    # Locate the fribdaq-readout executable.
    
    if 'MVLC_DIR' in os.environ:
        binary =  pathlib.Path(os.environ['MVLC_DIR']) / 'bin/fribdaq-readout'
        if binary.exists():
            return str(binary)
        else:
            return None
    else:
        mvlc = pathlib.Path('/usr/opt/mesytec-mvlc')
        if not mvlc.exists():
            return None
        # Which subdirectories have our program in their 'bin' subdirs:
        
        exists_in = []
        for subdir in mvlc.iterdir():
            program = subdir / 'bin/fribdaq-readout'
            if program.exists():
                exists_in.append(subdir)
        
        if len(exists_in) == 0:
            return None                          # None of those subdirs has the readout.
        # exists_in are the subdires that have our readout program.. we need to get the
        # one with the largest semantic version.
        
        latest = exists_in[0] 
        for version in exists_in:
            latest_version = Version(latest.parts[-1])
            this_version   = Version(version.parts[-1])
            if this_version > latest_version :
                latest = version
            
        #  Now latest is the most recent version whose bin has fribdaq-readout so:
        
        return str(latest / 'bin/fribdaq-readout')
    

        
        
            
