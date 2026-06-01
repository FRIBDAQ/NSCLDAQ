'''
  This contains the main line code for the pyScaler python scaler display.
  
  @file main.py
  @brief main pyScaler program.
'''


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
import configfile
from sys import argv, exit, stderr

## 
# Output program usage.
def usage():
    print("Usage:" , file=stderr)
    print(f'   {argv[0]}  configfile', file=stderr)
    print( 'Where', file=stderr)
    print('    configfile - is a pyScaler configuration file.', file=stderr)
##
#  entry point.
def main():
    # There must be exactly one parameter, the configuration file
    # and it must exist:
    
    if len (argv) != 2:
        usage()
        exit(-1)
    
    # 
    # Process the configuration file:
    #
    with open(argv[1], 'r') as f:
        config_text = f.read()
    configuration = configfile.Configuration(config_text)
    

if __name__ == '__main__':
    main()
