// evt to root converter
//#include <string.h>
#include <iostream>
#include <stdlib.h>

#include "RootConverter2.h"
#include "DDASEvent.h"
#include "ddaschannel.h"
#include <CRingItem.h>
#include <CPhysicsEventItem.h>
#include "TTree.h"
#include "TFile.h"

#include <stdint.h>

#include <iostream>
#include <string>
#include <exception>
#include <RingItemFactoryBase.h>
#include <memory>

using namespace ufmt;

RootConverter2::RootConverter2(RingItemFactoryBase* pFact) 
:   Converter(),
    m_pFactory(pFact),
    m_ddasevent(new DDASEvent) 
{}

RootConverter2::~RootConverter2() 
{   
    if (m_fileout!=0) {
        Close();
    }
    delete m_ddasevent;
    m_ddasevent=0;
}

void RootConverter2::Initialize(std::string in, std::string fout) 
{
    // Call the Initialize method of parent class to
    // open file and create the tree.
    Converter::Initialize(in,fout);

    // Add a new branch to the tree created by the parent
    // class 
    m_treeout->Branch("ddasevent",&m_ddasevent,32000,99);

}

void RootConverter2::DumpData(const CPhysicsEventItem& item)
{
  try {
    uint32_t  bytes = item.getBodySize();
    uint32_t  words = bytes/sizeof(uint32_t);
    const uint32_t* body  = reinterpret_cast<const uint32_t*>(item.getBodyPointer());

    uint64_t processed_words=0;

    // body pointer currently points to size of entire DDAS PHYSICS_EVENT
    ++body; // increment pointer to beginning of the first EVB_FRAGMENT
    ++processed_words; 

    // body pointer currently points to first 32-bit word of first EVB_FRAGMENT
    m_ddasevent->Reset();

    ddaschannel* ddaschan = 0;

    const uint32_t* prev_body = body;
    while (processed_words<words) {
        // take data from ring and pack it into a ddaschannel object in ROOT file
        ddaschan = ExtractDDASChannelFromEVBFragment(body);
        m_ddasevent->AddChannelData(ddaschan);

        processed_words += (body - prev_body);
        prev_body = body;
    }

    if (m_treeout->Fill() <0) {
        std::cerr << "Error filling! " << std::endl;
    }
  }
  catch (std::exception& exc) {
       std::cerr << "Caught exception while unpacking : " << exc.what() << std::endl;
       std::cerr << " Processing will continue with the next Ring Item\n";
  }

}


ddaschannel* RootConverter2::ExtractDDASChannelFromEVBFragment(const uint32_t*& body_ptr)
{

    // -------------------------------------------------------------
    // EVB_FRAGMENT Body
    // First seven 32-bit words are the "header" to a PHYSICS_EVENT  
    // Skip over the first 7 32-bit words pointed to by body_ptr    
    // The body of this will be explicitly iterated through rather than 
    // using a predefined structure to decouple the implementation from
    // the version of the nscldaq.

    // body_ptr[0] = lower 32-bits of timestamp
    ++body_ptr; // skip it

    // body_ptr[1] = upper 32-bits of timestamp
    ++body_ptr; // skip it

    // body_ptr[2] = source id
    ++body_ptr; // skip it

    // body_ptr[3] = exclusive payload size (ie. size of Pixie16 data from Readout + RingItemHeader)
    ++body_ptr; // skip it

    // body_ptr[4] = barrier type (=0 for PHYSICS_EVENTs)
    ++body_ptr; // skip it

    // Use the factory to create a ring item and get a pointer to its body:
    
    const RingItem* pFragRing = reinterpret_cast<const RingItem*>(body_ptr);
    std::unique_ptr<CRingItem> pUndiff(m_pFactory->makeRingItem(pFragRing));    
    std::unique_ptr<CPhysicsEventItem> pPhysics(m_pFactory->makePhysicsEventItem(*pUndiff));
    
    const uint32_t* body = reinterpret_cast<const uint32_t*>(pPhysics->getBodyPointer());                           
                                                
    
    // The subsequent data constitute a PHYSICS_EVENT built from the
    // Readout program.  
    // 
    int64_t numw = *body; 

    ddaschannel* dchan = new ddaschannel;

    dchan->UnpackChannelData(body);
    body_ptr  += pPhysics->size()/sizeof(uint32_t);  // Point to next fragment.
    
   
    return dchan;

}
