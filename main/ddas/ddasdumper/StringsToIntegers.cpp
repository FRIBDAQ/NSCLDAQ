/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
             NSCL
             Michigan State University
             East Lansing, MI 48824-1321
*/


#include "StringsToIntegers.h"

#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <string>

#include <DataFormat.h>
#include <stdexcept>

using namespace std;
using namespace ufmt;

static bool initialized = false;
static map<string, int>   textToInt;


static void initialize()
{
  textToInt["BEGIN_RUN"]           = BEGIN_RUN;
  textToInt["END_RUN"]             = END_RUN;
  textToInt["PAUSE_RUN"]           = PAUSE_RUN;
  textToInt["RESUME_RUN"]          = RESUME_RUN;
  textToInt["PACKET_TYPE"]         = PACKET_TYPES;
  textToInt["MONITORED_VARIABLES"] = MONITORED_VARIABLES;
  textToInt["PERIODIC_SCALERS"]    = PERIODIC_SCALERS;
  textToInt["PHYSICS_EVENT"]       = PHYSICS_EVENT;
  textToInt["PHYSICS_EVENT_COUNT"] = PHYSICS_EVENT_COUNT;
}

// 
//
static int
convertOne(string aNumber)
{
  char *end;

  int  value = strtol(aNumber.c_str(), &end, 0);
  if (*end != '\0') {
    if (textToInt.find(aNumber) != textToInt.end()) {
      return textToInt[aNumber];
    }
    else {
      string errmsg = "Converting a list to integers: must be an integer or a symbolic item type but was " + aNumber;
      throw std::invalid_argument(errmsg);
    }
  }
  return value;
}

////////////////////////////////////////////////////////////////////////////////
/*!
 *  This header rovides an unbound function that takes a comma separated
 *  list of integer (in string form) and converts them into a 
 *  vector of ints.
 *  This is most useful in decodin things like:
 *  
 * \verbatim
 *   ... --exclude=1,2,3 ...
 * \endverbatim
 *
 * \param items  - Stringified comma separated list of integers.
 * \return std::vector<int>
 * \retval Ordered vector of the integers decoded from the string.
 * \throw CInvalidArgumentException
 */
vector<int>
stringListToIntegers(string items) 
{
    size_t      start = 0;
    vector<int> result;
    size_t comma;

    if (!initialized) initialize();

    while ((comma = items.find(string(","), start)) != string::npos) {
        string aNumber;
	aNumber.assign(items, start, comma - start);

	result.push_back(convertOne(aNumber));
	start = comma+1;
    }
    // There's one last string that does not terminate in a comma:

    string aNumber;
    aNumber.assign(items, start, items.size() - start);
    result.push_back(convertOne(aNumber));

    return result;
}
