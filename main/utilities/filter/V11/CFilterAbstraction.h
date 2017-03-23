#ifndef DAQ_V11_CFILTERABSTRACTION_H
#define DAQ_V11_CFILTERABSTRACTION_H

#include <CFilterVersionAbstraction.h>
#include <V11/CRingItem.h>
#include <V11/CCompositeFilter.h>

#include <CSimpleAllButPredicate.h>

#include <memory>

namespace DAQ {

class CDataSource;
class CDataSink;
class CFilterMediator;

namespace V11 {

class CFilter;
class CFilterAbstraction;

using CFilterAbstractionUPtr = std::unique_ptr<CFilterAbstraction>;
using CFilterAbstractionPtr  = std::shared_ptr<CFilterAbstraction>;

class CFilterAbstraction : public CFilterVersionAbstraction {
private:
    CRingItem               m_item;
    CRingItem*              m_pInputItem;
    CRingItem*              m_pOutputItem;
    CCompositeFilterPtr     m_pFilter;
    CSimpleAllButPredicate  m_predicate;
    CFilterMediator*        m_pMediator;

public:
    CFilterAbstraction();
    CFilterAbstraction(const CFilterAbstraction&);
    CFilterAbstraction& operator=(const CFilterAbstraction&);
    ~CFilterAbstraction();

    virtual void readDatum(CDataSource& source);
    virtual void processDatum();
    virtual void outputDatum(CDataSink& sink);
    virtual uint32_t getDatumType() const;
    virtual void cleanUp();

    virtual void initialize();
    virtual void finalize();
    virtual void setFilterMediator(CFilterMediator& mediator);
    virtual CFilterMediator* getFilterMediator();

    virtual void setExcludeList(const std::string& excludeList);
    virtual void setSampleList(const std::string& sampleList);

    void registerFilter(CFilterPtr pFilter);
    CCompositeFilterPtr getFilter() const;
    CRingItem* dispatch(CRingItem &item);
};


} // end V11
} // end DAQ

#endif // DAQ_V11_CFILTERABSTRACTION_H
