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
  This script prepends a copy right notice to all of the c/c++
  headers and implementations on the command line.  The
  implementation files also have a copy right text embedded in them
  so that objects/executables based on them will include that 
  string
  
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
license_file = here + '/License.txt'       # THe c/c++ license text file.

# we want the date installed in the copyright string

year = time.localtime().tm_year
copyright_string = f'Copyright Michigan State University {year}, All rights reserved'


# suck in the entire license file text:

with open(license_file, "r") as f:
    license_contents = f.read()
    
# Define the file extensions that are implementations.
# Files of this type get a static const char* Coyright=copyright_string;
# stuffed into them.

implementation_types =('.cpp', '.cc', '.c', '.cxx', '.C')

#   Now iterate over the files on the command line:

for file in sys.argv[1:] :
    print("Processing: ", file)
    file_parts = path.splitext(file)  
    if len(file_parts) == 1:
        print(sys.stderr, file, " does not have an extension, skipping")
        continue
    extension = file_parts[1]
    
    #  Create a temp file.  We're going to write the comment header
    #  then, if extension is in implementation_types, the copyright
    # string definition, then the rest of the file.
    # After closing the temp file, we rename it to the original file and
    # delete the temp file.
    
    #  We turn of delete on destruction and close as we're going
    # To rename the temp file back into the original file after
    # closing it and appending the original to our prefix:
    
    outfile = NamedTemporaryFile(mode='wt', delete=False)
    tempname = outfile.name
    
    outfile.write(license_contents)
    if extension in implementation_types:
        print(file, " is an implementation, adding copyright string")
        outfile.write(f'static const char* Copyright = "{copyright_string}";\n')
        
    outfile.close()    # still around since we turned off delete_on_close
    
    # Append the input file to the output file:
    
    system(f'cat {file} >> {tempname}')
    
    # Rename the tempfile back to the  original:
    
    system(f'mv {tempname} {file}')
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

