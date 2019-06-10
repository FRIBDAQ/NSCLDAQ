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
#include "CConfigurableObject.h"
#include <CConfigurationParameter.h>
#include <TCLInterpreter.h>
#include <TCLResult.h>
#include <CIntConfigParam.h>
#include <CIntArrayParam.h>
#include <CBoolConfigParam.h>
#include <CStringConfigParam.h>
#include <CStringArrayparam.h>
#include <CEnumParameter.h>


using namespace std;



/*!
  Construct a Configurable object:
  - The name is saved in m_sName.
  - The base class is initialized to create a command that is the same as
     the name.
  - The command is registered with the interpreter.
  \param rName    (const string&):
      The string that identifies this object as well as the name of the
      command that will be created.
  \param rInterp   (CTCLInterpreter):
      The TCL Interpreter on which this command will be registered.
*/
SConfigurableObject::SConfigurableObject(const string&    rName,
					CTCLInterpreter& rInterp) :
  CTCLProcessor(rName.c_str(), &rInterp),
  m_sName(rName)
{
  // Note that the list has a fully functional default construtor.

  Register();			// Register command on current interp.
}

/*!
   Destructor:   The configuration parmaters pointed to by the elements of the list
   are assumed to be dynamically allocated (as they will be if they are created with
   our convenience functions.  They must be deleted.
   The list elements themselves are assumed to be destroyed by the
   list destructors.
*/
SConfigurableObject::~SConfigurableObject()
{
  DeleteParameters();
}


/*! 
    

Processes the module's command.  The default
implementation is to look for matches of the
pArgs[1] with:
- "config" calls the module's Configure member. member function.
- "cget"    calls the module's ListConfiguration member function.
- "help"    calls the module's Usage member function.

\param rInterp CTCLInterpreter& [in] 
            Interpreter running the command.
\param rResult CTCLResult& [in]
            The result string that will be returned to the
            caller.
\param nArgs int [in]  The number of parameters on the
            command line.  Note that the first one should
            be m_sName.
\param pArgs char** [in] The command parameters.

\return  Either of:
  - TCL_OK  if the command completed properly or
  - TCL_ERROR if the command failed.  If the command fails,
              rResult will be a descriptive error followed by
              the usage information for what we know:
  
  \note
      To extend functionaly to support additional keyword,
      override this, check for your own keywords and if you 
      don't find them, call the base class member with
      the unaltered parameters.
*/
int 
SConfigurableObject::operator()(CTCLInterpreter& rInterp, 
                            CTCLResult& rResult, 
                            int nArgs, char** pArgs)  
{ 
  int nStatus = TCL_OK;
  assert(m_sName == string(*pArgs));
  nArgs--;
  pArgs++;
  
  if(nArgs) {
    // Match the command keyword against the ones we understand:
    // config, cget or help
    //
    string sCommand(*pArgs);
    pArgs++; 
    nArgs--;
    if(sCommand == string("config")) {
      nStatus = Configure(rInterp, rResult, nArgs, pArgs);
    }
    else if(sCommand == string("cget")) {
      nStatus = ListConfiguration(rInterp, rResult, nArgs, pArgs);
    }
    else if(sCommand == string("help")) {
      rResult = Usage();
    }
    else {                    // NO command match.
      rResult = m_sName;
      rResult += " unrecognized subcommand \n";
      rResult += Usage();
      nStatus  = TCL_ERROR;
      
    }
  }
  else {
    rResult  = " Insufficient parameters:\n";
    rResult += Usage();
    nStatus  = TCL_ERROR;
  }

  return nStatus;
}  

/*!  Function: 	
   int Configure(CTCLInterpreter& rInterp, CTCLResult& rResult, int nArgs, char** pArgs) 
 Operation Type:
    
Purpose: 	

Handles the "configure" command.  The default 
implementation is to assume configuration is
a set of keyword value pairs. e.g:
- -threshold 5
- -pedestals {1 2, 3 4 5 6 7 8 9}
- -subtraction enable

Each keyword is matched against the parameters in the
following order.  Naturally duplicate command keys are not a
good thing.
- m_IntParameters keys that accept single integer parameters
- m_ArrayParameters keys that accept a fixed size list of integers
- m_BoolParameters keys that accept a boolean flag.

See CConfigurationParameter CIntConfigParam CIntArrayParam CBoolConfigParam
As many configuration options as can be performed get done.
Any failures are reported by returning TCL_ERROR and placing
stuff in the results string.

\param rInterp CTCLInterpreter& [in] Interpreter running the
            command.
\param rResult CTCLResult [in] Result string that is filled in
            either by us or by the configuration dudes.
\param nArgs int [in] Count of remaining parameters. the first
      one should be the first configuration keyword.
\param pArgs char** [in] The text of the parameters.
*/
int 
SConfigurableObject::Configure(CTCLInterpreter& rInterp, 
                            CTCLResult& rResult, 
                            int nArgs, char** pArgs)  
{
  int nStatus = TCL_OK;
  while (nArgs) {
    if(nArgs < 2) {
      if(nArgs) {
	rResult += *pArgs;
      }
      rResult += " : Keyword without parameters.";
      rResult += Usage();
      return TCL_ERROR;
    }
    string   Keyword(pArgs[0]);     // Extract the command
    string   Parameter(pArgs[1]);   // and its keywords.
    nArgs -= 2;
    pArgs += 2;
    
    ParameterIterator pParam = Find(Keyword);
    if(pParam != m_Configuration.end()) {
      CConfigurationParameter* p = *pParam;
      nStatus = (*p)(rInterp, rResult, Parameter.c_str());
      if (nStatus != TCL_OK) return nStatus;    // Stop on error right away.
    }
    else {
      nStatus = TCL_ERROR;
      rResult += "Unrecognized config keyword/param pair\n";
      rResult += " Keyword: ";
      rResult += Keyword;
      rResult += " Parameter: ";
      rResult += Parameter;
      rResult += "\n";
    }
  }
  return nStatus;
}  


/*! 

Lists the current module configuration.  The default implementation
iterates through the set of m_IntParameters, m_ArrayParameters and 
m_BoolParameters producing pairs of {parametername values} such as:

- {threshold 5}
- {pedestals {1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 }}
- {subtraction on}

\note
   In the list, no distinction is made between the types of
    parameter values.  
\note
    The string representationi is used, so the value of the
    bool will not be standardized, but will be whatever was
    used (e.g. on, 1 are both possible values.
    
  \param rInterp CTCLInterpreter& [in] The interpreter that
            is running the command.
  \param rResult CTCLResult& [in] The result string that
            will contain the configuration information.
  \param nArgs int [in]  Number of parameters, should be 0.
  \param pArgs char** [in] text of parameters.
  
  \note  There should be at most 1 parameter. If present it
    is a glob pattern that will be used to filter the output.
    (essentially no parameter results in a pattern of *).
  
  \return Any of:
    -  TCL_OK the configuraton was listed 
    -  TCL_ERROR the configuration could not be listed and
            rResult is an error string.
*/
int 
SConfigurableObject::ListConfiguration(CTCLInterpreter& rInterp,
                                    CTCLResult& rResult, 
                                    int nArgs, char** pArgs)  
{ 
  int nStatus = TCL_OK;
  if(nArgs > 1) {
    rResult += " Too many parameters in cget\n";
    rResult += Usage();
    nStatus = TCL_ERROR;
  }
  else {
    string sPattern = "*";     // Assume no parameters.
    if(nArgs) {
      sPattern = *pArgs;
    }
    string listing = ListParameters( sPattern);
    rResult        = listing;
  }
  return nStatus;
} 
/*!
  Adds an arbitrary configuration parameter to the
  set of parameters recognized by the default config parser.
  This "anonymous" addition factors out the push_back
  in the functions below as well as providing for user
  defined configuration types in extensions of the system.

  \param pConfigParam : CConfigurationParameter* [in]
      The parameter to add.

  \return ParameterIterator
  \retval An iterator that 'points' to the parameter within the configuration
          database.
*/
SConfigurableObject::ParameterIterator
SConfigurableObject::AddParameter(CConfigurationParameter* pConfigParam)
{
  m_Configuration.push_back(pConfigParam);
  return Find(pConfigParam->getSwitch());
}

/*!

Adds an integer configuration parameter to the
set recognized by the default configuration parser.

 \param sParamName const string& [in]
      Name of parameter to add.
 \param nDefault int [in] = 0
      The default value of the parameter.
*/
SConfigurableObject::ParameterIterator 
SConfigurableObject::AddIntParam(const string& sParamName,
                              int nDefault)  
{ 
  CIntConfigParam* pNewParam = new CIntConfigParam(sParamName,
                                                   nDefault);
  return AddParameter(pNewParam);
}  

/*! 

Adds an array of parameters to the set of
configuration parameter that are parsed by the default
Configure function.

  \param rParamName const string& [in]
        Name of the parameter to add.
  \param nArraySize int [in]
        Size of the parameter array expected.
  \param nDefault int [in] = 0
        Default value of the array elements.
*/
SConfigurableObject::ParameterIterator 
SConfigurableObject::AddIntArrayParam(const string& rParamName, 
                                   int nArraySize,
                                   int nDefault)  
{ 
  CIntArrayParam* pNew = new CIntArrayParam(rParamName,
                                            nArraySize,
                                            nDefault);
  return AddParameter(pNew);
}  

/*!  Function: 	

Adds a boolean parameter to the set of
parameters that are recognized by the
default Configure parser.

  \param rName const string& [in]
      Name of the parameter to create.
  \param fDefault bool [in] = false
      Defaults value of the parameter.
*/
SConfigurableObject::ParameterIterator
SConfigurableObject::AddBoolParam(const string& rName, 
                               bool fDefault)  
{ 
  CBoolConfigParam* pParam = new CBoolConfigParam(rName, 
                                                  fDefault);
  return AddParameter(pParam);

}

/*!
   Adds a string parameter to the set of parameters
   recognized by this module.  A string parameter
   is a parameter with a single string valued value.
  
   \param rName (const string& [in]):
      Name of the new parameter.
*/
SConfigurableObject::ParameterIterator
SConfigurableObject::AddStringParam(const string& rName)
{
  CStringConfigParam *p = new CStringConfigParam(rName);
  return AddParameter(p);



}
/*!
  Adds a string array parameter to the set of parameters
  recognized by this module. A string array parameter has
  a parameter that is a tcl formatted list where each list element
  is an arbitrary string.
  \param rName (const string & [in]):
     The name of the configuration parameter.
  \param nArrayAzie (int [in]):
     The number of elements in the array.
*/
SConfigurableObject::ParameterIterator
SConfigurableObject::AddStringArrayParam(const string& rName,
				     int nArraySize)
{
  CStringArrayparam* p = new CStringArrayparam(rName, nArraySize);
  return AddParameter(p);

}
/*!
  Adds an enumerated parameter to the set of parameters.
  \param name    - Name of the configuration parameter.
  \param values  - Possible enumerated values and the integer values corresponding to them.
  \param default - The default value string.
*/
SConfigurableObject::ParameterIterator
SConfigurableObject::AddEnumParam(string                    name,
				  vector<pair<string, int> > values,
				  string                     Default)
{
  vector<CEnumParameter::enumeratorValue>  validValues;
  for(int i=0; i < values.size(); i++) {
    validValues.push_back(CEnumParameter::enumeratorValue(values[i].first, values[i].second));
  }
  CEnumParameter* p = new CEnumParameter(name, validValues, Default);
  return AddParameter(p);
				       
}

/*!

Returns a string describing the command usage.  
The defafult implementation produces a
string of the form:
\verbatim

m_sName config Paramdescription
m_sName cget
m_sName help

\endverbatim

Paramdescription is produced by iterating through the set of
configuration parameter descriptions and for each of them listing
the name and the type expected e.g.:
- theshold int
- pedestal {int[16]}
- subtraction on|off|enable|disable

*/
string 
SConfigurableObject::Usage()  
{
  string help;
  help  = m_sName;
  help += " config ";
  help += ListKeywords();
  help += "\n";
  help += m_sName;
  help += " cget ?pattern?\n";
  help += m_sName;
  help += " help";
  
  return help;
}  

/*!

   \return SConfigurableObject::ParameterIterator
   Returns a begin of loop iterator that 'points' to the first configuration
   parameter.
*/
SConfigurableObject::ParameterIterator
SConfigurableObject::begin()
{
  return m_Configuration.begin();
}

/*!
    \return SConfigurableObject::ParameterIterator
    Returns an end of loop iterator that points just off the end of the
    m_Configuration collection.
*/
SConfigurableObject::ParameterIterator
SConfigurableObject::end()
{
  return m_Configuration.end();
}

/*!
    \return int
      Returns the number of configuration parameters that are in the 
      m_Configuration collection.
*/
int
SConfigurableObject::size()
{
  return m_Configuration.size();
}

/*!
  Produces a list of the configuration parameters that match 
the input pattern.  
\param sPattern const string& [in]
      The glob string pattern.
*/

string
SConfigurableObject::ListParameters(const string& rPattern)
{
  CTCLString result;
  ParameterIterator p = m_Configuration.begin();
  while(p != m_Configuration.end()) {
    if(Tcl_StringMatch( ((*p)->getSwitch().c_str()),
                       rPattern.c_str())) {
      result.StartSublist();
      result.AppendElement((*p)->getSwitch());
      result.AppendElement((*p)->getOptionString());
      result.EndSublist();
    }
    p++;
  }
  return string((const char*)result);
}
/*!
    List the allowed configuration keywords.  The words
    are returned as  a string of pairs.  The pairs are 
    \em not a bracketed list, but just a pair of words.
    the first word of each pair is the command keyword. 
    the second word is the parameter format as returned
    from CConfigurationParameter::GetParameterFormat()

  \return string - the results of the list.
*/
string
SConfigurableObject::ListKeywords()
{
  string result;
  ParameterIterator p = m_Configuration.begin();
  while(p != m_Configuration.end()) {
    result += (*p)->getSwitch();
    result += " ";
    result += (*p)->GetParameterFormat();
    result += " ";
    
    p++;
  }
  return result;
}


/*!
    Locate a parameter matching the configuration parameter
  test string.
  \param rKeyword The keyword to check.
*/
SConfigurableObject::ParameterIterator
SConfigurableObject::Find(const string& rKeyword)
{
  ParameterIterator p = m_Configuration.begin();
  while(p != m_Configuration.end()) {
    CConfigurationParameter* pParam = *p;
    if(pParam->Match(rKeyword)) { 
      return p;
    }
    p++;
  }
  return m_Configuration.end();
}

/*!
   Get the value of an integer configuration parameter  Note
   that a string exception is thrown if the parameter
   does not exist.
  
   @param name
      Name of the parameter to fetch.
   \return int
   \retval value of the parameter.
*/
int 
SConfigurableObject::getIntegerValue(string name)
{
  ParameterIterator p = FindOrThrow(name, "integer");
  CIntConfigParam* pInt = (CIntConfigParam*)*p;
  return pInt->getOptionValue();
}
/*!
   Get the value of a boolean configuration parameter.
   Note that a string exception is thrown if the parameter does not
   exist.
   @param name 
      Name of the parameter whose value needs to be fetched.
   \return bool
   \retval the value of the parameter.
*/
bool
SConfigurableObject::getBoolValue(string name)
{
  ParameterIterator p = FindOrThrow(name, "boolean");
  CBoolConfigParam* pInt = (CBoolConfigParam*)*p;
  return pInt->getOptionValue();

}
/*!
  Returns a pointer to an integer array parameter.
  Since array parameters are not single valued, it's
  best to return the entire object, and let the caller
  pick out what' needed.
  @param name
     Name of the paramter to fetch.
  \return CIntArrayParam*
  \retval  Pointer to the integer config param.  Note that this has
          array like semanntics.
*/
CIntArrayParam*
SConfigurableObject::getIntArray(string name)
{
  ParameterIterator p = FindOrThrow(name, "integer []");
  return (CIntArrayParam*)*p;
}
/*!
  Return the value associated with an enumerated config parameter.
*/
int
SConfigurableObject::getEnumValue(string name)
{
  ParameterIterator p = FindOrThrow(name, "enum");
  CEnumParameter* pConfig = reinterpret_cast<CEnumParameter*>(*p);
  return pConfig->GetEnumValue();
}


/////////////////////// Utility functions.
/*!
   Delete the parameter arrays.
*/
void
SConfigurableObject::DeleteParameters()
{
  // Kill off the bool parameters.
  
  ParameterIterator p = m_Configuration.begin();
  while (p != m_Configuration.end()) {
    CConfigurationParameter *pParam = *p;
    delete pParam;
    p++;
  }
  if(!m_Configuration.empty()) {
    m_Configuration.erase(begin(), end());
  }
}

/*!
  Find the named parameter or throw a string exception.
*/
SConfigurableObject::ParameterIterator
SConfigurableObject::FindOrThrow(string name, string type)
{
  ParameterIterator p = Find(name);
  if( p == end()) {
    string msg("Could not find ");
    msg += type;
    msg += " parameter : ";
    msg += name;
    throw msg;
  }
  return p;
}
