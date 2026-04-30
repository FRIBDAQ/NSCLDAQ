#! /usr/bin/env python3
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
  This script inserts a copyright notice in script files.
  note that unlike C++ files we can't just prepend it as   
  that would destroy any #! line in the script.
  Therefore we _append_ the  license file to the script.
  THe license file lives where we live and, for historical reasons
  is called License.tcl since most scripts were in Tcl in the past.
  however Tcl and Python have compatible commenting so...
  
  Note, this script is not intended to be intsalled but used by
  developers to ensure that copyright notices get appropriately
  added if they are missing.
  
'''
from os import path, system
import time
import sys
from tempfile import NamedTemporaryFile
# figure out where I am and the license filename

here = path.dirname(path.abspath(__file__))
license_file = here + '/License.tcl'       # The tcl/python license file.


#   Now iterate over the files on the command line:

for file in sys.argv[1:] :
    print("Processing: ", file)
    system(f'cat {license_file} >> {file}')#    This software is Copyright by the Board of Trustees of Michigan
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

