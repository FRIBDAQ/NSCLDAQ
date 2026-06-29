#!/usr/bin/env python3
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


''''
   This program replaces the rdo_RunControl.tcl run control script.  It's used a bit differently 
   than rdo_RunControl.tcl which only supports default ReST services for the 
   Readouts and, hence, would not be usable if more than one Readout was running in a host as is,
   for example, the case in the S800 or maybe in test lab systems.
'''
pgm_usage= \
'''   
   Usage:
      $DAQBIN/rdo_PyRunControl mgr_host, mgr_user /path/to/readout_descriptions
      
      Where:
         mgr_host is the host on which the manager is running.
         mgr_user is the user that's running thye manager.
         /path/to/readout_descriptions is a readout description file, see below.
      
         The readout description file is a CSV file. Each line in the file describes
         a Readout program.
         *  If there is one field in a line, that's just the name of a Readout program
         *  If there are two fields, the second field is the name of that Readout's ReST service
         For example:
      
Readout_one
Reeadout_two,MyService

         Describes two Readouts.  'Readout_one 'uses the default ReST service of ReadoutREST while 
         'Readout_two' advertises its ReST server on 'MyService'

'''

import sys
import csv



def Usage() -> None:
   # Print the program usagbe on stderr.
   
   global pgm_usage
   print(pgm_usage, file = sys.stderr)


def main():
   if len(sys.argv) != 4:
      Usage()
      sys.exit(-1)

def process_arguments(arglist : list[str]) -> tuple[str, str, list[str]]:
   
   mgr_host = sys.argv[1]
   mgr_user = sys.argv[2]

   with open(sys.argv[3], newline='' ) as readout_file:
      readouts = csv.reader(readout_file)
      readout_list = list()
      for line in readouts:
         readout_list.append(line)
      

   return (mgr_host, mgr_user, readout_list)     
      
mgr_host, mgr_user, readout_list = process_arguments(sys.argv)
      
print('host', mgr_host)
print('user', mgr_user)
   
print(readout_list)

if __name__ == '__main__':
   main()