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

    def tearDown(self):
        global port

        self._receiver.close()
        self._ctx.destroy()
        port = port - 1                # In case the server socket lingers.
    
    def decodeClient(self, frame):
        cmdlen = len(frame) - 36
        fmt    = 'LLLLI%ds' % cmdlen
        producer = struct.unpack(fmt, frame)
        
        # Befrore splitting, slice the command at the \0\0 terminator.
        
        pcommand = producer[5]
        cmdend   = pcommand.find("\x00\x00")
        pcommand = pcommand[0:cmdend]
        
        command = pcommand.split("\0")
        result = list(producer[0:5])
        result.append(command)
        return  result
    
    
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
        self.assertEqual(statusmessages.MessageTypes.RING_STATISTICS , header[0])     # Type.
        self.assertEqual(statusmessages.SeverityLevels.INFO, header[1])     # Severity  == info
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
        
        obj.addProducer(programWords, 100, 1000, 10)
        obj.endMessage()
        
        frames = self._receiver.recv_multipart()
        self.assertEqual(3, len(frames))
        
        # I am only interested in the producer frame:
        
        prod = frames[2]
        producer = self.decodeClient(prod)
        self.assertEqual(100, producer[0])
        self.assertEqual(1000, producer[1])
        self.assertEqual(0, producer[2])
        self.assertEqual(10, producer[3])
        self.assertEqual(1, producer[4])
        command = producer[5]
        
        self.assertEqual(len(programWords), len(command))
        for i in range(0, len(programWords)):
            self.assertEqual(programWords[i], command[i])
            
    # Test adding client parts to ring statistics messages:
    
    def test_clientParts(self, ):
        obj = statusmessages.RingStatistics(self._uri)
        obj.startMessage('aring')
        
        program1Words = ('/usr/opt/daq/12.1/bin/ringselector', 'fox', '--sample=PHYSICS_EVENT')
        program2Words = ('/usr/opt/daq/12.1/bin/dumper', '--source=tcp://localhost/fox', "--count=10")
        
        obj.addConsumer(program1Words, 100, 1000,1234, 10)
        obj.addConsumer(program2Words, 125, 5000, 5678, 20)
        
        obj.endMessage()
    
        frames = self._receiver.recv_multipart()
        self.assertEqual(4, len(frames))   # header, ringid, 2x consumers.
        
        cons1 = frames[2]
        cons2 = frames[3]
        
        cons1Info = self.decodeClient(cons1)
        cons2Info = self.decodeClient(cons2)

        self.assertEqual(100, cons1Info[0])
        self.assertEqual(1000, cons1Info[1])
        self.assertEqual(1234, cons1Info[2])
        self.assertEqual(10, cons1Info[3])
        self.assertEqual(0, cons1Info[4])   # producer flag.
        
        self.assertEqual(125, cons2Info[0])
        self.assertEqual(5000, cons2Info[1])
        self.assertEqual(5678, cons2Info[2])
        self.assertEqual(20, cons2Info[3])
        self.assertEqual(0, cons2Info[4])
        
        
        
    
if __name__ == '__main__':
    unittest.main()


