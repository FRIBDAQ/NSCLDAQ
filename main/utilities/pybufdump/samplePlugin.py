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


This sample will demonstrate that for simple DDAS data

Objects rather than an unbound function is used for formatting in case the formatting object
wants to have memory.

'''


#  This is the list of source ids that are DDAS modules

ddas_sources : list[int] = [0, 2]

class DDASFormatter:
    ''' Our sample formatter class.'''
    def __init__(self):
        pass                          # Any initialization is done here.
    
    def format(self, srcid : int, body : bytearray) -> str:
        ''' This does the formatting for any sources that
            we say have DDAS data  (see the ddas_sources global.)
            
            @param srcid :  int - the source id we are formatting.
            @param body  :  bytearray - the fragment body.  Note that this
                includes the ringitem header and the body header as well.
            @return str - The string we want displayed as the body of the
                 fragment.
        '''
        result = f'\nFormatting DDAS fragment body data for source {srcid}\n'
        result += f'The fragment body has {len(body)} bytes of data\n\n'
        
        return result

def registerFormatters(controller : object) -> None:
    '''
        After our plugin is loaded, this function is called and
        must register instances of DDASFormatter for each source id
        we're going to provide:
    '''
    for src in ddas_sources:
        formatter = DDASFormatter()
        controller.registerFormatter(src, formatter)