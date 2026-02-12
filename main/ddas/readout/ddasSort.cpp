/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
             NSCL
             Michigan State University
             East Lansing, MI 48824-1321
*/

/**
 * @file ddasSort.cpp
 * @brief Main program to sort DDAS hits.
 */

#include "DDASSorter.h"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include "ddasSortOptions.h"
#include <CRemoteAccess.h>
#include <CRingBuffer.h>
#include <Exception.h>

/**
 * @brief Entry point to the sorter: process comand line arguments,
 * instantiates and invokes the application class with appropriate parameters.
 * @details
 * In order to get best performance from DDASReadout, sorting of the resulting
 * hits has been pushed downstream. This program reads ring items that consist
 * of multiple hits from boards and outputs ring items that consist of single
 * hits from all boards in an experiment sorted by time. Ring item bodies we
 * take as input look like:
 *
 * +------------------------------------------------------+
 * | Size of the body in 16 bit words (uint32_t)          |
 * +------------------------------------------------------+
 * | Module ID uint32_t (note bit 21 says use ext clock)  |
 * +------------------------------------------------------+
 * | Clock scale factor (double precision)                |
 * +------------------------------------------------------+
 * | Hit 1, Hit 2, ...                                    |
 * | ...                                                  |
 */
int main(int argc, char **argv) {
  gengetopt_args_info parsedArgs;
  cmdline_parser(argc, argv, &parsedArgs);

  std::string sourceURI = parsedArgs.source_arg;
  std::string sinkRing = parsedArgs.sink_arg;
  float accumWindow = parsedArgs.window_arg;

  int status = EXIT_SUCCESS;
  std::string errorMessage;

  // Create the CRingBuffer objects for the source (which could specify
  // a remote ring) and sink (which must be a local ring).

  try {
    // Using unique pointers below ensures cleanup regardless how we
    // exit (e.g. including exceptions):
    std::unique_ptr<CRingBuffer> pSource(
        CRingAccess::daqConsumeFrom(sourceURI));
    /**
     * @todo (ASC 3/18/24): Sometimes the sorter fails to create its
     * sort ring. In my attempts to resurrect the sorter tests, I had
     * some issues making multiple `createAndProduce` calls from within
     * the same function. Here it had better be making that call only
     * once. Is this more reliable if I call `CRingBuffer::create()` and
     * `new CRingBuffer()` similar to whats done in sortertests.cpp?
     */
    std::unique_ptr<CRingBuffer> pSink(CRingBuffer::createAndProduce(sinkRing));
    DDASSorter sorter(*pSource, *pSink, accumWindow);
    sorter();

  } catch (CException &e) {
    errorMessage = "NSCLDAQ exception caught: ";
    errorMessage += e.ReasonText();
    status = EXIT_FAILURE;
  } catch (std::exception &e) {
    errorMessage = "C++ library exception caught: ";
    errorMessage += e.what();
    status = EXIT_FAILURE;
  } catch (std::string msg) {
    errorMessage = "std::string exception caught: ";
    errorMessage += msg;
    status = EXIT_FAILURE;
  } catch (const char *msg) {
    errorMessage = "const char* exception caught: ";
    errorMessage += msg;
    status = EXIT_FAILURE;
  } catch (...) {
    errorMessage = "Unanticpated exception type caught";
    throw; // Maybe C++ lib will tell us the type.
  }

  if (status != EXIT_SUCCESS) {
    std::cerr << errorMessage << std::endl;
  }

  exit(status);
}
