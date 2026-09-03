##
# @file
# @brief
#
#
import unittest
from test import support
from LogBook import LogBook
import os


class aTest(unittest.TestCase) :
    def setUp(self) :
        if os.path.isfile('logbook.log') :
           os.unlink('logbook.log') 
        LogBook.create('logbook.log', '0400x', 'Ron Fox', 'Python tests')
        self.logbook = LogBook.LogBook('logbook.log')
    
    def tearDown(self) :
        os.unlink('logbook.log')
        
    def test_exist1(self) :
        self.assertTrue(self.logbook.kv_exists('experiment'))
                    
    def test_exist2(self):
        self.assertFalse(self.logbook.kv_exists('nosuchkey'))
        
    def test_get1(self, ):
        self.assertEqual('Ron Fox', self.logbook.kv_get('spokesperson'))
    def test_get2(self, ):
        self.assertRaises(LogBook.error, self.logbook.kv_get, 'nosuchkey')
        
    def test_set1(self, ):
        self.logbook.kv_set('experiment', 'e17011')
        self.assertEqual('e17011', self.logbook.kv_get('experiment'))
    def test_set2(self, ):
        self.logbook.kv_set('newkey', 'created')
        self.assertEqual('created', self.logbook.kv_get('newkey'))
    
    def test_create1(self, ):
        self.logbook.kv_create('newkey', 'created')
        self.assertEqual('created', self.logbook.kv_get('newkey'))
    def test_create2(self, ):
        self.assertRaises(LogBook.error, self.logbook.kv_create, 'experiment', 'illigal')
    
    def test_list(self):
        info = self.logbook.kv_list()
        self.assertEqual(4, len(info))
        self.assertEqual(
            (('experiment', '0400x'), ('spokesperson', 'Ron Fox'), ('purpose', 'Python tests'), ('version', '1.0')),
            info
        )
        
    

if __name__ == '__main__' :
    unittest.main()
    