#!/bin/bash

##
# @file reglom.sh
# @brief An example script illustrating how to rebuild events offline
#

##
# @note This script assumes a few things:
#   - You're running inside a container
#   - You have setup the NSCLDAQ environment e.g., by sourcing the
#     daqsetup.bash script for the NSCLDAQ version you're using
#   - Input data files are in the current directory and follow the usual
#     NSCLDAQ naming convention of "run-xxxx-yy.evt"
#   - The caller has write permission for the current directory
#   - There is enough space to write the temporary output files
#
# If any of these things is not true, i.e., you are attempting to read data
# from /mnt/rawdata, this script will not run properly.
# 

window=1000               # Coincidence window to build events
output_sid=0              # Build event source ID
output_file=reglommed.evt # Output file

# Construct input file list and unglom into data source files:
input=(run-*.evt)
cat ${input[@]} | $DAQBIN/Unglom - 2>&1

# Construct URIs from data source files:
sid_files=(sid*)
sid_uris=()
for f in ${sid_files[@]}; do
    uri=file://$PWD/$f
    sid_uris+=($uri)
done

# Run reglom to build events:
$DAQBIN/reglom --dt=$window --sourceid=$output_sid --output=$output_file ${sid_uris[@]} 2>&1

# Cleanup:
rm -f ${sid_files[@]}
