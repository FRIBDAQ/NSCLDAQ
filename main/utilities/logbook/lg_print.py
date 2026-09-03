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
@file lg_print.py 
@brief Produce PDF's suitable for printing of chunks in the logbook.


'''


from nscldaq.LogBook import LogBookUIUtilities, logbookadmin, LogBook
import sys


def usage() -> None:
    print('Usage:', file=sys.stderr)
    print('    $DAQBIN/lg_print filename [what ...]', file=sys.stderr)
    print('Where:', file=sys.stderr)
    print('     filename - is the path of a file into which PDF will be generated.', file=sys.stderr)
    print('     what     - select what is to be printed.  If omitted, the entire logbook is printed.', file=sys.stderr)
    print('                Each "what" item can be any the text "none" to print notes not associated', file=sys.stderr)
    print('                with any run or a run number, to print that run and all associated notes.', file=sys.stderr)
    print('Examples:', file=sys.stderr)
    print('   $DAQBIN/lg_print all.pdf              # Prints everything into the file all.pdf', file=sys.stderr)
    print('   $DAQBIN/lg_print runs1-4.pdf 1 2 3 4  # Prints runs 1,2,3,4 into runs1-4.pdf', file=sys.stderr)
    print('   $DAQBIn/lg_print stuff.pdf none 1 2 3 # Print un-associated notes then runs 1-4', file=sys.stderr)
    print('\nThe items are rendered in the order in which they are specified.', file=sys.stderr)
    print('If a nonexistent run is specified a message is printed to stderr and processing continues', file=sys.stderr)

def list_unassociated_notes() -> list [LogBook.Note]:
    return logbookadmin.listNonRunNotes()

def get_run_items(number : int) -> list[LogBook.Run | LogBook.Transition | LogBook.Note]:
    # Return all the items for a run sorted in time order (so notes appear interspersed with
    # transitions)
    
    
    run = logbookadmin.findRun(number)
    if run is None:
        print(f'There is no run numberd {number} continuing', file = sys.stderr)
        return []
    
    # First item is always the header:
    result = [run,]
    
    stamped_items = [run.get_transition(n) for n in range(run.transition_count())]
    stamped_items.extend(logbookadmin.listNotesForRun(run.number))
    stamped_items.sort(key=lambda item: item.time)   # Both transitions and notes have a time attr.
    
    result.extend(stamped_items)
    
    return result

def make_item_list(specs : list[str]) -> list[LogBook.Run | LogBook.Transition | LogBook.Note]:
    # Given the whats in the command ine convert that to a list of logbook items os
    # we can use LogBookUIUtilities.generateMardownFromItemList to make the markdown for the print.
    
    result = []
    for item in specs:
        if item == 'none':
            result.extend(list_unassociated_notes())
        else:
            # Make a run number if possible and faile complete if cant':
            
            try:
                run_number = int(item)
                result.extend(get_run_items(run_number))
            except Exception as t:  # noqa: BLE001
                print(f'Invalid item specification {item}: {t}', file = sys.stderr)
                print('Aborting', file = sys.stderr)
                usage()
                sys.exit(-1)
    return result
    

def main() -> int:
    # Program entry point.
    if len(sys.argv) == 1:
        usage()
        return -1
    
    filename = sys.argv[1]
    
    # Make the list of stuff to print:
    
    what     = sys.argv[2:]
    if len(what) == 0:
        what =['none',]
        
        # This is not so efficient but it makes for consistent code:
        
        runs = [str(run.number) for run in logbookadmin.listRuns()]
        what.extend(runs)
    
    # Make the list of LogBook items to print.
    
    itemlist = make_item_list(what)
    markdown = LogBookUIUtilities.generateMarkdownFromItemList(itemlist)
    
    # Generate the PDF to the file:
    
    LogBookUIUtilities.markdownToPdf(markdown, filename)
    
    return 0
if __name__ == '__main__':
    sys.exit(main())

