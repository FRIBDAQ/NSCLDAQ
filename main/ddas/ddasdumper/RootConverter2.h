

#ifndef ROOTCONVERTER2_H
#define ROOTCONVERTER2_H

#include <string>

#ifndef CONVERTER_H
#include "Converter.h"
#endif

#include <stdint.h>

class DDASEvent;
class ddaschannel;

namespace ufmt {
    class CPhysicsEventItem;
    class RingItemFactoryBase;
}
    
/// \class RootConverter2
/** \brief Converter for data formats : NSCLDAQ 10.2-00x
 *
 * A concrete implementation of the Converter base class to 
 * support the conversion of data built with NSCLDAQ versions 
 * 10.2-00x. This class expects a ring item whose ring-item-type
 * is PHYSICS_EVENT. These are composed of a set of EVBFragment-type
 * structures that surround the type of data . These structures 
 * have the following format
 *
 *  struct data_structure {
 *      uint32_t size_in_bytes
 *      uint64_t timestamp
 *      uint32_t source_id
 *      uint32_t payload_size (bytes)
 *      uint32_t barrier_type
 *      uint32_t pixie16_data[];
 *  }
 *
 * The Pixie-16 data format is defined by the Pixie-16 digitizer
 * module. It consists of a minimum of 4 32-bit words for the 
 * Pixie16 data header followed by any subsequent optional data.
 * The optional data may include trace data, energy sum data, or QDC
 * data. The ddaschannel class is solely responsible for interpreting
 * its structure. 
 *
 * The RootConverter2 object creates a branch in the tree referenced
 * by Converter::m_treepointer named ddasevent. It holds the DDASEvent objects
 * produced by every call to the RootConverter2::DumpData(const CRingItem&) 
 * method. DDASEvent objects are really arrays of individual ddaschannel
 * objects.
 * 
*/

class RootConverter2 : public Converter
{

  private:
    ufmt::RingItemFactoryBase* m_pFactory;    // Turns bodies into ring items.
      DDASEvent *m_ddasevent;  /**< pointer to current DDASEvent object */

  public:
      /** \brief Default constructor
       * Constructs the DDASEvent object. @see Converter::Converter()
       */
    RootConverter2(ufmt::RingItemFactoryBase* pFact);

      /** \brief Default destructor
        * Calls the parent destructor and also deletes the object pointed
        * to by the m_ddasevent data member.
        * 
       */
      virtual ~RootConverter2(); 

      /** \brief Adds ddasevent branch to tree and associates it with local DDASEvent object 
       *
       * First calls the Converter::Initialize(std::string,std::string) method to create
       * the output file and the tree, then adds the ddasevent branch to the tree. This branch
       * holds the DDASEvent objects produced by the DumpData(const CRingItem&) method.
       *
       * @param filein currently not implemented
       * @param fileout name of the root file to store the outputted data tree 
       */
      virtual void Initialize(std::string filein, std::string fileout);

      /** \brief Conversion of CRingItem data and filling of the tree
       *
       * Responsible for converting the data stored in the CRingItem into the DDASEvent
       * object. This is the top level iterator through the data. It steps through the EVBFragments. 
       * For each EVBFragemt, the ExtractDDASChannelFromEVBFragment method is called and then 
       * the generated ddaschannel object is appended to the DDASEvent object. When there are
       * no more EVBFragments to process in the current item, the DDASEvent is added to the tree.
       *
       * @param item is the CRingItem (that should always be of type PHYSICS_EVENT)
       */
    virtual void DumpData(const ufmt::CPhysicsEventItem& item);

  private:

      /** \brief Stripping function to remove the EVBFragment header
       * 
       * A function to parse a single EVBFragment. The majority of the conversion is actually 
       * performed by the ddaschannel::UnpackData method and this merely strips away the 
       * EVBFragment information that is prepended to the 
       * Pixie-16 data structure. (which is what is understood by the ddaschannel::UnpackData method). 
       * In this implementation, the first six 32-bit data words are stripped away before delegating the 
       * remainder of the unpacking task to the ddaschannel object.
       *
       * @param body_ptr a pointer to the data body as a reference
       * @return the ddaschannel object containing all of the extracted Pixie-16 data
       */
      ddaschannel* ExtractDDASChannelFromEVBFragment(const uint32_t*& body_ptr);

};

#endif
