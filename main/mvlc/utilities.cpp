
/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox 
             Facility for Rare Isotope4s
             Michigan State University
             East Lansing, MI 48824-1321


@author Ron Fox <fox at frib dot msu dot edu>
@brief some utility functions for mvlcgenerate.
*/
#include "utilities.h"
#include "options.h"
#include <string>
#include <filesystem>


/** computeOutfile
 *    @param args - parsed parameters from gengetopt - by reference.
 *    @return the name of the output file.
 *    @retval - value of the --output option if provided.
 *    @retval - value of the input file with the extension changed to .yaml if --output is not
 *             provided.
 */
std::string
computeOutfile(gengetopt_args_info& args) {
    std::string result;
    if (args.output_given) {
        result =  std::string(args.output_arg);
    } else {
        std::filesystem::path outpath(std::string(args.inputs[0]));      // original path.
        std::filesystem::path ext(std::string(".yaml"));
        outpath.replace_extension(ext);
        result = outpath.string();
    }
    return result;
}