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

'''
    Create a new database, optionally prompting graphically for the
    parameters required to build it.   If the database already
    exists, we don't allow this.
    
    @file lg_create.py
    @brief Create a new logbook database.
    @author Ron Fox
    
'''
import sys
import argparse
from nscldaq import logbookadmin
from nscldaq.LogBook import LogBook

def define_arguments() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog = 'lg_create',
        description='Create and optionally make current a logbook',
        epilog='Only -filename is mandatory.   Missing options will be prompted graphically'
    )
    parser.add_argument('-filename', required=True, help='Path to logbook file to create')
    parser.add_argument('-current', type=bool, default=False, help='True if the logbook created should be made current, False if omitted')
    parser.add_argument('-experiment', help='The facility experiment number being logged')
    parser.add_argument('-spokesperson', help='The name of the experiment spokesperson')
    parser.add_argument('-purpose', help='The experiment purpose.')
    return parser
    
def main() -> int:
    parser = define_arguments()
    parsed_args = parser.parse_args()    # Defaults to procecessing sys.argv
    
    # Extract the values of the arguments that were parsed into the namespace
    
    filename     = parsed_args.filename
    current      = parsed_args.current
    experiment   = parsed_args.experiment
    spokesperson = parsed_args.spokesperson
    purpose      = parsed_args.purpose
    
    # If we have all we need, make the logbook:
    
    if experiment and spokesperson and purpose:
        try :
            logbookadmin.createLogBook(filename, experiment, spokesperson, purpose, current)
        except LogBook.error as e:
            print(f'Unable to create logbook: {e}')
            return -1
        return 0
    else:
        return 0
    
    
    

if __name__ == "__main__":
    sys.exit(main())
