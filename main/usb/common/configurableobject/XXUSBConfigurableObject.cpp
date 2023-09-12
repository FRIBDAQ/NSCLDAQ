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

#include <config.h>
#include "XXUSBConfigurableObject.h"
#include <set>
#include <stdlib.h>
#include <errno.h>
#include <tcl.h>
#include <ErrnoException.h>
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <math.h>

#include <stdlib.h>
#include <sstream>

using namespace std;
using namespace XXUSB;

////////////////////////////////////////////////////////////////////////////
//////////////////////////    Constants ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static const CConfigurableObject::ListSizeConstraint unconstrainedSize = 
  {CConfigurableObject::limit(0),
   CConfigurableObject::limit(0)};


// Inline utilities:

// Convert an integer to a string:

inline std::string itos(int i)
{
  std::stringstream ss;
  ss << i;
  return ss.str();
}


///////////////////////////////////////////////////////////////////////////
//////////////////////// Canonical member functions ///////////////////////
///////////////////////////////////////////////////////////////////////////

/*!
   Contruct the object:
   \param name : std::string
       Name to give to the object.
*/
CConfigurableObject::CConfigurableObject(string name) :
  m_name(name)
{
}

/*!
  Destruction probably will result in some memory leaks since
  it is possible that user typeChecker's may have parameters that
  are dynamically allocated. 

  If people use the convenience functions for building typed parameters,
  we will manage that memory properly.  Each dynamic item gets added to the m_constraints
  list from which it gets deleted via free.

*/
CConfigurableObject::~CConfigurableObject()
{
  releaseConstraintCheckers();


}

/*!
   Copy construction.  We can just copy the name and map.
   Copy construction should be rare as normally names are unique.
*/
CConfigurableObject::CConfigurableObject(const CConfigurableObject& rhs) :
  m_name(rhs.m_name),
  m_parameters(rhs.m_parameters)
{

}
/*!
   Assignement is similar to copy construction:
*/
CConfigurableObject&
CConfigurableObject::operator=(const CConfigurableObject& rhs)
{
  if (this != &rhs) {
    m_name       = rhs.m_name;
    m_parameters = rhs.m_parameters; 
  }
  return *this;
}

/*!
  Equality is via item comparison.
*/
int
CConfigurableObject::operator==(const CConfigurableObject& rhs) const
{
  return ((m_name == rhs.m_name)   &&
	  (m_parameters == rhs.m_parameters));
}
/*!
  Inequality is the logical inverse of equality:
*/
int
CConfigurableObject::operator!=(const CConfigurableObject& rhs) const
{
  return !(*this == rhs);
}

//////////////////////////////////////////////////////////////////////////
///////////////////////// Selectors //////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/*!
  \return string
  \retval The name of the object.
 */
string
CConfigurableObject::getName() const
{
  return m_name;
}

/*!
   Get the value of a single configuration item.
   \param name : std::string
      Name of the parameter to retrieve.

   \return std::string
   \retval the value of the configuration parameter.

   \throw string - if name is not a known configuration parameter, afte all,
                   clients should know the configuration parameters.. if they don't
		   see the next cget which retrieves parameters and values.
*/
string
CConfigurableObject::cget(string name) 
{
  ConfigIterator found = m_parameters.find(name);
  if (found == m_parameters.end()) {
    string msg("CConfigurableObject::cget was asked for parameter: ");
    msg += name;
    msg += " which is not defined";
    throw msg;
  }
  ConfigData data = found->second;
  return data.first;
}
/*!
   Get the values of all the configuration parameters.
   \return CCOnfigurableObject::ConfigurationArray
   \retval A vector of parameter name/value pairs.
           Given an element, ele of the vector,
	   ele.first is the parameter name, ele.second the value.

   \note While at present, the parameters come out sorted alphabetically,
         you should not count on that fact.
*/
CConfigurableObject::ConfigurationArray
CConfigurableObject::cget() 
{
  ConfigurationArray result;
  ConfigIterator p = m_parameters.begin();
  while(p != m_parameters.end()) {
    
    string name = p->first;
    ConfigData data = p->second;
    string value = data.first;
    result.push_back(pair<string, string>(name, value));
    
    p++;
  }
  return result;
}
///////////////////////////////////////////////////////////////////////////
/////////////////////// Establishing the configuration /////////////////////
///////////////////////////////////////////////////////////////////////////

/*!
   Adds a configuration parameter to the configuration.
   If there is already a configuration parameter by this name,
   it is silently ovewritten.

   \param name : std::string
      Name of the parameter to add.
   \param checker : typeChecker
      A type checker to validate the values proposed for the parameter,
      if NULL, no validation is performed.
   \param arg   : void
      Parameter passed without interpretation to the typechecker at validation
      time.
   \param defaultValue : string (default = "")
      Initial value for the parameter.
*/
void
CConfigurableObject::addParameter(string      name, 
				      typeChecker checker,
				      void*       arg,
				      string      defaultValue)
{
  TypeCheckInfo checkInfo(checker, arg);
  ConfigData    data(defaultValue, checkInfo);
  m_parameters[name] = data;	// This overwrites any prior.
}

/*!
    Configure the value of a parameter.
    \param name : std::string
       Name of the parameter to configure.
    \param value : std::string
       New value of the parameter

   \throw std::string("No such parameter") if the parameter 'name' is not defined.
   \throw std::string("Validation failed for 'name' <- 'value'") if the value
           does not pass the validator for the parameter.
*/
void
CConfigurableObject::configure(string name, string value)
{
  // Locate the parameter and complain if it isn't in the map:

  ConfigIterator item = m_parameters.find(name);
  if(item == m_parameters.end()) {
    string msg("No such parameter: ");
    msg  += name;
    throw msg;
  }
  // If the parameter has a validator get it and validate:

  TypeCheckInfo checker = item->second.second;
  typeChecker   pCheck  = checker.first;
  if (pCheck) {			// No checker means no checkig.
    if (! (*pCheck)(name, value, checker.second)) {
      string msg("Validation failed for ");
      msg += name;
      msg += " <- ";
      msg += value;
      throw msg;
    }
  }
  // Now set the new validated value:

  m_parameters[name].first = value;

}
/*!
  clear the current configuration.  The configuration map m_parameters
  map is emptied.
*/
void
CConfigurableObject::clearConfiguration()
{
  m_parameters.clear();
  releaseConstraintCheckers();
}


/////////////////////////////////////////////////////////////////////////////
///////////////////// convenience functions /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

// Convenience functions return the value of a parameter converted to one
// a commonly used form.  The caller is responsible for ensuring that the
// parameter is type checked to that form prior to using these functions,
// as they do not report conversion errors...but instead return some
// value that may or may not make sense.


/*!
    Return the value of an integer parameter.
    \param name : std::string
       Name of the value.

    \return int
    \retval the integer equivalent of the config paramter.

*/
std::int64_t
CConfigurableObject::getIntegerParameter(string name)
{
  string value = cget(name);

  char* end;
  long long iValue = strtoll(value.c_str(), &end, 0);
  if (end == value.c_str()) {
    string msg = "Expected an integer parameter value for config. parameter ";
    msg += name;
    msg += "got: ";
    msg += value;
    throw msg;
  }
  return iValue;
}
/*! 
  Same as above but for an integer.  Needed because strtol for something
  bigger than MAXINT returns MAXINT.
*/
std::uint64_t
CConfigurableObject::getUnsignedParameter(string name)
{
  string value = cget(name);

  char* end;
  std::uint64_t iValue = strtoull(value.c_str(), &end, 0);
  if (end == value.c_str()) {
    string msg = "Expected an integer parameter value for config. parameter ";
    msg += name;
    msg += "got: ";
    msg += value;
    throw msg;
  }
  return iValue;
}

/*!
  Return the value of a bool parameter.
  This uses the same set of true values as the checker.. however
  any other value is assumed to be false.

  \param name : std::string
     Name of the value.
  
  \return bool
  \retval the boolean equivalent of the config param
*/
bool
CConfigurableObject::getBoolParameter(string name)
{
  string value = cget(name);
  return strToBool(value);


}

/*!
   Return a parameter decoded as a double.

   \param name : std::string
     Name of the parameter

   \return double

*/
double
CConfigurableObject::getFloatParameter(string name)
{
  string value = cget(name);
  return atof(value.c_str());
}

/*!
  Return a parameter that is a list of integers.
  \param name - name of the parameter.
  \return vector<int>
  \retval Vector containing the integers in the list.

*/
vector<int64_t>
CConfigurableObject::getIntegerList(string name)
{
  string value = cget(name);
  int argc;
  const char** argv;
  vector<int64_t> result;

  Tcl_SplitList(NULL, value.c_str(), &argc, &argv);

  for (int i =0; i < argc; i++) {
    result.push_back(static_cast<int64_t>(strtol(argv[i], NULL, 0)));
  }
  Tcl_Free((char*)argv);
  return result;

}

vector<uint64_t>
CConfigurableObject::getUnsignedList(string name)
{
  string value = cget(name);
  int argc;
  const char** argv;
  vector<uint64_t> result;

  Tcl_SplitList(NULL, value.c_str(), &argc, &argv);

  for (int i =0; i < argc; i++) {
    result.push_back(static_cast<uint64_t>(strtoul(argv[i], NULL, 0)));
  }
  Tcl_Free((char*)argv);
  return result;
}

vector<bool>
CConfigurableObject::getBoolList(string name)
{
  string value = cget(name);
  int argc;
  const char** argv;
  vector<bool> result;

  Tcl_SplitList(NULL, value.c_str(), &argc, &argv);

  for (int i =0; i < argc; i++) {
    result.push_back(strToBool(argv[i]));
  }
  Tcl_Free((char*)argv);
  return result;
}

/**
 * getFloatList
 *    Get a list of values that are doubles.
 *  @param name - name of the parameter.
 *  @return std::vector<double> - list values.
 */
vector<double>
CConfigurableObject::getFloatList(string name)
{
  string value = cget(name);
  
  // bust it up into a  list:
  
  int argc;
  const char** argv;
  Tcl_SplitList(NULL, value.c_str(), &argc, &argv);
  
  // Parse the values intothe result.
  
  vector<double> result;
  for (int i = 0; i < argc; i++) {
      result.push_back(strtod(argv[i], nullptr));
  }
  // Free the parsed list elements:
  
  Tcl_Free(reinterpret_cast<char*>(argv));
  
  return result;
}

/**
 * Return a parameter that is a list as a vector of strings.
 *
 * @param name - name of configuration parameter.
 *
 * @return std::vector<std::string>
 * @retval vector whose elements are the list elements.
 */
vector<string>
CConfigurableObject::getList(string name)
{
    string value = cget(name);
    int argc;
    const char** argv;
    vector<string> result;
    Tcl_SplitList(NULL, value.c_str(), &argc, &argv);
    
    for (int i = 0; i < argc; i++) {
        result.push_back(argv[i]);
    }
    Tcl_Free((char*)argv);
    return result;
}

/**
 * getEnumParameter
 *
 *  Given a parameter name and the null terminated list of
 *  values it can take, returns the index into that list of
 *  the value the parameter currently has, or throws if there
 *  is no match
 *
 * @param name - Option name.
 * @param pValues - Array of pointers to the values.  The final pointer is null
 *
 * @return int index of the matching enum value.
 * @throw std::string - If there is no matching value in the
 *    pValues array.yyy
 */
int
CConfigurableObject::getEnumParameter(std::string name, const char** pValues) 
{
  std::string value = cget(name);
  
  int i = 0;
  while(*pValues) {
    if(value == std::string(*pValues)) {
      return i;
    }
    i++;
    pValues++;
  }

  std::string msg("Enum parameter ");
  msg += name;
  msg += " has the value ";
  msg += value;
  msg += " which is not one of the allowed values";

  throw msg;
  
}
/**
 * getListOfLists
 *     Return the value of a paramter that is a list of lists.
 *  @param name -name of the parameter
 *  @return std::vector<std::vector<std::string>>
 */
std::vector<std::vector<std::string>>
CConfigurableObject::getListOfLists(std::string name)
{
  // Get the raw value:
  
  auto rawValue = cget(name);          // Throws if nosuch.
  std::vector<std::vector<std::string>> result;
  // Break up the outer list and marshall that into a vector of strings
  // so we can immediately free the result:
  
  int argc;
  const char** argv;
  if (Tcl_SplitList(nullptr, rawValue.c_str(), &argc, &argv) != TCL_OK) {
    std::string msg = rawValue;
    msg += " is not a valid Tcl list";
    throw msg;
  
  }
  std::vector<std::string> outerList;
  for (int i =0; i < argc; i++) {
    outerList.push_back(argv[i]);
  }
  Tcl_Free((char*)(argv));
  
  // Now split each sublist:
  
  for (auto l : outerList) {
    if (Tcl_SplitList(nullptr, l.c_str(), &argc, &argv) != TCL_OK) {
      std::string msg = "sublist: " ;
      msg += l;
      msg += " is not a valid Tcl list";
      throw msg;
      
    }
    // Now marshall the sublist into its vector and push it into the result:
    
    std::vector<std::string> sublist;
    for (int i =0; i < argc; i++) {
      sublist.push_back(argv[i]);
    }
    Tcl_Free((char*)(argv));
    result.push_back(sublist);
  }
  
  return result;
  
  
}
/** 
 * Add an integer parameter to the configuration that has no limits.
 *
 * @param name - Name of the configuration item (e.g. -someparam).
 * @param defaultVal - Value given the configuration parameter if it is not explicitly configured.
 */
void
CConfigurableObject::addIntegerParameter(std::string name, std::int64_t defaultVal)
{


  // Use the normal creation function.

  addParameter(name, CConfigurableObject::isInteger, NULL, std::to_string(defaultVal));

}
/**
 * Add an integer parameter to the configuration with supplied lower and
 * upper limits.
 *
 * @param name - Name of the parameter (e.g. -someparam).
 * @param low  - Low limit on the values accepted for the parameter.
 * @param high - High limit on the values accepted for the parameter.
 * @param defaultVal - Default value of not explicitly given.  This is 0 unless that's outside the
 *                     low/high range in which case, low is used.
 */
void
CConfigurableObject::addIntegerParameter(std::string name, std::int64_t low, std::int64_t high, std::int64_t defaultVal)
{
  // Build the constraint object, and hook it into the autodelete mechanism.

  Limits* pLimit = new Limits;
  pLimit->first.s_checkMe  = true;
  pLimit->first.s_value    = low;
  pLimit->second.s_checkMe = true;
  pLimit->second.s_value   = high;

  DynamicConstraint c = {
    releaseLimitsConstraint,
    reinterpret_cast<void*>(pLimit)
  };
  m_constraints.push_back(c);

  // Add the parameter:

  addParameter(name, CConfigurableObject::isInteger, pLimit, std::to_string(defaultVal));


}
/**
 * addFloatParameter
 *    Add a parameter with a floating point type checker.  Overloads support
 *    range checking as well:
 *
 * @param name  - name of the parameter
 * @param defaultValue -optional default value (defaults to zero).
 * @param low     - parameter low limit
 * @param high    - parameter high limit.
 * @note float parameters  are actually doubles.
 */
void
CConfigurableObject::addFloatParameter(std::string name, double defaultValue)
{
    addParameter(name, CConfigurableObject::isFloat, nullptr, std::to_string(defaultValue));
}
void
CConfigurableObject::addFloatParameter(
      std::string name, double low, double high, double defaultValue
)
{
    // Constraint on value - note we're just going to leak memory since
    // anticipated lifetime is program lifetime.
    
    FloatingLimits& constraint = *(new FloatingLimits);
    constraint.first.s_checkMe = true;
    constraint.first.s_value   = low;
    constraint.second.s_checkMe = true;
    constraint.second.s_value   = high;
    
    addParameter(
        name, CConfigurableObject::isFloat, &constraint, std::to_string(defaultValue)
    );
}

/**
 * Add a boolean parameter to the configuration.
 *
 * @param name - Name of the parameter e.g. -somebool
 * @param defaultVal - Default value of the parameter
 */
void
CConfigurableObject::addBooleanParameter(std::string name, bool defaultVal)
{
  addParameter(name, CConfigurableObject::isBool, NULL,
	       std::string(defaultVal ? "true" : "false"));
}
/**
 * Convenience function to add an enumerated parameter.
 * @param name     The name of the new parameter.
 * @param pValues  Null terminated list of strings that are the valid values for the
 *                 enumerator.
 * @param defaultValue default value of the parameter.  This parameter is optional and
 *                  defaults to first valid value
 * @note it is illegal; for pValues to contain an empty string but it's the responsibility of the
 *       caller to ensure this.
 */
void
CConfigurableObject::addEnumParameter(string       name,
				      const char** pValues,
				      string       defaultValue)
{
  const char** p = pValues;
  isEnumParameter* pNewItem = new isEnumParameter;
  while(*p) {
    pNewItem->insert(*p++);
  }
  addParameter(name, isEnum, pNewItem,
	       defaultValue == "" ?
	       pValues[0] : defaultValue);

  // Arrange for the constraint to be freed on construction:

  DynamicConstraint cRelease = {
    CConfigurableObject::releaseEnumConstraint, reinterpret_cast<void*>(pNewItem) 
  };
  m_constraints.push_back(cRelease);
}

/**
 * Add a configuration parameter that is a list of booleans.  In this case the list
 * has a fixed size (e.g. channel enables for a module).
 *
 * @param name - Name of the parameter e.g. -enables.
 * @param size - Number of elements in the list.
 * @param defaultVal - Default value.  This  is a single bool.  The resulting
 *                     initial configuration value is a Tcl list of 'size' elements
 *                     all of them defaultVal's value.
 */
void
CConfigurableObject::addBoolListParameter(std::string name, unsigned size, bool defaultVal)
{
  // Map this into a call to the more generic overload:

  addBoolListParameter(name, size, size, defaultVal, size);


}

/**
 * Add a boolean list parameter with a few more constraints:
 *
 * @param name - Name of the parameter.
 * @param minLength - Minimum allowed length of the list.
 * @param maxLength - Maximum allowed length of the list.
 * @param defaultVal - The default value given to each element of the list (see below).
 * @param defaultLength - Number of elements in the default list.  If not supplied, minLength is 
 *                        used.
 */
void 
CConfigurableObject::addBoolListParameter(std::string name, unsigned minLength, unsigned maxLength, 
					  bool defaultVal, int defaultSize)
{

  defaultSize = computeDefaultSize(minLength, maxLength, defaultSize);

  // Make the default list.  Lists of bools are more easily made with string manipulation
  // than with Tcl list operations:

  std::string sDefault = 
    defaultVal ? "true" : "false"; // trailing space is not a typo. but used to make Tcl lists.
  std::string defaultList = simpleList(sDefault, defaultSize);

  
  // Build the constraint and arrange for it to get destroyed:

  ListSizeConstraint* pConstraint = createListConstraint(minLength, maxLength);


  // Add the parameter:

  addParameter(name, CConfigurableObject::isBoolList, pConstraint, defaultList);
  

}
/**
 * Add an integer list parameter.  
 *
 * @param name - name of the config parameter.
 * @param size - Number of elements in the list.
 * @param defaultVal - Default value given to each element of the list.
 */
void
CConfigurableObject::addIntListParameter(std::string name, unsigned size, std::int64_t defaultVal)
{
  // This is just a special case of another overload:

  addIntListParameter(name, size, size, defaultVal, size);
}
/**
 * Add an integer list parameter where the size of the  list is constrained.
 * 
 * @param name      - name of the parameter.
 * @param minLength - Minimum number of elements in the list.
 * @param maxLength - Maximum number of elements inthe list.
 * @param defaultVal - Default value given to each list element.
 * @param defaultLength - Default list length (minLength if not given).
 */
void
CConfigurableObject::addIntListParameter(std::string name, unsigned minLength, unsigned maxLength,
					 std::int64_t defaultVal, int defaultSize )
{
  // Figure out the actual list size and build the default list.
  // string operations are much simpler than Tcl list ones since each element is an integer.


  defaultSize = computeDefaultSize(minLength, maxLength, defaultSize);
  std::string defaultList = simpleList(std::to_string(defaultVal), defaultSize);

  // Build the constraint struct and arrange for it to be freed when this object is
  // destroyed:

  ListSizeConstraint* pConstraint = createListConstraint(minLength, maxLength);
  
  // Add the parameter:

  addParameter(name, CConfigurableObject::isIntList, pConstraint, defaultList);
  
}
/**
 * addIntListParameter
 *    Add an integer list parameter where the list elements are range checked.
 *
 * @param name      - name of the parameter.
 * @param minValue - Smallest allowed value for the integer.
 * @param maxValue - Largest allowed value for the integer.
 * @param minLength - Minimum number of elements in the list.
 * @param maxLength - Maximum number of elements inthe list.
 * @param defaultVal - Default value given to each list element.
 * @param defaultLength - Default list length (minLength if not given).
 */
void
CConfigurableObject::addIntListParameter(std::string name, std::int64_t minValue, std::int64_t maxValue,
                           unsigned minlength, unsigned maxLength, unsigned defaultSize,
			   std::int64_t defaultVal)
{
    // Figure out the actual list size and build the default list.
    // string operations are much simpler than Tcl list ones since each element is an integer.
  
  
    defaultSize = computeDefaultSize(minlength, maxLength, defaultSize);
    std::string defaultList = simpleList(std::to_string(defaultVal), defaultSize);
  
    // First build the constraint on the values of the list:
    
    Limits* pRange = reinterpret_cast<Limits*>(malloc(sizeof(Limits)));
    pRange->first.s_checkMe  = true;
    pRange->first.s_value    = minValue;
    pRange->second.s_checkMe = true;
    pRange->second.s_value   = maxValue;
    DynamicConstraint c = {                  // Ensure this gets destroyed.
       free,
       reinterpret_cast<void*>(pRange)
     };
     m_constraints.push_back(c);
     
     // Now the list constraint with the above used with the element
     // constraints:
     
     isListParameter* pConstraint =
        reinterpret_cast<isListParameter*>(malloc(sizeof(isListParameter)));
     *pConstraint = isListParameter(minlength, maxLength, TypeCheckInfo(isInteger,pRange));  
    DynamicConstraint l = {
        free,
        reinterpret_cast<void*>(pConstraint)
    };
    m_constraints.push_back(l);
    
    // Add the parameter:
  
    addParameter(name, CConfigurableObject::isIntList, pConstraint, defaultList);    
}
/**
 * addFloatList
 *   Parameter that is a list of validated floats with optional low/high limits
 *   and optional default (missing default ->0.0).
 *
 * @param name - parameter name.
 * @param minlen - minimum allowed list length.
 * @param maxlen - maximum allowed list length.
 * @param defaultsize - default list length.
 * @param min    - minimum value for each list element.
 * @param max    - min value for each list element.
 * @param defaultValue - default value for each element.
 */
void
CConfigurableObject::addFloatListParameter(                                                    
    std::string name, unsigned minlen, unsigned maxlen, unsigned defaultsize,
    double defaultValue 
)
{
     // Build the default value string:
     
     defaultsize = computeDefaultSize(minlen, maxlen, defaultsize);
     std::string defaultList =
        simpleList(std::to_string(defaultValue), defaultsize
     );
        
      // Parameter checker is an isFloatList with a list size constraint
      
      ListSizeConstraint* pConstraint = createListConstraint(minlen, maxlen);
      
      addParameter(name, CConfigurableObject::isFloatList, pConstraint, defaultList);
}

void
CConfigurableObject::addFloatListParameter(                                              
    std::string name, double low, double high,
    unsigned minlen, unsigned maxlen, unsigned defaultlen,
    double defaultValue
)
{
    // Build the default value string:
     
    defaultlen = computeDefaultSize(minlen, maxlen, defaultlen);
    std::string defaultList =
       simpleList(std::to_string(defaultValue), defaultlen
    );
       
    // First build the value constraint so that it can be applied to the list
    // constraint:
    
    FloatingLimits* range =
      reinterpret_cast<FloatingLimits*>(malloc(sizeof(FloatingLimits)));
    range->first.s_checkMe = true;
    range->first.s_value = low;
    range->second.s_checkMe = true;
    range->second.s_value = high;
    DynamicConstraint c = {                  // Ensure this gets destroyed.
       free,
       reinterpret_cast<void*>(range)
     };
     m_constraints.push_back(c);
    
    // now the list length constraint with the added range constraint:
    
    isListParameter* pConstraint =
      reinterpret_cast<isListParameter*>(malloc(sizeof(isListParameter)));
    *pConstraint = isListParameter(minlen, maxlen, TypeCheckInfo(isFloat, range));
      DynamicConstraint l = {
        free, reinterpret_cast<void*>(pConstraint)
      };
      m_constraints.push_back(l);
    
     addParameter(name, isFloatList, pConstraint, defaultList);
}


/**
 * addEnumListParameter
 *    Add a parameter whose value is a list of validated enumerated values.
 * @param name - name of the parameter.
 * @param pValues - null terminated array of valid values.
 * @param defaultValue - default value if not supplied
 * @param minlength - minimum allowed length.
 * @param maxlenth  - maximum allowed length.
 * @param defaultlen - Default list length (-1 if maxlength).
 * 
 */
void
CConfigurableObject::addEnumListParameter(
  std::string name, const char** pValues, const char* defaultValue,
  unsigned minlength, unsigned maxlength, int defaultSize
)
{
    // figure out the default list of values:
    
    defaultSize = computeDefaultSize(minlength, maxlength, defaultSize);
    std::string defaultList = simpleList(defaultValue, defaultSize);
    
    // Create the constraint object:
    
    auto validValues = makeEnumSet(pValues);
    isEnumParameter* pV = new isEnumParameter(validValues);
    isListParameter* pConstraint =
       reinterpret_cast<isListParameter*>(malloc(sizeof(isListParameter)));
    *pConstraint = isListParameter(minlength, maxlength, TypeCheckInfo(isEnum, pV));
    std::string dList = simpleList(defaultValue, defaultSize);
    addParameter(name, isList, pConstraint, dList);
    
}
/**
 * addListOfEnumLists
 *    This one is needed for the per channel triggers sources:
 *    The parameter is a list of lists where each sublist is a list of values
 *    from a set of valid values. For examples [list [list GlobalTrigger SelfTrigger] ...]
 *    would trigger a channel 0 on the or of its self trigger and the global trigger
 *    conditions.
 *    See the companion getListOfLists which fetches the resulting value as
 *    a vector of vector of strings.  Where the outer vector is the outer string elements
 *    and each inner vector contains the list for each element.
 * @param name -parameter name.
 * @param pValues - the values elements of each of the sublist elements can take.
 * @param defaultValue - The default value for each list element.
 * @param minlength - minimum length of the outer list.
 * @param maxlength - maximum length of the outer list
 * @param defaultSize - default list size.
 *
 * @note there is no constraint on the sublist sizes though the actual device
 *       parameter may constrain that (e.g. there must be at least one
 *       trigger condition).
 */
void
CConfigurableObject::addListOfEnumLists(
    std::string name, const char** pValues, const char* defaultValue,
     unsigned minlen, unsigned maxlen, int defaultSize
)
{
  // The constraint is really pretty much the same as the enum list but
  // we use the element type checker isEnumlist which breaks apart the list it
  // is given and checks each element against the enums rather than its entire argument.
  
   // figure out the default list of values:
    
    defaultSize = computeDefaultSize(minlen, maxlen, defaultSize);
    std::string defaultList = simpleList(defaultValue, defaultSize);
    
    // Create the constraint object:
    
    auto validValues = makeEnumSet(pValues);
    isEnumParameter* pV = new isEnumParameter(validValues);
    isListParameter* pConstraint =
       reinterpret_cast<isListParameter*>(malloc(sizeof(isListParameter)));
    *pConstraint = isListParameter(minlen, maxlen, TypeCheckInfo(isEnumList, pV));
    std::string dList = simpleList(defaultValue, defaultSize);
    addParameter(name, isList, pConstraint, dList);
}
/**
 * Add a string parameter whose value is a fixed length array of strings.
 * 
 * @param name - name of the parameter.
 * @param size - number of items in the list.
 * @param defaultVal - Default value for all the strings.  This is the empty string.
 */
void
CConfigurableObject::addStringListParameter(std::string name, unsigned size, std::string defaultVal)
{
  addStringListParameter(name, size, size, defaultVal, size);
}
/**
 * Add a string array parameter with constraints on size:
 *
 * @param name - name of the parameter.
 * @param minLength - minimum number of items in the list.
 * @param maxLength - Maximum number of items in the list.
 * @param defaultVal - Default value for all the strings.  This is the empty string.
 * @param defaultLength - default length of the list.
 */
void
CConfigurableObject::addStringListParameter(std::string name, unsigned minLength, unsigned maxLength,
			      std::string defaultVal, int defaultLength)
{
  defaultLength = computeDefaultSize(minLength, maxLength, defaultLength);
  std::string dList = simpleList(defaultVal, defaultLength);

  // build the constraint and  create the parameter

  ListSizeConstraint* pConstraint = createListConstraint(minLength, maxLength);
  addParameter(name, CConfigurableObject::isStringList, pConstraint, dList);
}




////////////////////////////////////////////////////////////////////////////
/////////////////////  Stock type checkers //////////////////////////////////
////////////////////////////////////////////////////////////////////////////

/*!
   Validate an integer parameter with optional limits.
   \param name : std::string
       Name of the parameter being checked (ignored).
   \param value : std::string
       Proposed new value.
   \param arg : void*
       This is actually a pointer to an Limits structure or NULL.
       - If NULL, no range checking is done.
       - If not null, range checking is done.  Each limit contains a checkme
         flag which allows validation to occur when one or both limits are needed.
         limits are inclusively valid.
    \return bool
    \retval true  - Validation passed.
    \retval false - Validation failed.
*/
bool
CConfigurableObject::isInteger(string name, string value, void* arg)
{
  // first determine the 'integernes' using strtoul.


  char* end;
  auto lvalue = strtoull(value.c_str(), &end, 0);	// Base allows e.g. 0x.
  if (*end != '\0') {		               // String is not just an integer.
    return false;
  }
  // If there's no validator by definition it's valid:

  if(!arg) return true;

  // Get the validator in the correct form:

  Limits* pRange = static_cast<Limits*>(arg);
  if (!pRange) {
    string msg("BUG: argument for integer validator for parameter: ");
    msg += name;
    msg += " is not a pointer to a Limits type";
  }
  // check lower limit:

  if((pRange->first.s_checkMe) && (lvalue < pRange->first.s_value)) {
    return false;
  }
  if ((pRange->second.s_checkMe) && (lvalue > pRange->second.s_value)) {
    return false;
  }
  return true;
}

/*!
    Validate a bool parameter.  Bool parameters have any of the values:
    - true: true, yes, 1, on, enabled
    - false: false, no, 0, off, disabled
   We'll just delegate this off to isEnum.
   \param name  : std::string
     Name of the parameter
   \param value : std:: string
     proposed new value.
   \param ignored : void*
     Ignored value parameter.
   
    \return bool
    \retval true  If valid.
    \retval false If invalid.
*/
bool
CConfigurableObject::isBool(string name, string value, void* ignored)
{
  // Build the set required by isEnum:

  set<string> allowedValues;
  allowedValues.insert("true");  // True values:
  allowedValues.insert("yes");
  allowedValues.insert("1");
  allowedValues.insert("on");
  allowedValues.insert("enabled");

  allowedValues.insert("false"); // False values.
  allowedValues.insert("no");
  allowedValues.insert("0");
  allowedValues.insert("off");
  allowedValues.insert("disabled");

  return isEnum(name, value, &allowedValues);
}
/*!
   Validate an enum parameter.
   An enumerated parameter is one that can be a string drawn from a limited
   set of keywords. Validation fails of the string is not one of the
   valid keywords.
   \param name : std::string
      Name of the parameter.
   \param value : std::string
      Proposed parameter value.
   \param values : void*
      Actually an std::set<string>* where the set elements are the
      valid keywords.
  \return bool
  \retval true If valid. 
  \retval false If invalid.
*/
bool
CConfigurableObject::isEnum(string name, string value, void* values)
{
  set<string>& validValues(*(static_cast<set<string>*>(values)));

  return ((validValues.find(value) != validValues.end()) ? true : false);

}
/**
 * validate an enumerated list  - This is used by list of enumlists.
 * in that list each element is a list of enums whose elements must be validated.
 * We just bust the value up into its elements and pass each one  to isEnum,
 * requiring them all to validate.
 *
 * @param name - name of the parameter.
 * @param value - value of a list element.
 * @param values - Passed to isEnum without interpretation.
 * @return bool - true if all list elements satisfy isEnum, false if even one of them does.
 * @note we allow a short circuit failure.
 */
bool
CConfigurableObject::isEnumList(std::string name, std::string value, void* arg)
{
  // First this must be a valid list:
  
  int argc;
  const char** argv;
  if (Tcl_SplitList(nullptr, value.c_str(), &argc, &argv) != TCL_OK) {
    return false;
  }
  // To make the failure logic simpler we marshall the strings into a vector
  // of strings and then free argv: vectors and strings free normally
  
  std::vector<std::string> listElements;
  for (int i =0; i < argc; i++) {
    listElements.push_back(argv[i]);
  }
  Tcl_Free((char*)argv);
  
  // Now require each string be valid:
  
  for (auto s : listElements) {
    if (!isEnum(name, s, arg)) return false;    // a failure.
  }
  // all passed
  
  return true;
}

/*!
  Validate a floating point parameter.  Floating parameters allow the implementor of a device
  to let the user specify values in 'real units' which are then converted transparently to device
  register values.  Floating point values are actually handled as doubles, since C/C++ likes to 
  upconvert floats to doubles in any event.  The conversion fails if the string does not
  convert to a floating point value or the etire string is not used in the conversion.
  NAN's are also considered invali, and isnan is used to determine if the conversion resulted
  in an NAN.  

  The application can place limits on the floating point value as well.  See
  the parameters below:

  \param name : std::string
     Parameter name (ignored).
  \param value : std::string
     The value of the paramter.
  \param values : void*
     Actually a ppointer to a FloatingLimits type which sets the limits
     on the parameter.  If the FloatingLimits type is NULL, all floating
     values are allowed...except for NAN's... which are never allowed.

  \return bool
  \retval true  - if Valid.
  \retval false - if not Valid.

*/
bool
CConfigurableObject::isFloat(string name, string value, void* values)
{
  FloatingLimits* pLimits = static_cast<FloatingLimits*>(values);

  // Do the conversion..and check the basic legality

  const char* start(value.c_str());
  char* endptr;
  double fValue = strtod(start, &endptr);
  if((strlen(start) != (endptr - start))) {
    return false;		// Did not convert entire string e.g. 1.0garbage
  }
  if(isnan(fValue)) {
    return false;
  }

  // If a range was supplied check it:

  if (pLimits) {
    if (pLimits->first.s_checkMe) {
      if (fValue < (pLimits->first.s_value)) {
	return false;
      }
    }
    if (pLimits->second.s_checkMe) {
      if (fValue > (pLimits->second.s_value)) {
	return false;
      }
    }
  }
  
  // All tests passed:

  return true;


}



/*!
   Validate a list parameter.   To simplify; a list parameter
   is a Tcl formatted list.  List validation can fail for the following
   reasons:
   - The value is not a properly formatted list.
   - The value has too many list elements.
   - The value has too few list elements.
   - One or more of the list elements fails validation.
   \param name : std::string
      Name of the parameter being modified.
   \param value : std::string
      Value proposed for the parameter.
   \param validity : void*
      Actually an optional pointer to an isListParameter:
      - If validity is NULL, no size or element validity checking is done.
      - If validity is non-null, it's s_allowedSize element is used to 
        validate the list size, and if the s_checker.first is not null,
        it is used to validate the list elementss.

   \return bool
   \retval true List is valid.
   \retval fals List is invalid.

*/
bool
CConfigurableObject::isList(string name, string value, void* validity)
{
  // First ensure the list can be split:

  int    listSize;
  const char** list;
  int status = Tcl_SplitList((Tcl_Interp*)NULL, value.c_str(),
			     &listSize, &list);
  if (status != TCL_OK) {
    return false;
  }
  // The list is valid. Validate elements if needed:

  bool result = true;
  if (validity) {
    isListParameter& listValidity(*static_cast<isListParameter*>(validity));
    

    // Check length constraints.

    if (listValidity.s_allowedSize.s_atLeast.s_checkMe && 
	(listSize < listValidity.s_allowedSize.s_atLeast.s_value)) {
      result = false;
    }
    if (listValidity.s_allowedSize.s_atMost.s_checkMe &&
	(listSize > listValidity.s_allowedSize.s_atMost.s_value)) {
      result = false;
    }
    // If checker supplied, check all the list elements for validity.

    if (result && listValidity.s_checker.first) {
      for (int i=0; (i < listSize) && result; i++) {
	result = (*listValidity.s_checker.first)(name, list[i], 
						 listValidity.s_checker.second);
      }
    }

  }

  Tcl_Free((char*)(list));
  return result;

}
/*!
    Check that a list is a valid bool list.
    This is done by constructing the listValidity object with a list element
    checker set to the bool element checker.
    \param name : std::string
      Name of the parameter.
    \param value : std:: string
      Proposed new value.
    \param sizes : void*
      Actually a pointer to ListSizeConstraint if non null:
      - If NULL elements are checked for validity but any list size is ok.
      - If non NULL elements are checked for validity and Limits set the
        limits on size.
    \return bool
    \retval true List validates.
    \retval false List is not valid.
*/
bool
CConfigurableObject::isBoolList(string name, string value, void* size)
{
  // Set up our isListParameter struct initialized so that only
  // elements will be checked:

  isListParameter validator;
  validator.s_checker.first = isBool;
  validator.s_checker.second= NULL;

  if (size) {
    ListSizeConstraint& limits(*static_cast<ListSizeConstraint*>(size));
    validator.s_allowedSize = limits;

  }
  else {
    validator.s_allowedSize = unconstrainedSize;

  }
  return isList(name, value, &validator);
}
/*!
    Check that this is a valid list of integers.
    For string lists in this implementation we are not able to do range
    checking on the values of the list elements.  We do, however enforce
    the integer-ness of each element of the list.
    \param name : std::string
       Name of the parameter.
    \param value : std::string
       Proposed value of the parameter.
    \param sizes : void*
       Actually a pointer to a ListSizeConstraint which, if non null places
       constraints on the number of elements in the list.
    \return bool
    \retval true  - List validated.
    \retval false - List not validated

    \todo Extend the parameter to us to supply optional limit information.
*/
bool
CConfigurableObject::isIntList(string name, string value, void* size)
{
  isListParameter validator;
  validator.s_checker.first  = isInteger; // Require integer but 
  validator.s_checker.second = NULL;      // No range requirements. 

 
  

  if (size) {
    ListSizeConstraint& limits(*static_cast<ListSizeConstraint*>(size));
    validator.s_allowedSize = limits;
  }
  else {
    validator.s_allowedSize = unconstrainedSize;
  }
  return isList(name, value, &validator);
}

/**
 * isFloatList
 *    Check that a proposed list is a valid list of floating point values.
 *    The values could also be further constrained by range constraints.
 * @param name - name of the parameter.
 * @param value - proposed value for the list.
 * @param checker to a list size constraint which can put constraints on the
 *                 list sizes and optional value limits as well.
 * @return bool - true if list passed tests else false.
 */
bool
CConfigurableObject::isFloatList(std::string name, std::string value, void* checker)
{
    // we need to build the list validator for isList .first will require
    // elements be floats.  .second will be the list size contstraint
    // which can include range limits on the values if provided else
    // unconstrained.
    
    isListParameter validator;
    validator.s_checker.first = isFloat;
    validator.s_checker.second = NULL;   // no range requirement.
    validator.s_allowedSize    = unconstrainedSize;  // if not provided.
    if  (checker) {
      ListSizeConstraint& limits(*static_cast<ListSizeConstraint*>(checker));
      validator.s_allowedSize = limits;
    }
    
    return isList(name, value, &validator);
}
/*!
    Check for a string list.  String lists are allowed to have just about anything
    as element values...therefore, if the validSizes parameter is present, we'll
    do list size checking only, otherwise on list syntax checking.
    \param name : std::string
       Name of the parameter being modified.
    \param value ; std::string
       proposed new value for the parameter.
    \param validSizes : void*
       Actually a pointer to a ListSizeConstraint structure that, if non-null
       defines the list size checking that will take place.  If NULL, any sized
       list (including empty) is allowed.
    \return bool
    \retval true  - List validated.
    \retval false - List not validated

*/
bool
CConfigurableObject::isStringList(string name, string value, void* validSizes)
{
  isListParameter validator;
  validator.s_checker.first  = static_cast<typeChecker>(NULL);
  validator.s_checker.second = NULL;

  if (validSizes) {
    ListSizeConstraint& limits(*static_cast<ListSizeConstraint*>(validSizes));
    validator.s_allowedSize = limits;
  }
  else {
    validator.s_allowedSize = unconstrainedSize;
  }

  return isList(name, value, &validator);
}


////////////////////////////////////////////////////////////////////////////////////////
// Other utilities

/**
 * Create a dynamically allocated ListSizeConstraint which is automatically
 * deleted when we are destroyed.
 *
 * @param minLength - Minimum list length.
 * @param maxLength - Maximum list length
 *
 * @return ListSizeConstraint*
 * @retval Pointer to dynamically allocated list size constraint struct filled in as
 *         directed by the parameters
 *
 * @throws CErrnoException if memory allocation fails.
 */
CConfigurableObject::ListSizeConstraint* 
CConfigurableObject::createListConstraint(unsigned minLength, unsigned maxLength)
{
  ListSizeConstraint* pConstraint = 
    reinterpret_cast<ListSizeConstraint*>(malloc(sizeof(ListSizeConstraint)));

  if (!pConstraint) {
    throw CErrnoException("Allocating a list size constraint struct");
  }

  pConstraint->s_atLeast.s_checkMe = true;
  pConstraint->s_atLeast.s_value   = minLength;
  pConstraint->s_atMost.s_checkMe  = true;
  pConstraint->s_atMost.s_value    = maxLength;

  DynamicConstraint c = {
    free,
    static_cast<void*>(pConstraint)
  };
  m_constraints.push_back(c);

  return pConstraint;
}

/*!
  Helper static member function to construct an isEnumParam set
  from an array of char* pointers terminated in a NULL.

   \param values - the values to put in the set:
*/
CConfigurableObject::isEnumParameter
CConfigurableObject::makeEnumSet(const char** values)
{
  isEnumParameter result;
  while (*values) {
    result.insert(string(*values));
    values++;
  }
  return result;
}


/**
 * Convert a string to a bool...once the string has been validated.
 * @param value - Value to convert.
 * @return bool
 * @retval true - if value is in the set of strings that represent true
 * @retval false - if not.
 */
bool
CConfigurableObject::strToBool(string value)
{
  set<string> trueValues;
  addTrueValues(trueValues);

  // Is enum does what we want if we pass it the set of true values...
  // it'll give us a true if the value is in the set of legal trues and false otherwise:

  return isEnum("null", value, &trueValues);
}
// Add the legal true value strings to a set of strings:

void
CConfigurableObject::addTrueValues(set<string>& values)
{
  values.insert("true");  // True values:
  values.insert("yes");
  values.insert("1");
  values.insert("on");
  values.insert("enabled");
}
// Add the legal false value strings to a set of strings.

void
CConfigurableObject::addFalseValues(set<string>& values)
{
  values.insert("false"); // False values.
  values.insert("no");
  values.insert("0");
  values.insert("off");
  values.insert("disabled");
}

// Destroy dynamically created enum parameter checkers:

void
CConfigurableObject::deleteEnumCheckers()
{
  for (int i = 0; i < m_EnumCheckers.size(); i++) {
    delete m_EnumCheckers[i];
  }
  m_EnumCheckers.clear();
}
// Add a set of enum checkers to ours:
void
CConfigurableObject::addEnumCheckers(const EnumCheckers& rhs) 
{
  for (int i =0; i < rhs.size(); i++) {
    m_EnumCheckers.push_back(new isEnumParameter(*(rhs[i])));
  }
}


/**
 * Given a default size and limits force the default size to live inside the limits.
 */
int
CConfigurableObject::computeDefaultSize(unsigned minLength, unsigned maxLength, int defaultSize)
{
  if (defaultSize < minLength) return minLength;
  if (defaultSize > maxLength) return maxLength;
  return defaultSize;
}


/**
 * Given a string, produce a well formed Tcl list that consists of n repetitions
 * of the string.  This is used to create default lists for list parameters
 * 
 * @param value - String to repeat.
 * @param nElements - Number of elements in the list.
 * 
 * @return std::string
 * @retval string representation of the list.
 */
std::string
CConfigurableObject::simpleList(std::string value, unsigned nElements)
{
  CTCLInterpreter Interp;	// Need an interpreter for list stuff.
  CTCLObject      theList;      // Tcl_Obj that will build the list.
  theList.Bind(Interp);

  for (int i = 0; i < nElements; i++) {
    theList += value;
  }
  std::string result = theList;	// Coerce back to string rep.
  return result;

}
////////////////////////////////////////////////////////////////////////////////////////
//  Destructors for elements of the m_constraints list.
//

/*
 * Kill off an enum constraint
 */
void 
CConfigurableObject::releaseEnumConstraint(void* pConstraint)
{
  isEnumParameter *pItem = static_cast<isEnumParameter*>(pConstraint);
  if (pItem) {
    delete pItem;
  } else {
    throw std::string("releaseEnumConstraint -- constraint does not cast to an isEnumParameter");
  }
}
/*
 *  Kill off a Limits constraint.
 */
void
CConfigurableObject::releaseLimitsConstraint(void* pConstraint)
{
  Limits* pItem = static_cast<Limits*>(pConstraint);

  if (pItem) {
    delete pItem;
  } else {
    throw std::string("releaseLimitsConstraint -- constraint does not cast to a Limits object");
  }

}
/**
 * Release storage associated with dynamically created constraint checkers:
 */
void
CConfigurableObject::releaseConstraintCheckers()
{
  while (!m_constraints.empty()) {
    DynamicConstraint Item = m_constraints.front();
    (Item.s_Releaser)(Item.s_pObject); // Release the constraint.
    m_constraints.pop_front();
  }
  m_constraints.clear();
}


