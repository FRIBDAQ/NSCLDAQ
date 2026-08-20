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
    
    @file lg_addperson.py
    @brief Add a person to the current data base (which must be set).
    @author Ron Fox.
'''
import sys
from nscldaq import logbookadmin
from nscldaq.LogBook import LogBook


def usage() -> None:
    print('Usage:', file=sys.stderr)
    print('     $DAQBIN/lg_addperson [lastname] [firstname] [salutation]', file=sys.stderr)
    print('Where', file=sys.stderr)
    print('    lastname - is the last name of the person to add to the logbook', file=sys.stderr)
    print('    firstnam - is the firstname of the person to add to the logbook', file=sys.stderr)
    print('    salutation - is the salutation.', file=sys.stderr)
    print('Any omitted arguments are prompted for graphically', file=sys.stderr)

def duplicate_person(lastname, firstname) -> bool:
    # True if this person already exists:
    
    
    people = logbookadmin.listPeople()    
    all_names = [(n.lastname, n.firstname) for n in people]
    for name in all_names:
        if (lastname, firstname) == name:
            return True
    return False

def main() -> int:
    if logbookadmin.currentLogBook() is None:
        print('No current log book has been established, use $DAQBIN/lg_current to do that.')
        return -1
    if len(sys.argv) > 4:
        usage()
        return -1
    
    lastname = None if len(sys.argv) < 2 else sys.argv[1]
    firstname = None if len(sys.argv) < 3 else sys.argv[2]
    salutation = None if len(sys.argv) < 4 else sys.argv[3]
    
    if not (lastname and firstname and salutation):
        return -1             # @todo gui prompt.
    
    if lastname and firstname and salutation:
        try:
            # Prevent a duplicate lastname, firstname pair:
            if duplicate_person(lastname, firstname):
                print(f'{firstname} {lastname} already exists', file=sys.stderr)
                return -1
            logbookadmin.addPerson(lastname, firstname, salutation)
            return 0
        except LogBook.error as e:
            print(
                f'Unable to add {salutation} {firstname} {lastname} : {e}', 
                file=sys.stderr
            )
            return -1
    else:
        # THe dialog omitted some stuff... return success.
        return 0

if __name__ == '__main__':
    sys.exit(main())
    
