#!/usr/bin/env python



#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   ringstatisticstests.py
# @brief  Tests of python bindings to CRingStatisticsMessage
# @author <fox@nscl.msu.edu>

import unittest
from nscldaq.status import statusmessages
import zmq
import struct
import socket
import string

port = 29000

class TestRingStatistics(unittest.TestCase):
    def setUp(self):
        global port
        self._ctx = zmq.Context(1)
        self._uri = 'tcp://localhost:%d' % port
        self._receiver = self._ctx.socket(zmq.PULL)
        self._receiver.bind('tcp://*:%d' % port) 
        statusmessages.enableTest()
    def tearDown(self):
        global port
        statusmessages.disableTest()
        self._receiver.close()
        self._ctx.destroy()
        port = port - 1                # In case the server socket lingers.
        
    ############################  the tests ####################################
    
    def test_construct(self):
        obj = statusmessages.RingStatistics(self._uri)
    def test_construct_withApp(self):
        obj = statusmessages.RingStatistics(self._uri, 'myapplication')
    
    # Minimal message of header, ring id.
    
    def test_minimal(self):
        obj = statusmessages.RingStatistics(self._uri)
        obj.startMessage('aring')
        obj.endMessage()
        
        frames = self._receiver.recv_multipart()
        self.assertEqual(2, len(frames))  # Should have two frames.
        
        # look at the contents of the header.
        
        hdr = frames[0]
        header = struct.unpack('ii32s128s', hdr)
        self.assertEqual(0, header[0])     # Type.
        self.assertEqual(1, header[1])     # Severity  == info
        self.assertEqual('RingStatDaemon', string.rstrip(header[2], "\0"))
        self.assertEqual(socket.getfqdn(), string.rstrip(header[3], "\0"))
                         
        # look at the contents of the ring id -- need to  know how big the
        # ring name is:
    
        ringIdStruct = frames[1]
        ringNamelen   = len(ringIdStruct) - 8
        formatString = 'L%ds' % ringNamelen
        
        ringid = struct.unpack(formatString, ringIdStruct)
        self.assertEqual('aring', string.rstrip(ringid[1], "\0"))
        
    #  Add a producer part to the ring statistics message:
    
    def test_producerPart(self, ):
        obj = statusmessages.RingStatistics(self._uri)
        obj.startMessage('aring')
        
        programWords = ('/usr/opt/daq/12.1/bin/ringselector', 'fox', '--sample=PHYSICS_EVENT')
        
        obj.addProducer(programWords, 100, 1000)
        obj.endMessage()
        
        frames = self._receiver.recv_multipart()
        self.assertEqual(3, len(frames))
        
        # I am only interested in the producer frame:
        
        prod = frames[2]
        cmdlen = len(prod) - 20
        fmt    = 'LLI%ds' % cmdlen
        producer = struct.unpack(fmt, prod)
        self.assertEqual(100, producer[0])
        self.assertEqual(1000, producer[1])
        self.assertEqual(1, producer[2])
        pcommand = string.rstrip(producer[3], "\0")
        command = pcommand.split("\0")
        self.assertEqual(len(programWords), len(command))
        for i in range(0, len(programWords)):
            self.assertEqual(programWords[i], command[i])


        
        
        
        
    
    
        
        
        
    
if __name__ == '__main__':
    unittest.main()


