
#ifndef _CFILEDATASINK_H
#define _CFILEDATASINK_H

#include <CRingItem.h>
#include <FileDataSink.h>

using CFileDataSink = NSCLDAQ::FileDataSink<CRingItem>;

//
//#include "CDataSink.h"
//#include <unistd.h>
//#include <errno.h>
//#include <string>
//#include <CErrnoException.h>
//
/////! \brief A "file" data sink
///**!
//*   Owns and manages a general file object. The user should
//*   prefer constructing from a filename rather than a file
//*   descriptor because this reduces the risk for leaking a 
//*   file.
//*/
//class CFileDataSink : public CDataSink
//{
//private: 
//    int m_fd;  ///!< The file descriptor
//
//public:
//    /**! Constructors
//    */
//    CFileDataSink (int fd);    
//    CFileDataSink (std::string pathname);    
//
//    /**! Destructors
//    */
//    virtual ~CFileDataSink ();    
//
//private:
//    // Copy and assignment are not sensible b/c ownership
//    // of the file becomes ambiguous
//    CFileDataSink(const CFileDataSink&);
//    CFileDataSink& operator=(const CFileDataSink&);
//
//
//public:
//
//    /*
//     *  Implementation of the required interface methods
//    */
//    virtual void putItem(const CRingItem& item);
//    virtual void put(const void* pData, size_t nBytes);
//
//    /**! Flush file to syncronize
//    */
//    void flush()
//    { 
//        int retval = fsync(m_fd); 
//        if (retval<0) {
//            throw CErrnoException("CFileDataSink::flush() failed");
//        }
//    }
//
//    // Private utilities
//private:
//    bool isWritable();
//
//};

#endif
