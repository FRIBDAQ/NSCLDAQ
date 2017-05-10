/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
       Jeromy Tompkins
       NSCL
       Michigan State University
       East Lansing, MI 48824-1321
*/

#ifndef DAQ_CTESTSOURCESINK_H
#define DAQ_CTESTSOURCESINK_H

#include "CDataSource.h"
#include "CDataSink.h"
#include <CTimeout.h>
#include <vector>

namespace DAQ {


class CTestSourceSink;
using CTestSourceSinkUPtr = std::unique_ptr<CTestSourceSink>;
using CTestSourceSinkPtr  = std::shared_ptr<CTestSourceSink>;

/*!
 * \brief A source/sink that enables testing of IO operations
 *
 * Often we have applications that are sending or reading data from
 * an IO device. This class enables testing such applications by enabling
 * the ability to load a data source with data that will be read out by
 * a subsequent read operation. Also, you can send data to the device, and
 * then read out what was sent. FOr example, you can ultimately make this happen
 * by doing:
 *
 * \code
 *    CTestSourceSink ss;
 *
 *    union IOInteger {
 *      int   asInt;
 *      char  asBytes[sizeof(int)];
 *    };
 *
 *    IOInteger x, y;
 *    x.asInt = 4;
 *
 *    ss.put(x.asBytes, sizeof(x.asBytes)];
 *
 *    ss.read(y.asBytes, sizoef(y.asBytes)];
 *
 *    cout << x.asInt << endl;
 *    cout << y.asInt << endl;
 * \endcode
 *
 *  Because you can overload the << and >> operators, you can easily make this
 *  much more intuitive to work with than the IOInteger implementation above. See
 *  the FormattedIO library for overloads of common types.
 *
 *  At the heart of this class is a vector of bytes. It is used to store data that
 *  has been put into the object. When data is read from it, the data is removed. So
 *  it acts essentially as a queue.
 *
 * \todo Implement based on a std::queue rather than a std::vector
 */
class CTestSourceSink : public CDataSource, public CDataSink
{
  private:
    std::vector<char> m_buffer;

  public:
    CTestSourceSink();
    CTestSourceSink(size_t buffer_size);
    virtual ~CTestSourceSink();

    /*! \brief Insert data into the sink
     *
     *  The data are pushed onto the back of the m_buffer.
     *
     *  \param pData  address of the block of data to write
     *  \param nBytes number of bytes to write
     */
    virtual void put(const void* pData, size_t nBytes);
    virtual void putv(const std::vector<std::pair<const void *, size_t> > &buffers);
    /*!
     * \brief Read dead from the source
     *
     *  It is the caller's responsibility to ensure that the number
     *  destination of the data being read is large enough to hold the
     *  requested data.
     *
     * \param pBuffer   location to place the data
     * \param nBytes    number of bytes to read
     *
     *  \throws std::runtime_error if insufficient data exists in buffer to satisfy request
     */
    virtual size_t timedRead(char* pBuffer, size_t nBytes, const CTimeout& );


    size_t availableData();
    void   ignore(size_t nBytes);
    size_t peek(char *pBuffer, size_t nBytes);
    size_t tell() const;

    /*!
     * \brief Access the underlying buffer
     * \return read-only reference to the data buffer
     */
    const std::vector<char>& getBuffer() const { return m_buffer; };
};


} // end DAQ
#endif

