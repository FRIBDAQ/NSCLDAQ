#ifndef DAQ_CFILTERVERSIONABSTRACTION_H
#define DAQ_CFILTERVERSIONABSTRACTION_H

#include <memory>
#include <cstdint>

namespace DAQ {

class CDataSource;
class CDataSink;
class CFilterMediator;

// Some useful typedefs
class CFilterVersionAbstraction;
using CFilterVersionAbstractionUPtr = std::unique_ptr<CFilterVersionAbstraction>;
using CFilterVersionAbstractionPtr  = std::shared_ptr<CFilterVersionAbstraction>;


/*!
 * \brief The CFilterVersionAbstraction Interface
 *
 * The CFilterVersionAbstraction class is a purely virtual interface that defines
 * what functionality a CFilterMediator expects it to have. The order in which
 * the methods are called in CFilterMediator::mainLoop() is:
 *
 * 1. readDatum(CDataSource&)
 * 2. processDatum()
 * 3. outputDatum(CDataSink&)
 * 4. cleanUp()
 *
 * Before anything is done, initialize() is called. After all processing is done,
 * finalize() is called.
 *
 */
class CFilterVersionAbstraction {
public:
    virtual ~CFilterVersionAbstraction() {};

    /*! Read data from the source
     *
     * \param   source  the data source
     *
     */
    virtual void readDatum(CDataSource& source) = 0;

    /*!
     * \brief Process the datum read from the source
     *
     * It is expected that the derived class will pass the last read datum to
     * some kind of handler, e.g. a filter, in this method.
     */
    virtual void processDatum() = 0;

    /*!
     * \brief Output data to the data sink
     *
     * The resulting datum generated by processDatum() should be outputted to the
     * sink here.
     */
    virtual void outputDatum(CDataSink&) = 0;

    /*!
     * \brief Returns the type of the current data item
     *
     * This is NOT to be confused with the version of the data.
     *
     * \return the current type
     */
    virtual uint32_t getDatumType() const = 0;

    /*!
     * \brief Clean up
     *
     * Free memory that needs to be freed. All in all, reset the object to
     * a state in which it is ready for readDatum() to be called again.
     */
    virtual void cleanUp() = 0;

    /*!
     * \brief Store a set of data types to ignore
     *
     * \param excludeList   a list of data types
     */
    virtual void setExcludeList(const std::string& excludeList) = 0;

    /*!
     * \brief Store a set of data types that should be sampled (i.e. they can be skipped if needed)
     *
     * At the moment, there is limited support for sampling.
     *
     * \param sampleList   a list of data types
     */
    virtual void setSampleList(const std::string& sampleList) = 0;

    /*!
     * \brief Logic to be called prior to any reading, processing, or writing of data
     */
    virtual void initialize() = 0;

    /*!
     * \brief Logic to perform after all reading, processing, and writing of data has completed
     */
    virtual void finalize() = 0;

    /*!
     * \brief Cache a reference to the filter mediator that owns this
     *
     * \param mediator  the filter mediator
     */
    virtual void setFilterMediator(CFilterMediator& mediator) = 0;

    /*!
     * \return a pinter to the cached filter mediator
     */
    virtual CFilterMediator* getFilterMediator() = 0;

    /*!
     * \return the major version of the filter version abstraction
     */
    virtual int getMajorVersion() const = 0;

    virtual void setOneShotMode(int nSources) = 0;
};


} // end DAQ


#endif // CFILTERVERSIONABSTRACTION_H
