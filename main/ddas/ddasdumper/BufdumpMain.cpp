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

#include "BufdumpMain.h"

#include <string>
#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <memory>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <sys/types.h>
#include <pwd.h>

// These are headers for the abstrct ring items we can get back from the
// factory. As new ring items are added this set of #include's must be
// updated as well as the switch statement in the processItem method.
//

#include <DataFormat.h>
#include <NSCLDAQFormatFactorySelector.h>
#include <RingItemFactoryBase.h>
#include <CRingItem.h>
#include <CAbnormalEndItem.h>
#include <CDataFormatItem.h>
#include <CGlomParameters.h>
#include <CPhysicsEventItem.h>
#include <CRingFragmentItem.h>
#include <CRingPhysicsEventCountItem.h>
#include <CRingScalerItem.h>
#include <CRingTextItem.h>
#include <CRingStateChangeItem.h>
#include <CUnknownFragment.h>

//

#include <URL.h>

#include "dumperargs.h"
#include "StringsToIntegers.h"
#include "RootConverter2.h"
#include "DataSource.h"
#include "FdDataSource.h"
#include "StreamDataSource.h"

using namespace std;
using namespace ufmt;

///////////////////////////////////////////////////////////////////////////////////
//
// Constructor and destructor
//

/*!
   Construct the object.  No real action occurs until the
   operator() is called, as all of the interseting data must be
   determined by parsing the command line arguments.
*/
BufdumpMain::BufdumpMain() :
  m_ringSource(true),
  m_pDataSource(0),
  m_skipCount(0),
  m_itemCount(0),
  m_prootconverter(0),
  m_pRingItemFactory(0)
{
}

/*!
  Destroy the object:
*/
BufdumpMain::~BufdumpMain()
{
  delete m_pDataSource;

}
///////////////////////////////////////////////////////////////////////////////
//
// Public interface:
//

/*!
   Entry point for the dumper.
   - Parse the arguments.
   - Open the data source.
   - Accept items from the ring and dump them to stdout.

   \param argc   Number of command line arguments.
   \param argv   Array of pointers to command line arguments.

   \return int
   \retval EXIT_SUCCESS - Successful execution
   \retval EXIT_FAILURE - Some problem.. or may just exit.
*/
int
BufdumpMain::operator()(int argc, char** argv)
{
  // parse the arguments:

  gengetopt_args_info parse;
  cmdline_parser(argc, argv, &parse);

  // figure out the sample/exclusion vectors:

  vector<uint16_t> sample;
  vector<uint16_t> exclude;
  vector<int>      s;
  if (parse.sample_given) {
    try {
      s = stringListToIntegers(string(parse.sample_arg));
    }
    catch (...) {
      cerr << "Invalid value for --sample, must be a list of item types was: "
	   << string(parse.sample_arg) << endl;
      return EXIT_FAILURE;
    }
    for(int i=0; i < s.size(); i++) {
      sample.push_back(s[i]);
    }
  }
  
  vector<int> e;
  if (parse.exclude_given) {
    try {
      e = stringListToIntegers(string(parse.exclude_arg));
    }
    catch (...) {
      cerr << "Invalid value for --exclude, must be a list of item types was: "
	   << string(parse.sample_arg) << endl;
      return EXIT_FAILURE;
      
    }
    for (int i = 0; i < e.size(); i++) {
      exclude.push_back(e[i]);
    }
  }   

  // figure out what sort of data source we have:

  string sourceName = defaultSource();
  if (parse.source_given) {
    sourceName = parse.source_arg;
  }  

  enum_format version = format_arg_v12;  
  // Use the optional command flag to use the appropriate converter 
  if (parse.legacy_mode_given) {
    
    std::cout << "Note legacy mode flag has been deprecated.  Use --format instead\n";
    std::cout << "Selecting --format=v10 unless --format overrides us\n";
    version = format_arg_v10;
    
  }
  // If legacy mode was given it can be overrident by --format ... if --format
  // not given but legacy mode given use that for the format rather than
  // the default --format value
  if (parse.format_given || (!parse.legacy_mode_given)) {
    version = parse.format_arg;
  }

  // Given the version string, create a an appopriate ring item factory.
  
  enum FormatSelector::SupportedVersions selectedVersion = FormatSelector::v12;
  if (version  == format_arg_v10) {
    selectedVersion = FormatSelector::v10;
  } else if (version == format_arg_v11) { 
    selectedVersion = FormatSelector::v11;
  } else if (version == format_arg_v12) {
    selectedVersion = FormatSelector::v12;
  } else {
    std::cerr << "Invalid format selector: " << parse.format_help << std::endl;
    return EXIT_FAILURE;
  } 
  m_pRingItemFactory = &FormatSelector::selectFactory(selectedVersion);
  
  // check name
  
  if (std::string("-") != sourceName) {
    m_pDataSource = new URL(sourceName);
  } else {
    m_pDataSource = new URL("file:///-");
  }
  
  // I think the root converter now can be independent of the data format
  // since it will be handed CPhysicsEventItems from the correct format type:
  
  m_prootconverter = new RootConverter2(m_pRingItemFactory);
  string filein = m_pDataSource->getPath();
  string fileout= parse.fileout_arg;
  m_prootconverter->Initialize(filein, fileout);

  DataSource* pSource;
  try {
    
    if (parse.sample_given) {
        std::cerr << "Note: --sample is deprecated!! \n";
        std::cerr << "To sample data from a ring use us in a pipeline with ringselector.\n";
    }
    
    // File data sources are file:///path and file:///-  The latter opens
    // STDIN_FILENO and makes an fd data source, the former a stream data
    // source:
    
    if (m_pDataSource->getProto() == "file") {
      if (sourceName == "-") {                        // lazy.
        pSource = new  FdDataSource(m_pRingItemFactory, STDIN_FILENO);
      } else {
        std::ifstream* pStream = new std::ifstream(m_pDataSource->getPath().c_str());
        pSource = new StreamDataSource(m_pRingItemFactory, *pStream);
      }
    } else {
      // Not using ring data source for unified event format. Use the file
      // data source and make a pipeline using ringselector | ddasdumper –
      // where the sample/exclude parameters are specified by the ringselector.
      // This is what SpecTcl does. Inform the user and exit.
      // -- ASC 4/12/23
      
      //CRingBuffer* pRing = CRingAccess::daqConsumeFrom(sourceName);
      //pSource = new RingDataSource(m_pRingItemFactory, *pRing);

      std::cerr << "This version of ddasdumper has not been built with ringbuffer data source support.\nTo read data from a ringbuffer make a pipeline using ringselector | ddasdumper -\n";
      exit(EXIT_FAILURE);
    }
  }
  catch (std::string msg) {
    std::cerr << "failed to open the data source " << sourceName << " : "
      << msg << std::endl;
      return EXIT_FAILURE;
  }

  // We can now actually get stuff from the ring... but first we need to set
  // the skip and item count:

  if (parse.skip_given) {
    if (parse.skip_arg < 0) {
      cerr << "--skip value must be >= 0 but is "
	   << parse.skip_arg << endl;
      return EXIT_FAILURE;
    }
    m_skipCount = parse.skip_arg;
  }
  
  if (parse.count_given) {
    if (parse.count_arg < 0) {
      cerr << "--count value must be >= 0 but is "
	   << parse.count_arg << endl;
      return EXIT_FAILURE;
    }
    m_itemCount = parse.count_arg;
  }

  // Skip any items that need to be skipped:

  while (m_skipCount) {
    std::unique_ptr<CRingItem> pSkip(pSource->getItem());
    m_skipCount--;
  }
  
  size_t numToDo = m_itemCount;
  bool done = false;
  while (!done) {
    std::unique_ptr<CRingItem> pItem(pSource->getItem());
    if (pItem.get()) {
      processItem(*(pItem.get()));      
      numToDo--;      
      if ((m_itemCount != 0) && (numToDo == 0)) done = true;
    } else {
      done = true;
    }
  }

  m_prootconverter->Close();

  return EXIT_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////
//
//   Utilities.

/*
** Process an item recieved from the ring.
** At this level, we just need to figure out what type of item we have
** cast construct a stand in for it, if necessary, and 
** dispatch to the appropriate dumper member function:
**
** Parameters:
**    item - Reference to the item to dump.  This is passed as an abstract
**           CRingItem but is actually a CRingItem from the underlying
**           format selected in the operator() method.
**  @note using the unified format library allows us to
**     - add back information grabbed from non PHYSICS_EVENT items.
**     - get the pointer to physics item bodies in a much simpler way
**     - Unify the converters because of that.
**     - Remove a bunch of code no longer needed.
**     
*/
void
BufdumpMain::processItem(const CRingItem& item)
{

  try {
    switch (item.type()) {
    case RING_FORMAT:
      {
	// Dont bother to try anything else if the supplied format is wrong:	
	try {
	  std::unique_ptr<CDataFormatItem> ringFormat(m_pRingItemFactory->makeDataFormatItem(item));
	  cout << ringFormat->toString() << endl;
	}
	catch (std::bad_cast e) {
	  cerr << "Unable to dump a data format item... likely you've specified the wrong --format\n";
	  m_prootconverter->Close(); // Clean this mess up
	  exit(EXIT_FAILURE);
	}
      }
      break; 
    case EVB_FRAGMENT:
    case EVB_UNKNOWN_PAYLOAD:
    case EVB_GLOM_INFO:
      break;
    case ABNORMAL_ENDRUN:
      std::cout << "Run ended abnormally\n";
      break;
    case BEGIN_RUN:
    case END_RUN:
    case PAUSE_RUN:
    case RESUME_RUN:
      {        
        std::unique_ptr<CRingStateChangeItem> stateChange(m_pRingItemFactory->makeStateChangeItem(item));
	cout <<	stateChange->toString() << endl;
        //dumpStateChangeItem(cout, *(stateChange.get()));
      }
      break;
     
    case PACKET_TYPES:
    case MONITORED_VARIABLES:
      {
        std::unique_ptr<CRingTextItem> textItem(m_pRingItemFactory->makeTextItem(item));
	cout << textItem->toString();
        //dumpStringListItem(cout, *(textItem.get()));    
      }
      break;
  
    case PERIODIC_SCALERS:
      {
        std::unique_ptr<CRingScalerItem> scaler(m_pRingItemFactory->makeScalerItem(item));
	cout << scaler->toString() << endl;
        // dumpScalerItem(cout, *(scaler.get()));
      }
      break;  
    case PHYSICS_EVENT:
      {
	std::unique_ptr<CPhysicsEventItem> pitem(m_pRingItemFactory->makePhysicsEventItem(item));
	dumpPhysicsItem(cout, *(pitem.get()));
      }
      break;
    case PHYSICS_EVENT_COUNT:
      {
        std::unique_ptr<CRingPhysicsEventCountItem> eventCount(m_pRingItemFactory->makePhysicsEventCountItem(item));
	cout << eventCount->toString();
        //dumpEventCountItem(cout, *(eventCount.get()));
      }
      break;
    default:
      {
	cout << item.toString() << endl;
        //dumpUnknownItem(cout, item);
      }
    }
  }
  catch (std::exception& e) {
    std::cerr << "Exception processing item: " << e.what() << std::endl;
  }
}
/*
** Dump a state change item.
** 
** Paramters:
**   out      - stream to which the dump should go.
**   item     - Ring state change item to dump.
**   @note Thanks to the unified formatting system and that CRingStateChangeItem
**         is, in fact, a CRingStateChangeItem from the proper format we can
**         do this for all formats:
*/
void
BufdumpMain::dumpStateChangeItem(ostream& out, const CRingStateChangeItem& item)
{
  uint32_t run       = item.getRunNumber();
  uint32_t elapsed   = item.getElapsedTime();
  string   title     = item.getTitle();
  string   timestamp = timeString(item.getTimestamp());



  out <<  timestamp << " : Run State change : ";
  switch (item.type()) {
  case BEGIN_RUN:
    out << " Begin Run ";
    break;
  case END_RUN:
    out << "End Run ";
    break;
  case PAUSE_RUN:
    out << "Pause Run ";
    break;
  case RESUME_RUN:
    out << "Resume Run ";
    break;
  }
  out << "  at " << elapsed << " seconds into the run \n";
  out << "Title    : " << title << endl;
  out << "RunNumber: " << run   << endl;
}

/*
** Dump a string list item.
** Parmeters:
**   out    - Output stream to which the item will be dumped.
**   item   - Reference to the item to dump.
**   @note item is actually a reference to a CRingTextItem fromt he appropriate
**        format so this works for all data formats we support.
*/
void
BufdumpMain::dumpStringListItem(ostream& out, const CRingTextItem& item)
{
  uint32_t elapsed  = item.getTimeOffset();
  string   time     = timeString(item.getTimestamp());
  vector<string> strings = item.getStrings();

  out << time << " : Documentation item ";
  switch (item.type()) {
   
  case PACKET_TYPES:
    out << " Packet types: ";
    break;
  case MONITORED_VARIABLES:
    out << " Monitored Variables: ";
    break;
    
  }
  out << elapsed << " seconds in to the run\n";
  for (int i = 0; i < strings.size(); i++) {
    out << strings[i] << endl;
  }

}
/*
** Dump a scaler item.
**
** Parameters:
**   out   - the file to which to dump.
**   item  - reference to the item to dump.
**   @note item is actually drawn from the the selected format.
*/
void
BufdumpMain::dumpScalerItem(ostream& out, const CRingScalerItem& item)
{
  uint32_t end   = item.getEndTime();
  uint32_t start = item.getStartTime();
  string   time  = timeString(item.getTimestamp());
  vector<uint32_t> scalers = item.getScalers();

  float   duration = static_cast<float>(end - start);

  out << time << " : Incremental scalers:\n";
  out << "Interval start time: " << start << " end: " << end << " seconds in to the run\n\n";
  


}

/*
** Dump a physics item.  
**
** Parameters:
**   out  - The output file on which to dump the item.
**   item - The item to dump
**   @note item is actualy drawn from the correct format type.
**
*/
void
BufdumpMain::dumpPhysicsItem(ostream& out, const CPhysicsEventItem& item)
{
  m_prootconverter->DumpData(item);


}
/*
** Dumps an item that describes the number of accepted triggers.
**
** Parameters:
**   out  - Where to dump the item.
**   item - The item itself.
**
*/
void
BufdumpMain::dumpEventCountItem(ostream& out, const CRingPhysicsEventCountItem& item)
{
  string   time   = timeString(item.getTimestamp());
  uint32_t offset = item.getTimeOffset();
  uint64_t events = item.getEventCount();


  out << time << " : " << events << " Triggers accepted as of " 
      << offset << " seconds into the run\n";
  out << " Average accepted trigger rate: " 
      <<  (static_cast<double>(events)/static_cast<double>(offset))
      << " events/second \n";
}
/*
**  Dump an item of some unknown type.  Just a byte-wise binary dump.
**
** Parameter:
**   out   - stream to which to dump the item.
**   item  - Item to dump.
*/
void
BufdumpMain::dumpUnknownItem(ostream& out, const CRingItem& item)
{
  uint16_t type  = item.type();
  uint32_t bytes = item.getBodySize();
  uint8_t* p     = reinterpret_cast<uint8_t*>(const_cast<CRingItem&>(item).getBodyPointer());

  out << "Unknown item type: " << type << endl;
  out << "Body size        : " << bytes << endl;
  out << "Body:\n";


  for (int i =1; i <= bytes; i++) {
    char item[16];
    sprintf(item, "%02x ", *p++);
    out << item;
    if ((i%16) == 0) {
      out << endl;
    }
  }
  out << endl;


}

/*
** Return the default source URL which is 
** tcp://localhost/username
**
*/
string
BufdumpMain::defaultSource() const
{
  string source = "tcp://localhost/";

  // now figure out who I am:

  uid_t id = getuid();
  struct passwd* p = getpwuid(id);

  source += p->pw_name;

  return source;
			
}

/*
** Returns the time string associated with some time_t
*/
string
BufdumpMain::timeString(time_t theTime) const
{

  string result(ctime(&theTime));
  
  // For whatever reason, ctime appends a '\n' on the end.
  // We need to remove that.

  result.erase(result.size()-1);

  return result;
}
