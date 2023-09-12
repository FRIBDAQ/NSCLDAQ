/*
*-------------------------------------------------------------
 
 CAEN SpA 
 Via Vetraia, 11 - 55049 - Viareggio ITALY
 +390594388398 - www.caen.it

------------------------------------------------------------

**************************************************************************
* @note TERMS OF USE:
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License as published by the Free Software
* Foundation. This program is distributed in the hope that it will be useful, 
* but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. The user relies on the 
* software, documentation and results solely at his own risk.
*
* @file     Dig2Device.h
* @brief    Class to encapsulate the Dig2 device interface.
* @author   Ron Fox
*
*/
#ifndef DIG2DEVICE_H
#define DIG2DEVICE_H
#include <cstdint>
#include <string>
#include <cstdarg>

namespace caen_nscldaq {
    void set_tracing(bool onoff) ;
    /**
     * @class Dig2Device
     *       Encapsulates the dig2 library in a bit simpler-to-use
     *       mechanism.
     * @note at present I don't see the advantage to monitor connections
     *       so they are not supported -- though I imagine support could be added.
     */
    class Dig2Device {
    private:
        std::uint64_t m_deviceHandle;
        std::uint64_t m_endpointHandle;
    public:
        Dig2Device(const char* hostOrPid, bool isUsb = false);
        virtual ~Dig2Device();
        
        // Use overloads to make the user's life simpler:
        void SetValue(const char* parameterName, const char* value) const;
        void SetValue(const char* parameterName, int  value) const;
        void SetValue(const char* parameterName, std::uint64_t value) const;
        void SetValue(const char* parameterName, double value) const;
        void SetValue(const char* parameterName, bool value) const;

        
        void SetDeviceValue(const char* devParName, const char* value) const;
        void SetDeviceValue(const char* devParName, int value) const;
        void SetDeviceValue(const char* devParName, std::uint64_t value) const;
        void SetDeviceValue(const char* devParName, double value) const;
        void SetDeviceValue(const char* devParName, bool value) const;

        
        void SetChanValue(unsigned chan, const char* chanParName, const char* value) const;
        void SetChanValue(unsigned chan, const char* chanParName, int value) const;
        void SetChanValue(unsigned chan, const char* chanParName, std::uint64_t value) const;
        void SetChanValue(unsigned chan, const char* chanParName, double value) const;
        void SetChanValue(unsigned chan, const char* chanParName, bool value) const;

        void SetLVDSValue(unsigned quartet, const char* LVDSName, const char* value) const;
        void SetLVDSValue(unsigned quartet, const char* LVDSName, int value) const;
        void SetLVDSValue(unsigned quartet, const char* LVDSName, std::uint64_t value) const;
        void SetLVDSTriggerMask(unsigned maskNo, std::uint64_t mask) const;
        
        
        
        // Sadly we can't do the overload trick for getValue since
        // C++ does not overload solely on return value type so:
        
        std::string GetValue(const char* parameterName, const char* initial = nullptr) const;
        int         GetInteger(const char* parameterName) const;
        std::uint64_t GetULong(const char* parameterName) const;
        double      GetReal(const char* parameterName) const;
        bool        GetBool(const char* parameterName) const;

        
        std::string GetDeviceValue(const char* parameterName) const;
        int         GetDeviceInteger(const char* parameterName) const;
        std::uint64_t GetDeviceULong(const char* parameterName) const;
        double      GetDeviceReal(const char* parameterName) const;
        bool        GetDeviceBool(const char* parameterName) const;

        
        std::string GetChanValue(unsigned chan, const char* parameterName) const;
        int         GetChanInteger(unsigned chan, const char* parameterName) const;
        std::uint64_t GetChanULong(unsigned chan, const char* parameterName) const;
        double      GetChanReal(unsigned chan, const char* parameterName) const;
        bool        GetChanBool(unsigned chan, const char* parameterName) const;

        
        std::string GetLVDSValue(unsigned quartet, const char* parameterName) const;
        int         GetLVDSInteger(unsigned quartet, const char* parameterName) const;
        std::uint64_t GetLVDSULong(unsigned quartet, const char* parameterName) const;
        std::uint64_t GetLVDSTriggerMask(unsigned maskNo) const;

        
        // Digitizer commands:
        
        void Command(const char* command) const;

        
        // Acquistion -- these need a higher level of abstraction
        // than offered by felib IMHO.
        
        void SetActiveEndpoint(const char* ep) const;
        std::string GetActiveEndpoint() const;
        void SetReadDataFormat(const char* json) const;
        
   // Can we hide JSON generation?
        
        bool ReadData(int timeout, int argc, void** argv) const;
        bool hasData() const;
                             // True if a device has data.
                             
    private:
        std::string devPath(const char* devParName) const;
        std::string chanPath(unsigned chan, const char* chanParName) const;
        std::string LVDSPath(const char* lvdsParName, int quartet) const;
        std::string lastError() const;
        std::string encodeLVDSquartet(unsigned quartet, const char* value=0) const;
        std::string encodeLVDSValue(unsigned quartet, std::uint64_t value) const;
        std::uint64_t getActiveEndpointHandle() const;
        
    };
}

#endif