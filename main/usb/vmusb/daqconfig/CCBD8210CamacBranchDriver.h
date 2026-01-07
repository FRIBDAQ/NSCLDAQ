
#ifndef CCBD8210CAMACBRANCHDRIVER
#define CCBD8210CAMACBRANCHDRIVER

#include <stdint.h>
#include "CCamacBranchDriver.h"

#include "CCBD8210CrateController.h"
#include "CCBD8210ReadoutList.h"
#include "CCamacBranchException.h"

#include <CBCNAF.h> 


class CCBD8210CamacBranchDriver : public CCamacBranchDriver
{
    public:
    static const uint32_t BIT16;
    static const uint32_t BIT24;
    static const uint16_t QMask;
    static const uint16_t XMask;
    static const uint16_t QShift;
    static const uint16_t XShift;

    public:
    CCBD8210CamacBranchDriver();

    public:
    CCBD8210CrateController* createCrateController(int branch, int crate);
    CCBD8210ReadoutList* createReadoutList(int branch, int crate);

    // Provide the proper camac interface
    void addRead16(CCamacReadoutList& list, int b, int c, 
                   int n, int a, int f);

    void addRead24(CCamacReadoutList& list,  int b, int c, 
                   int n, int a, int f);

    void addWrite16(CCamacReadoutList& list,  int b, int c, 
                    int n, int a, int f, uint16_t data);

    void addWrite24(CCamacReadoutList& list,  int b, int c, 
                    int n, int a, int f, uint32_t data);

    static uint32_t convertToAddress(int b, int c, int n, 
                                     int a, int f);

    static void decodeAddress(uint32_t addr);

    void initializeBranch(CCrateController& controller, int b);     
    void initializeCrate(CCrateController& controller, int b, int c);     

    private:
    static bool isValidBCNAF(int b, int c, int n, int a, int f);
};

#endif
