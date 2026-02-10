
#ifndef CCBD8210CRATECONTROLLER_H
#define CCBD8210CRATECONTROLLER_H

#include "CCrateController.h"    // It's all inlines.
#include "CCBD8210ReadoutList.h"

//class CNAF;
#include "CNAF.h"
class CVMUSB;

class CCBD8210CrateController : public CCrateController 
{
    private:
    int m_branch;
    int m_crate;
    
    CVMUSB* m_ctlr;

    public:
    CCBD8210CrateController(int branch, int crate);

    int simpleWrite16(int n, int a, int f, uint16_t data, uint16_t& qx);
    int simpleWrite24(int n, int a, int f, uint32_t data, uint16_t& qx);
    int simpleRead16( int n, int a, int f, uint16_t& data, uint16_t& qx);
    int simpleRead24( int n, int a, int f, uint32_t& data, uint16_t& qx);
    int simpleControl(int n, int a, int f, uint16_t& qx);

    CCBD8210ReadoutList* createReadoutList() const;
    int executeList(CCamacReadoutList& list, void* pbuffer, size_t bufsize, size_t *nbytes);

    public:
    int getCrateIndex() const {return m_crate;}
    int getBranchIndex() const {return m_branch;}

    void setCrateIndex(int c) { m_crate = c;}
    void setBranchIndex(int b) { m_branch = b;}

    CVMUSB* getController() const { return m_ctlr;}
    void setController(CVMUSB& controller) { m_ctlr = &controller;}

    bool qTest();
    bool xTest();
    uint16_t formXQ(uint16_t x, uint16_t q) const;
};

#endif
