
#include "CCBD8210CamacBranchDriver.h"
#include "CCBD8210CrateController.h"
#include "CCBD8210ReadoutList.h"
#include "CCamacBranchException.h"
#include "CTypeA1Commands.h"
#include <unistd.h>
#include <iostream>
#include <iomanip>

//#define VERBOSE 
//#define XVERBOSE 

// Transfer width 
const uint32_t CCBD8210CamacBranchDriver::BIT16 (0x000002);
const uint32_t CCBD8210CamacBranchDriver::BIT24 (0x000000);

// Address Masks
static const uint32_t SizeMask (0x000002);
static const uint32_t FMask    (0x00007C);
static const uint32_t AMask    (0x000780);
static const uint32_t NMask    (0x00F800);
static const uint32_t CMask    (0x070000);
static const uint32_t BMask    (0x380000);

// Offsets
static const uint32_t SizeOffset (1);
static const uint32_t FOffset    (2);
static const uint32_t AOffset    (7);
static const uint32_t NOffset    (11);
static const uint32_t COffset    (16);
static const uint32_t BOffset    (19);

// Internal registers
//                                 c  n a f
static const CCNAF CSR             (0,29,0,0);  // read/write 
static const CCNAF ITF             (0,29,0,4);  // write
static const CCNAF IntCont1Control (0,29,0,5);  // read/write 
static const CCNAF IntCont1Data    (0,29,0,1);  // read/write 
static const CCNAF IntCont2Control (0,29,0,6);  // read/write 
static const CCNAF IntCont2Data    (0,29,0,2);  // read/write 
static const CCNAF IntCont3Control (0,29,0,7);  // read/write 
static const CCNAF IntCont3Data    (0,29,0,3);  // read/write 
static const CCNAF CAR             (0,29,0,8);  // read/write 
static const CCNAF BTB             (0,29,0,9);  // read
static const CCNAF BZ              (0,29,0,9);  // write
static const CCNAF GL              (0,29,0,10); // read 

// Control status register CSR Masks
//
static const uint16_t IT2Mask  (0x000001);  // External interrupt
static const uint16_t IT4Mask  (0x000002);
static const uint16_t MIT2Mask (0x000004);  // External interrupt masks
static const uint16_t MIT4Mask (0x000008);
static const uint16_t MLAMMask (0x000010);
static const uint16_t MTOMask  (0x000020);
static const uint16_t SY1Mask  (0x000040);
static const uint16_t SY2Mask  (0x000080);
static const uint16_t SY3Mask  (0x000100);
static const uint16_t SY4Mask  (0x000200);
static const uint16_t SY5Mask  (0x000400);
static const uint16_t NoXMask  (0x000800);
static const uint16_t BDMask   (0x001000);
static const uint16_t TOMask   (0x002000);
static const uint16_t XMask    (0x004000);
const uint16_t CCBD8210CamacBranchDriver::QMask    (0x008000);
const uint16_t CCBD8210CamacBranchDriver::XMask    (0x004000);
const uint16_t CCBD8210CamacBranchDriver::QShift   (15);
const uint16_t CCBD8210CamacBranchDriver::XShift   (14);


// Come back to these defs
// Control status register CSR Masks
//
CCBD8210CamacBranchDriver::CCBD8210CamacBranchDriver()
 : CCamacBranchDriver()
{
}

CCBD8210CrateController* 
CCBD8210CamacBranchDriver::createCrateController(int branch, int crate)
{
    return new CCBD8210CrateController(branch, crate);
}

CCBD8210ReadoutList* 
CCBD8210CamacBranchDriver::createReadoutList(int branch, int crate)
{
    return new CCBD8210ReadoutList(branch,crate);
}

void CCBD8210CamacBranchDriver::addRead16(CCamacReadoutList& list, 
        int b, 
        int c, 
        int n, 
        int a, 
        int f)
{
    // Upcast the list to real CCBD8210ReadoutList...
    // this will throw if it cannot be done
    CCBD8210ReadoutList& convList = dynamic_cast<CCBD8210ReadoutList&>(list);
    convList.setBranchIndex(b);
    convList.setCrateIndex(c);
    convList.addRead16(n,a,f);
}

void CCBD8210CamacBranchDriver::addRead24(CCamacReadoutList& list, 
        int b, 
        int c, 
        int n, 
        int a, 
        int f)
{
    // Upcast the list to real CCBD8210ReadoutList...
    // this will throw if it cannot be done
    CCBD8210ReadoutList& convList = dynamic_cast<CCBD8210ReadoutList&>(list);
    convList.setBranchIndex(b);
    convList.setCrateIndex(c);
    convList.addRead24(n,a,f);
}

void CCBD8210CamacBranchDriver::addWrite16(CCamacReadoutList& list, 
        int b, 
        int c, 
        int n, 
        int a, 
        int f,
        uint16_t data)
{
    // Upcast the list to real CCBD8210ReadoutList...
    // this will throw if it cannot be done
    CCBD8210ReadoutList& convList = dynamic_cast<CCBD8210ReadoutList&>(list);
    convList.setBranchIndex(b);
    convList.setCrateIndex(c);
    convList.addWrite16(n,a,f, data);
}

void CCBD8210CamacBranchDriver::addWrite24(CCamacReadoutList& list, 
        int b, 
        int c, 
        int n, 
        int a, 
        int f,
        uint32_t data)
{
    // Upcast the list to real CCBD8210ReadoutList...
    // this will throw if it cannot be done
    CCBD8210ReadoutList& convList = dynamic_cast<CCBD8210ReadoutList&>(list);
    convList.setBranchIndex(b);
    convList.setCrateIndex(c);
    convList.addWrite24(n,a,f, data);
}


void 
CCBD8210CamacBranchDriver::initializeBranch(CCrateController& controller, int b)
{
   CCBD8210CrateController& ctlr 
                    = dynamic_cast<CCBD8210CrateController&>(controller); 
   
   uint16_t qx=0;
   
   int prev_c = ctlr.getCrateIndex();

   ctlr.setCrateIndex(CSR.c());
   ctlr.write16(CSR.naf(), NoXMask | MTOMask | MLAMMask | MIT2Mask | MIT4Mask, qx); 

   ctlr.setCrateIndex(ITF.c());
   ctlr.write16(ITF.naf(), uint16_t(0),qx);

   ctlr.setCrateIndex(BZ.c());
   ctlr.write16(BZ.naf(), uint16_t(0),qx);

   ctlr.setCrateIndex(prev_c);
    
   sleep(2);
}

void 
CCBD8210CamacBranchDriver::initializeCrate(CCrateController& controller, 
                                           int b, int c)
{
    CCBD8210CrateController& ctlr 
                    = dynamic_cast<CCBD8210CrateController&>(controller); 

    uint16_t cmask = (1 << c);

    uint16_t qx=0;
    uint16_t data=0;

    int prev_b = ctlr.getBranchIndex();
    int prev_c = ctlr.getCrateIndex();
    ctlr.setCrateIndex(BTB.c());

    ctlr.read16(BTB.naf(),data,qx);
    while ( (data & cmask) ==0) {
        std::cerr << "    Branch " << b << " crate " << c << " is offline";
        std::cerr << std::endl;

        // wait a bit then try again
        sleep(5);
        ctlr.read16(BTB.naf(),data,qx);
    } 
    
    // ensure the controller is going to issue commands to the proper branch
    // crate
    ctlr.setBranchIndex(b);
    ctlr.setCrateIndex(c);

    using namespace TypeA1Command;
    // issue some more initialization commands
    ctlr.control(GenDatawayC,qx);
    ctlr.control(GenDatawayZ,qx);
    ctlr.control(RemoveDatawayI,qx);

    // reset the branch/crate index to its former value
    ctlr.setCrateIndex(prev_b);
    ctlr.setCrateIndex(prev_c);

    std::cerr << "    Branch " << b << " crate " << c << " is ONLINE";
}

uint32_t 
CCBD8210CamacBranchDriver::convertToAddress(int b, int c, 
                                            int n, int a, 
                                            int f) 
{
#ifdef VERBOSE
    std::cout << "Forming command : (b,c,n,a,f) = ("; 
    std::cout << std::setw(2) << b << ",";
    std::cout << std::setw(2) << c << ",";
    std::cout << std::setw(2) << n << ",";
    std::cout << std::setw(2) << a << ",";
    std::cout << std::setw(2) << f << ")" << std::endl;
#endif

    // Set up bit fields
    uint32_t branch, crate, station, subaddr, func;

    if (isValidBCNAF(b,c,n,a,f)) {
        branch   = ((b<<BOffset) & BMask);
        crate    = ((c<<COffset) & CMask); 
        station  = ((n<<NOffset) & NMask); 
        subaddr  = ((a<<AOffset) & AMask); 
        func     = ((f<<FOffset) & FMask); 
    } else {
        throw CBadBCNAF();
    }

    uint32_t addr = 0;
    addr = (branch | crate | station | subaddr | func);
    addr |= 0x800000;
    return addr;
}


void CCBD8210CamacBranchDriver::decodeAddress(uint32_t addr)
{
    int b = ((addr & BMask) >> BOffset);
    int c = ((addr & CMask) >> COffset); 
    int n = ((addr & NMask) >> NOffset); 
    int a = ((addr & AMask) >> AOffset); 
    int f = ((addr & FMask) >> FOffset); 
    int is16 = ((addr & SizeMask) >> SizeOffset); 

    uint32_t bcnafMask = (BMask | CMask | NMask | AMask | FMask | SizeMask);
    int leftover = addr & (~bcnafMask);
    std::cout << "Decodes command : (b,c,n,a,f) = ("; 
    std::cout << std::setw(2) << b << ",";
    std::cout << std::setw(2) << c << ",";
    std::cout << std::setw(2) << n << ",";
    std::cout << std::setw(2) << a << ",";
    std::cout << std::setw(2) << f << ") ";

    if (is16) {
        std::cout << " 16-bit ";
    } else {
        std::cout << " 24-bit ";
    }
    std::cout << " Extra = 0x" << std::hex << leftover;
    std::cout << std::dec << std::endl;
}


bool 
CCBD8210CamacBranchDriver::isValidBCNAF(int b, int c, 
        int n, int a, 
        int f)
{
    bool flag = (   (b>=0 && b<8) && (c>=0 && c<8)
            && (n>=0 && n<32) && (a>=0 && a<16)
            && (f>=0 && f<32) 
            );
#ifdef XVERBOSE
    if (flag) 
        std::cout << " valid" << std::endl;
    else
        std::cout << " bad" << std::endl;
#endif
    return flag;
}
