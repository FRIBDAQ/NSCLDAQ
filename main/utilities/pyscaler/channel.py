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
        if self._samples == 0:
            return 0.0
        else:
            return sqrt(self._sumOfSquares/(self._samples-1) - self._total/self._elapsedSeconds)

    # Implement read/write attributes:
    
    def lowAlarm(self) -> int | None:
        return self._lowAlaram
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
            self._UpdateNonIncremental(start, end, counts)
            
        # Update for eventual std dev computation.
        # This is common code once the rates have been computed.
        
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