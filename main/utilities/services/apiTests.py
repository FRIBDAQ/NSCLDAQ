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
# @file   apiTests.py
# @brief  Tests for the Python bindings to the CServiceApi.cpp class.
# @author <fox@nscl.msu.edu>



import unittest

import nscldaq.vardb.varmgr
import nscldaq.vardb.services
import tempfile
import os

class apiTests(unittest.TestCase):
    def setUp(self):
        dbInfo = tempfile.mkstemp('.db')
        fname  = dbInfo[1]
        file   = dbInfo[0]
        nscldaq.vardb.varmgr.create(fname)
        self._db = nscldaq.vardb.varmgr.Api('file:///' + fname)
        os.close(file)
        self._fname = fname
        
        self._api   = nscldaq.vardb.services.Api('file:///' + fname)
        
        
    def tearDown(self):
        os.remove(self._fname)
    
    
    # Note about the tests.  They look superficial but remember the
    # detailed API tests were done on the base C++ API.  We really just need
    # to know here that the jackets are properly marshalling parameters
    # and properly calling their analogs in the API.
    
    # Test for schema presence and create operation:
    
    def test_exists_no(self):
        self.assertFalse(self._api.exists())
    def test_create_exists(self):
        self._api.create()
        self.assertTrue(self._api.exists())
        
    # Test creating a program:
    
    def test_createprog(self):
        self._api.create()
        self._api.createProgram('atest', '/usr/bin/ls', 'ahost.nscl.msu.edu')
        self._db.cd('/Services')
        subdirs = self._db.ls()
        self.assertEquals(1, len(subdirs))
        self.assertEquals('atest', subdirs[0])
        
        self.assertEquals('/usr/bin/ls', self._db.get('atest/path'))
        self.assertEquals('ahost.nscl.msu.edu', self._db.get('atest/host'))
    
    # Test setting host name:
    
    def test_sethost(self):
        self._api.create()
        self._api.createProgram('atest', '/usr/bin/ls', 'ahost.nscl.msu.edu')
        self._api.setHost('atest', 'anotherhost.nscl.msu.edu')
        self.assertEquals('anotherhost.nscl.msu.edu', self._db.get('Services/atest/host'))
        
    def test_setprog(self):
        self._api.create()
        self._api.createProgram('atest', '/usr/bin/ls', 'ahost.nscl.msu.edu')
        self._api.setCommand('atest', '/some/thing/else')
        self.assertEquals('/some/thing/else', self._db.get('Services/atest/path'))
        
    def test_remove(self):
        self._api.create()
        self._api.createProgram('atest', '/usr/bin/ls', 'ahost.nscl.msu.edu')
        self._api.remove('atest')
        self.assertEquals(0, len(self._db.ls('/Services')))
        
    def test_listall(self):
        self._api.create()
        self._api.createProgram('atest', '/usr/bin/ls', 'ahost.nscl.msu.edu')
        self._api.createProgram('ztest', 'z', 'host')
        self._api.createProgram('qtest', 'q', 'ahost');
        
        result = self._api.list()
        self.assertEqual(
            {'atest': ('/usr/bin/ls', 'ahost.nscl.msu.edu'),
            'ztest': ('z', 'host'),
            'qtest': ('q', 'ahost')},
            result
        )
    def test_list(self):
        self._api.create()
        self._api.createProgram('atest', '/usr/bin/ls', 'ahost.nscl.msu.edu')
        
        result = self._api.listProgram('atest')
        self.assertEqual(('/usr/bin/ls', 'ahost.nscl.msu.edu'), result)
    
    def test_setEditorPos(self):
        self._api.create()
        self._api.createProgram('atest', '/usr/bin/ls', 'ahost.nscl.msu.edu')
        self._api.setEditorPosition('atest', 100, 250)
        
        self.assertEqual('100', self._db.get("/Services/atest/editorx"))
        self.assertEqual('250', self._db.get('/Services/atest/editory'))
    def test_getEditorXPos(self):
        self._api.create()
        self._api.createProgram('atest', '/usr/bin/ls', 'ahost.nscl.msu.edu')
        self._api.setEditorPosition('atest', 100, 250)
        
        self.assertEqual(100, self._api.getEditorXPosition('atest'))
    def test_getEditorYPos(self):
        self._api.create()
        self._api.createProgram('atest', '/usr/bin/ls', 'ahost.nscl.msu.edu')
        self._api.setEditorPosition('atest', 100, 250)
        
        self.assertEqual(250, self._api.getEditorYPosition('atest'))

    # Tests for the properties api:
    
    def test_setProperty_create(self):
        self._api.create()
        self._api.createProgram('atest', '/usr/bin/ls', 'ahost.nscl.msu.edu')
        self._api.setProperty("atest", "aprop", "avalue")
        self.assertEqual('avalue', self._db.get('/Services/atest/aprop'))
    
    def test_setProperty_modify(self):
        self._api.create()
        self._api.createProgram('atest', '/usr/bin/ls', 'ahost.nscl.msu.edu')
        self._api.setProperty("atest", "aprop", "avalue")
        self._api.setProperty('atest', 'aprop', 'anewvalue', False)
        self.assertEqual('anewvalue', self._db.get('/Services/atest/aprop'))
    
    def test_setProperty_nocreate(self):
        self._api.create()
        self._api.createProgram('atest', '/usr/bin/ls', 'ahost.nscl.msu.edu')
        with self.assertRaises(nscldaq.vardb.services.error) :
            self._api.setProperty('atest', 'aprop', 'avalue', False)
            
    def test_getProperty_exists(self):
        self._api.create()
        self._api.createProgram('atest', '/usr/bin/ls', 'ahost.nscl.msu.edu')
        self._api.setProperty("atest", "aprop", "avalue")
        
        self.assertEqual('avalue', self._api.getProperty('atest', 'aprop'))
        
    def test_getProperty_nox(self):
        self._api.create()
        self._api.createProgram('atest', '/usr/bin/ls', 'ahost.nscl.msu.edu')
        with self.assertRaises(nscldaq.vardb.services.error) :
            self._api.getProperty('atest', 'aprop')
    
            
if __name__ == '__main__':
    unittest.main()