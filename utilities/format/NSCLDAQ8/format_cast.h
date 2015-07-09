#ifndef FORMAT_CAST_H
#define FORMAT_CAST_H

#include <CRawBuffer.h>
#include <CScalerBuffer.h>
#include <CPhysicsEventBuffer.h>
#include <CV8Buffer.h>
#include <DataFormatV8.h>

#include <typeinfo>

namespace DAQ
{
  namespace V8 {

    /*! \brief Cast operator for casting to or from a CRawBuffer (i.e. GENERIC)
     *
     */
    template<class T> T format_cast(const CV8Buffer& anyBuffer)
    {
      T newItem;

      if ((newItem.type() == GENERIC) ) {

        CRawBuffer buffer(gBufferSize);
        anyBuffer.toRawBuffer(buffer);

        return buffer;

      } else {
        if (anyBuffer.type() == GENERIC) {

          const CRawBuffer& rawBuffer = dynamic_cast<const CRawBuffer&>(anyBuffer);

          return T(rawBuffer);

        } else {
          // In case someone tries to cast from a type to the same type.
          // This will either succeed or throw a std::bad_cast.
          const T& specificType = dynamic_cast<const T&>(anyBuffer);

          return T(specificType);
        }
      }


    // we really should never reach this point
      return newItem;
    }
  }
}

#endif // FORMAT_CAST_H
