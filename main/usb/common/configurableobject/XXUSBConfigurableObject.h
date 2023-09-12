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

#ifndef XXUSB_CCONFIGURABLEOBJECT_H
#define XXUSB_CCONFIGURABLEOBJECT_H

// Necessary includes (kept to a minimum using forward class defs).

#include <string>
#include <map>			// also defines std::pair
#include <vector>
#include <set>
#include <list>
#include <cstdint>

// Typedefs for the parameter checker are in the global namespace:

typedef bool (*typeChecker)(std::string name, std::string value, void* arg);


// This namespace disambiguates this class from the  CConfigurableObject
// hiding in libTclPlus TODO - merge the capabilities of that class.

namespace XXUSB {

/*!
   Configurable object consist of a name and a configuration.
   A configuration is a set of name value pairs.  Each name value
   pair can have an associated validity checker who is responsible
   for ensuring that for any configuration value a candidate new value
   is legal.  Type checkers are just unbound functions, or static
   member functions.  See
   the typedef for typeChecker above.
 
   For convenience, some of the static functions of this class provide
   commonly used type checkers.

   How to use this:

   In general, one would subclass this object.  The subclassed object would 
   at some point call addParameter a number of times to provide the configuration 
   parameters needed by the subclass.  

   The configuration subsystem would at some point call configure to set up
   the configuration (note subclassed objects can also call this to set up their
   default configuration).

   The data taking system would then call subclass functions to inform it that
   configuration is complete.  The subclass then would call cget to get and
   react to its configuration.

   This class does not implement any policy about how the configuration is gotten.
   This can be  done by manual parsing of data files, by internal data structures,
   by a Tcl interpreter reading scripts or any other practical means.
*/
class CConfigurableObject {

  // public typedefs:

public:
  typedef std::pair<typeChecker, void*>         TypeCheckInfo;	// Type checker + arg.
  typedef std::vector<std::pair<std::string, std::string> > ConfigurationArray;
  
  // Arg types for the standard type checkers.

  struct  limit {
    bool   s_checkMe;
    std::int64_t    s_value;
    limit() :
      s_checkMe(false) 
    {}
    limit(std::int64_t value) : 
      s_checkMe(true), 
      s_value(value) {}
  } ;
  typedef std::pair<limit, limit>   Limits;
  typedef std::set<std::string> isEnumParameter;
  typedef struct _ListSizeConstraint {
    limit s_atLeast;
    limit s_atMost;
  } ListSizeConstraint;

  struct isListParameter {
    ListSizeConstraint s_allowedSize;
    TypeCheckInfo      s_checker;
    isListParameter(limit atLeast, limit atMost, TypeCheckInfo checker) {
      s_allowedSize.s_atLeast = atLeast;
      s_allowedSize.s_atMost  = atMost;
      s_checker               = checker;
    }
    isListParameter(ListSizeConstraint limits, TypeCheckInfo checker) {
      s_allowedSize = limits;
      s_checker     = checker;
    }
    isListParameter() {}  
  };

  typedef void(*ConstraintFreer)(void*); // Free function for dynamic constraint objects.
  typedef struct _DynamicConstraint {	 // 
    ConstraintFreer s_Releaser;
    void*           s_pObject;
  } DynamicConstraint, *pDynamicConstraint;

  struct  flimit {
    bool   s_checkMe;
    float s_value;
    flimit() :
      s_checkMe(false) {}
    flimit(float value) :
      s_checkMe(true), 
      s_value(value) {}
  };

  typedef std::pair<flimit, flimit> FloatingLimits;
  typedef std::vector<isEnumParameter*>         EnumCheckers;

  // Internal typedefs
private:
  typedef std::pair<std::string, TypeCheckInfo> ConfigData;
  typedef std::map<std::string, ConfigData>     Configuration;
  typedef Configuration::iterator               ConfigIterator;

  typedef std::list<DynamicConstraint>         ConstraintReleaseList;

private:
  std::string               m_name;	        //!< Name of this object.
  Configuration             m_parameters;	//!< Contains the configuration parameters.
  ConstraintReleaseList     m_constraints;      //!< Dynamically created constraints.
  EnumCheckers    m_EnumCheckers;

public:
  // Canonicals..

  CConfigurableObject(std::string name);
  CConfigurableObject(const CConfigurableObject& rhs);
  virtual ~CConfigurableObject();
  CConfigurableObject& operator=(const CConfigurableObject& rhs);
  int operator==(const CConfigurableObject& rhs) const;
  int operator!=(const CConfigurableObject& rhs) const;

  //  Selectors:

public:
  std::string getName() const;
	void setName(const char* pName) {m_name = pName; }
  std::string cget(std::string name) ;
  ConfigurationArray cget();

  // Convenience functions that get common conversions:
	//Note the return types assume int64_t and uint64_t are at most long longs wide.

  std::int64_t              getIntegerParameter (std::string name);
  std::uint64_t     getUnsignedParameter(std::string name);
  bool             getBoolParameter    (std::string name);
  double           getFloatParameter   (std::string name);
  std::vector<int64_t> getIntegerList      (std::string name);
	std::vector<uint64_t> getUnsignedList (std::string name);
  std::vector<std::string> getList     (std::string name);
	std::vector<double> getFloatList     (std::string name);    //
	std::vector<bool>  getBoolList(std::string name);
  int              getEnumParameter(std::string name, const char**pValues);
	std::vector<std::vector<std::string>> getListOfLists(std::string name);
	
    
  // Operations:

public:
  // Establishing the configuration:

  void addParameter(std::string name, typeChecker checker, void* arg,
		    std::string defaultValue = std::string(""));

  void clearConfiguration();

  // Manipulating and querying the configuration:

  void configure(std::string name, std::string value);


  // Convenience methods for creating typical parameter typse:
  
  void addIntegerParameter(std::string name, std::int64_t defaultVal = 0);
  void addIntegerParameter(std::string name, std::int64_t low, std::int64_t high, std::int64_t defaultVal = 0);

  void addBooleanParameter(std::string name, bool defaultVal = true);

  void addEnumParameter(std::string name,
			const char** pValues,
			std::string defaultValue = std::string(""));
	void addEnumListParameter(
			std::string name,
			const char** pValues, const char* defaultValue,
			unsigned minlength, unsigned maxlength, int defaultSize = -1
	);
	void addListOfEnumLists(
		std::string name, const char** pValues, const char* defaultValue,
		unsigned minlength, unsigned maxlength, int defaultSize = -1);
	void addFloatParameter(std::string name, double defaultValue = 0.0);             
	void addFloatParameter(
			std::string name, double low, double high, double defaultValue = 0.0        
	);

  void addBoolListParameter(std::string name, unsigned size, bool defaultVal = true);
  void addBoolListParameter(std::string name, unsigned minLength, unsigned maxLength, 
			    bool defaultVal = true, int defaultSize = -1);

  void addIntListParameter(std::string name, unsigned size, std::int64_t defaultVal = 0);
  void addIntListParameter(std::string name, unsigned minlength, unsigned maxLength,
			   std::int64_t defaultVal = 0, int defaultSize = -1);
  void addIntListParameter(std::string name, std::int64_t minValue, std::int64_t maxValue,
                           unsigned minlength, unsigned maxLength, unsigned defaultSize,
			   std::int64_t defaultVal);
  void addFloatListParameter(                                                    
			std::string name, unsigned minlen, unsigned maxlen, unsigned defaultsize,
			double defaultValue = 0.0
	);
	void addFloatListParameter(                                                   
			std::string name, double low, double high,
			unsigned minlen, unsigned maxlen, unsigned defaultlen,
			double defaultValue = 0.0
	);

  void addStringListParameter(std::string name, unsigned size, std::string defaultVal = "");
  void addStringListParameter(std::string name, unsigned minLength, unsigned maxLength,
			      std::string defaultVal =  "", int defaultLength = -1);


  // common type checkers:

  static bool isInteger(std::string name, std::string value, void* arg);
  static bool isBool(   std::string name, std::string value, void* arg);
  static bool isEnum(   std::string name, std::string value, void* arg);
  static bool isEnumList(std::string name, std::string value, void* arg); // elements of list of enum list
	static bool isFloat(  std::string name, std::string value, void* arg);
  static bool isList(   std::string name, std::string value, void* arg);
  static bool isBoolList(std::string name,std::string value, void* arg);
  static bool isIntList(std::string name, std::string value, void* arg);
	static bool isFloatList(std::string name, std::string value, void* arg);
  static bool isStringList(std::string name, std::string value, void* arg);
	
	
	

  // Utilities:

  // Build enum set from a list of char*'s:

  static isEnumParameter makeEnumSet(const char** values);

  // Utility to convert a validated string to a bool:

  static bool strToBool(std::string value);

private:
  ListSizeConstraint* createListConstraint(unsigned minLength, unsigned maxLength);
  int                 computeDefaultSize(unsigned minLength, unsigned maxLength, int defaultSize);
  std::string         simpleList(std::string value, unsigned nElements);
  static void addTrueValues(std::set<std::string>& values);
  static void addFalseValues(std::set<std::string>& values);
  void        deleteEnumCheckers();
  void        addEnumCheckers(const EnumCheckers& rhs);
  void        releaseConstraintCheckers();


  // Constraint releasers.
private:
  static void releaseEnumConstraint(void* pConstraint);
  static void releaseLimitsConstraint(void* pConstraint);
};

};
#endif
