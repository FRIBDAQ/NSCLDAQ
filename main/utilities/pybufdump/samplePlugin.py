#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014, 2026
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#	     FRIB
#	     Michigan State University
#	     East Lansing, MI 48824-1321

'''
@file samplePlugin.py
@brief Sample plugin module that can format DDAS data
@author Ron Fox
'''


'''
A plugin has the following requirements:
-  It must have a function called registerFormatters that is called with 
   the pybufdump controller as a parameter (no need to worry about what that is
   too much).
-  It must have one or more formatting classes.
-  For each source id it wants to format, rather than using the default formatter,
   registerFormatters must invoke the controller's registerFormatter method passing
   it, in order, the source id it wants to format and an instance of the formatting
   class that will format that source id.
   
The formatting class must implement a method called format that, in addition to self,
take in order the source id of the fragment body being formatted and the fragment's body
as a bytearray object.  The formatter must return a string that will be used as the
formatted fragment body.


This sample will demonstrate that for simple DDAS data from NSCLDAQ-11.. There are differences
in the DDAS-12 format so you can't use it for that.

Objects rather than an unbound function is used for formatting in case the formatting object
wants to have memory.



'''
import struct

#  This is the list of source ids that are DDAS modules

ddas_sources : list[int] = [0, 2]

class DDAS11Formatter:
    ''' Our sample formatter class.'''
    def __init__(self,controller):
        # We save the controller in case we want to 
        # use some of its features for formatting.
        
        self._controller = controller
    
    
    def _moduletype(self, info : int) -> (int, int, int):
        # Given the digitizer informatino word,
        # return trip of (rev, bits, mhz).
        
        mhz = info & 0xffff
        bits = (info >> 16) & 0xff
        rev   = (info >> 24) & 0xff
        return (rev, bits, mhz)
    
    def _decodeHdr0(self, hdr0) -> (int, int, int, int, int):
        # Decode header 0 into event length, header length, crate, slot, chan
        
        evlen = (hdr0 >>  17) & 0x3fff
        hdrlen= (hdr0 >> 12)  & 0x1f
        crate = (hdr0 >> 8)   & 0xf
        slot  = (hdr0 >> 4)  & 0xf
        chan  = (hdr0  & 0xf)
        
        return (evlen, hdrlen, crate, slot, chan)
    
    def _decodeTimeAndCFD(self, hdr1 : int, hdr2: int) -> (bool, int, int):
        # Decode the time and cfd information in the header1/2 words.
        # Returns the cfd forced trigger flag, the cfd fractional time and
        # the timestamp:
        
        forcedCfd = True if (hdr2 & 0x8000) != 0 else False
        ts        = hdr1 | ((hdr2 & 0xffff) << 32)
        cfdfine   = (hdr2 >> 16) & 0x7fff
        return (forcedCfd, cfdfine, ts)
    def format(self, srcid : int, body : bytearray) -> str:
        ''' This does the formatting for any sources that
            we say have DDAS data  (see the ddas_sources global.)
            
            @param srcid :  int - the source id we are formatting.
            @param body  :  bytearray - the fragment body.  Note that this
                includes the ringitem header and the body header as well.
            @return str - The string we want displayed as the body of the
                 fragment.
            @note there is no DDAS data from version 10 formats, therefore we
                will have body header info but...
        '''
        result = f'\nFormatting DDAS fragment body data for source {srcid}\n'
        
        if False:                     # DEbugging.
            result += '-------\n'
            result += self._controller._formatByteArray(body)
            result += '-------\n'
            
        # Skip the ring item header and body header to get to the 
        # actual DDAS Data:
        
        bodyheader_offset = 8      # Size of ring item header.
        (body_header_size,)  = struct.unpack('<L', body[bodyheader_offset:bodyheader_offset+4])
        
        # In earlier NSCLDAQ, if there was no body header size, this field had 0 so:
        # Make it sizeof(uint32) if that's the case as it is for version 12.
        
        body_header_size  = 4 if body_header_size == 0 else body_header_size
        
        # This is the actual DDAS data:
        
        
        ddasbody = body[bodyheader_offset + body_header_size:]
        (ddasbodylongs,digitizerinfo) = struct.unpack('<LL', ddasbody[0:8])     
        ddasbodylongs = ddasbodylongs/2                          # 16 bit item size -> 32 bit item size.
    
        
        (rev, bits, mhz) = self._moduletype(digitizerinfo)
        result += f'Module is rev {rev:x}, {bits} bits wide sampling at {mhz}MHz\n'
    
        
        # Next is the 4 long word fixed pixie header:
        
        pixieHeader = struct.unpack('<LLLL', ddasbody[8:24])
        
        (evlen, hdrlen, crate, slot, chan) = self._decodeHdr0(pixieHeader[0])
        result += f'Data from crate {crate}, slot {slot}, channel {chan}\n'
        
        (cfdforced, fractionalTime, evtime) = self._decodeTimeAndCFD(pixieHeader[1], pixieHeader[2])
        if cfdforced:
            result += 'CFD had forced trigger. '
            
        # @todo - fold evtime and fractional time together.
        result += f'Coarse timestamp: {evtime}  CFD correction: {fractionalTime}\n'
        
        result += '\n\n'
        return result

def registerFormatters(controller : object) -> None:
    '''
        After our plugin is loaded, this function is called and
        must register instances of DDASFormatter for each source id
        we're going to provide:
    '''
    for src in ddas_sources:
        formatter = DDAS11Formatter(controller)
        controller.registerFormatter(src, formatter)