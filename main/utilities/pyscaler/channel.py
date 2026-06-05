'''
  This module models a channel.  Channels contain the data required to keep
  track of a single scaler channel.
  
  @brief channel.py
  @brief scaler channel class.
  @author Ron Fox
'''



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
import math
OVERFLOW_CORRECTION : int = 1 << 32    # Size of a single overflow.

class Channel:
    '''
        Models a single scaler channel. The following
        readonly attributes are provided to clients.
        
        
        - total       - Total number of counts.
        - rate        - Instantaneous rate.
        - averageRate - Average count rate.
        - rateStdDev   - Standard deviation of the arverage rate.
        
        R/W attributes:
        
        - lowAlarm - low alarm rate (None if there is no low rate alarm)
        - highAlarm  - High alarm rate (None if there is no high rate alarm).
        
        
        @note for developers - there are other bits of 
              internal object data used to compute some of these
              attributes.
        @note we don't support 24 bit (CAMAC) scalers, only 32 bit scalers. 
            Although this matters only for non-incremental scalers and 24 bit 
            scalers with trash in their upper bits.
    
    '''
    def __init__(self, incremental: bool = True, lowAlarm: int | None = None, highAlaram: int | None = None):
        '''
            The constructor.  Parameters are only keywords:
            
            @param incremental - bool if true the counts in an update are incremental, that is 
                                counts since the last update. If false, they are total counts
                                over the run, note that since hardware has a fixed width,
                                it is possible for the count to overflow.   This is corrected for.
            @param lowAlarm   - Level of low alarm, or None if there isn't one.
            @param highAlarm  - Level of high alarm or None if there isn't one.
        '''
        #  Initialie all the instance data:
        
        self._isIncremental = incremental
        self._lowAlarm     = lowAlarm
        self._highAlarm    = highAlaram

        self.clear()
        
        
    #  Implement readonly atribute getters.
    #  Note some are computed:
    
    def total(self) -> int:
        return self._total
    def rate(self)  -> float:
        return self._rate
    def averageRate(self) -> float:
        if self._elapsedSeconds == 0:
            return 0.0
        else:
            return self._total/self._elapsedSeconds
    def rateStdDev(self) -> float:
        ''' Std dev of average rate'''
        if self._samples < 2:
            return 0.0
        else:
            return math.sqrt(math.fabs(float(self._sumOfSquares)/float((self._samples)) - 
                             float((self.averageRate()*self.averageRate()))))

    # Implement read/write attributes:
    
    def lowAlarm(self) -> int | None:
        return self._lowAlarm
    def setLowAlarm(self, value : int | None) -> None:
        self._lowAlarm = value
    def highAlarm(self) -> int | None:
        return self._highAlarm
    def setHighAlarm(self, value : int | None) -> None:
        self._highAlarm = value

    # Alarm tests
    
    def isLowAlarm(self) -> bool:
        ''' True if the channel is in low alarm state. '''
        if self._lowAlarm is None:
            return False
        else:
            return self._rate < self._lowAlarm
    def isHighAlarm(self) -> bool:
        ''' True if the channel is in high alarm state: '''
        if self._highAlarm is None:
            return False
        else:
            return self._rate > self._highAlarm
        
    #  Meat and potatoes:
    
    def update(self, start : float, end : float, counts : int) -> None:
        '''
            Should be called when  a scaler update occurs.
            @param start - Start of counting interval in seconds into the run.
            @param end   - End of counting interval in seconds into the run.
            @param counts- Scaler value - meaning depends on _isIncremental
            
        '''
        if self._isIncremental :
            self._updateIncremental(start, end, counts)
        else:
            self._updateNonIncremental(start, end, counts)
            
        self._elapsedSeconds = end
        self._samples += 1
        self._sumOfSquares += self._rate*self._rate
    
    def clear(self) -> None:
        '''
            Clear (e.g. begin a new run.)
        '''
        
        self._total          : int  = 0
        self._rate           : float= 0.0
        self._elapsedSeconds : int  = 0
        self._sumOfSquares   : float= 0.0
        self._samples        : int  = 0
        self._lastValue      : int  = 0      # Needed for nonincremental to detect overflows.
        self._overflows      : int  = 0      # For non-incremental # of overflows.
        
    # 'private' methods.
    
    #  Update totals and ratss for an incremental scaler.
    def _updateIncremental(self, start : float, end : float, counts : int) -> None:
        self._total += counts
        self._rate  = counts/(end - start)
        

    # Update totals and ratses for a non-incremental scaler.
    def _updateNonIncremental(self, start : float, end : float, counts : int) -> None:
        
        if counts < self._lastValue:
            self._overflows += 1
        
        next_total = self._overflows * OVERFLOW_CORRECTION + counts
        self._rate = (next_total - self._total)/(end - start)
        self._total = next_total
        self._lastValue = counts
        
        
#Tests:

if __name__ == "__main__":
    import unittest
    
    # This test is quite white box.
    class Tests(unittest.TestCase) :
        def setUp(self):
            self._incr = Channel(incremental = True)
            self._nonincr = Channel(incremental = False)
        def tearDown(self):
            self._incr = None
            self._nonincr = None
            
        def test_init_incr(self):
            incr = self._incr
            self.assertEqual(0, incr._total)
            self.assertEqual(0.0, incr._rate)
            self.assertEqual(0.0, incr._sumOfSquares)
            self.assertEqual(0, incr._samples)
            self.assertEqual(0, incr._lastValue)
            self.assertEqual(0, incr._overflows)
            self.assertTrue(incr._isIncremental)
            self.assertIsNone(incr._lowAlarm)
            self.assertIsNone(incr._highAlarm)
            
        def test_init_nonincr(self):
            nonincr = self._nonincr
            self.assertEqual(0, nonincr._total)
            self.assertEqual(0.0, nonincr._rate)
            self.assertEqual(0.0, nonincr._sumOfSquares)
            self.assertEqual(0, nonincr._samples)
            self.assertEqual(0, nonincr._lastValue)
            self.assertEqual(0, nonincr._overflows)
            self.assertFalse(nonincr._isIncremental)
            self.assertIsNone(nonincr._lowAlarm)
            self.assertIsNone(nonincr._highAlarm)
            
        def test_incr_update(self):
            o = self._incr
            o.update(10, 12, 100)
            self.assertEqual(100, o._total)
            self.assertEqual(100/2, o._rate)
            self.assertFalse(o.isLowAlarm())
            self.assertFalse(o.isHighAlarm())
            self.assertEqual(12, o._elapsedSeconds)
            self.assertEqual(1, o._samples)
            self.assertEqual(50*50, o._sumOfSquares)
            
        def test_noincr_update_nooverflow(self):    
            o = self._nonincr
            o.update(10, 12, 100)
            self.assertEqual(100, o._total)
            self.assertEqual(100/2, o._rate)
            self.assertFalse(o.isLowAlarm())
            self.assertFalse(o.isHighAlarm())
            self.assertEqual(12, o._elapsedSeconds)
            self.assertEqual(1, o._samples)
            self.assertEqual(50*50, o._sumOfSquares)
            self.assertEqual(0, o._overflows)
            
        def test_noincr_update_overflow(self):
            o = self._nonincr
            o.update(0,2, 1000)
            o.update(2, 4, 100)                      # Trigger overflow.
            self.assertEqual(1, o._overflows)
            self.assertEqual(100, o._lastValue)
            self.assertEqual(OVERFLOW_CORRECTION + 100, o._total)
            self.assertEqual((OVERFLOW_CORRECTION + 100 - 1000)/2, o._rate)
        
        def test_lowAlarm_1(self) :
            # note that incrementl/nonincremental don't differ here.:
            o = self._incr
            self.assertIsNone(o.lowAlarm())
            o.setLowAlarm(100)
            self.assertEqual(100, o.lowAlarm())
        def test_lowAlarm_2(self):
            # isLowAlamr is correct:
            
            o = self._incr
            self.assertFalse(o.isLowAlarm())
            o.setLowAlarm(100)
            o.update(0, 2, 100)      # Rate is 50.
            self.assertTrue(o.isLowAlarm())
            o.update(2, 4, 1000)     # rate is 500 > 100
            self.assertFalse(o.isLowAlarm())
            
        def test_hiAlarm_1(self):
            # Can set/get high alarm value:
            
            o = self._incr
            self.assertIsNone(o.highAlarm())
            o.setHighAlarm(1000)
            self.assertEqual(1000, o.highAlarm())
        
        def test_hiAlarm_2(self):
            # can detect high alarm trip:
                
            o = self._incr
            self.assertFalse(o.isHighAlarm())    # no alarm.
            o.setHighAlarm(1000)
            o.update(0, 2, 10000)        # Rate is 5000 > 1000
            self.assertTrue(o.isHighAlarm())
            o.update(2, 4, 1000)         # rate is 500 < 1000
            self.assertFalse(o.isHighAlarm())
            
        def test_clear(self):
            # test the clear method:
            
            incr = self._incr
            # short run:
            incr.update(0, 2, 100)
            incr.update(2, 4, 200)
            incr.update(4, 5, 50)
            
            incr.clear()
            self.assertEqual(0, incr._total)
            self.assertEqual(0.0, incr._rate)
            self.assertEqual(0.0, incr._sumOfSquares)
            self.assertEqual(0, incr._samples)
            self.assertEqual(0, incr._lastValue)
            self.assertEqual(0, incr._overflows)
            self.assertTrue(incr._isIncremental)
            self.assertIsNone(incr._lowAlarm)
            self.assertIsNone(incr._highAlarm)
        
        def test_statistics_incr(self):
            o = self._incr
            # Short run with easy to understand constant rate:
            
            o.update(0, 2, 100)
            o.update(2, 4, 100)
            o.update(4,5, 50)

            self.assertEqual(250, o.total())
            self.assertEqual(50, o.rate())
            self.assertEqual(50, o.averageRate())
            self.assertEqual(0.0, o.rateStdDev())
            
        def test_statistics_nonincr(self) :
            o = self._nonincr
            
            # We already tested that carries work
            # So we'll do a simple short run.
            
            o.update(0, 2, 100)
            o.update(2, 4, 200)
            o.update(4, 5, 250)
            self.assertEqual(250, o.total())
            self.assertEqual(50, o.rate())
            self.assertEqual(50, o.averageRate())
            self.assertEqual(0.0, o.rateStdDev())
            
    unittest.main()
            