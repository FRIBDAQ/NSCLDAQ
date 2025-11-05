#!/usr/bin/python3

''''
This program provides a utility for plotting the statistics 
database.  The idea is that you can open a database file
and get a tabbed notebook with a tab for each ringbuffer.
The initial plots for each tab will be for the most recent
24 hours of data, however tabs will also have controls
that look like this:

+---------------------------------+
| Begins from     Ends with       |
| [ QDateEdit]   [QDateEdit]      |
| () since () between () before.  |
|        [ Refresh ]              |
+---------------------------------+
initalized with since checked, the leftmost QDateEdit enabled and
initialized with now-24 hour and th right QDateEdit disabled.

Checking "betwen" enables both QDateEdits and Checking "before" enables
the right most only.

The plo section of each tab contains six plot canvases,
Each canvas plots data vs time for 

+-----------------------------------------------------+
|    Data Rate      Events in run  Event Rate         |
|    Data Volume    Data in run    Events             |
+-----------------------------------------------------+


Future:
   Add the ability to follow new data.

'''
import sys
from PyQt5.QtWidgets import (QWidget, QApplication, QDateTimeEdit,
    QPushButton, QRadioButton, QLabel, QTabWidget,
    QMenuBar, 
    QVBoxLayout, QHBoxLayout, QGridLayout,
    QAction
)
from PyQt5.QtCore import (Qt, QDate, QTime, QDateTime, QTimeZone,
  pyqtSignal)


import sqlite3
import datetime

import matplotlib.figure
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg
import pandas as pd
import numpy as np



matplotlib.use('Qt5Agg')


#  The control class for the bottom of each
#  Tabbed notebook:


class DateRange(QWidget):
    ''' 
      This widget provides a mechanism for
      choosing the range of dates for which plots are to be done.
      Properties:
        type  - One of 'since', 'between', or 'before'
                 indicating the type of date/time range
        start - QDateTime in the start date/time edit widget.
        end   - QDateTime in the end date/time widget.
      Signals:
         refresh - Refresh button clicked.
         
    '''
    refresh = pyqtSignal()
    
    def __init__(self, parent=None):
        
        
        super(DateRange, self).__init__(parent)
        
        # Make the widgets:
        
        self._begin_label = QLabel("From", self)
        self._end_label   = QLabel("To", self)
        
        self._begin       = QDateTimeEdit(self)
        self._begin.setCalendarPopup(True)
        self._end         = QDateTimeEdit(self)
        self._end.setCalendarPopup(True)
        self._end.setEnabled(False)         # Initial state is Since.
        
        self._since       = QRadioButton("Since", self)
        self._since.setChecked(True)        # INitially Since is checked.
        self._between     = QRadioButton("Between", self)
        self._before      = QRadioButton("Before", self)
        
        self._update      = QPushButton("Update", self)
        
        # Set the layout to a grid and layout the widgets:
        
        self._layout  = QGridLayout(self)
        self.setLayout(self._layout)
        
        self._layout.addWidget(self._begin_label, 0,  0)
        self._layout.addWidget(self._end_label, 0, 2)
        
        self._layout.addWidget(self._begin, 1, 0)
        self._layout.addWidget(self._end, 1, 2)
        
        self._layout.addWidget(self._since, 2, 0)
        self._layout.addWidget(self._between, 2, 1)
        self._layout.addWidget(self._before, 2, 2)
        
        self._layout.addWidget(self._update, 3, 1)
        
        # Hook in the signals (internal and external)
        
        # the check buttons update the enables (purely internal).
        
        self._since.clicked.connect(self._adjustEnables)
        self._between.clicked.connect(self._adjustEnables)
        self._before.clicked.connect(self._adjustEnables)
        
        # The "Update" button signals Refresh:
        
        self._update.clicked.connect(self.refresh)
        

    # Internal methods:
    
    def _adjustEnables(self):
      # Enable/disable the appropriate date/time edits depending on
      # the time range type selected:
      
      if self._since.isChecked():
        begin = True
        end   = False
      elif self._between.isChecked():
        begin = True
        end   = True
      elif self._before.isChecked():
        begin = False
        end   = True
      else:                     # Should not happen but...
        begin = False
        end = False
      
      self._begin.setEnabled(begin)
      self._end.setEnabled(end)
  
    # Public methods.
    #   Implement the properties:
    
    #    Type property:  
    def type(self):
      ''' Return the currently selected date/time type.''' 
      if self._since.isChecked():
        return 'since'
      if self._between.isChecked():
        return 'between'
      if self._before.isChecked():
        return 'before'    
        
    def setType(self, new):
      if new == 'since':
        self._since.setChecked(True)
      elif new == 'between':
        self._between.setChecked(True)
      elif new == 'before':
        self._before.setChecked(True)
      else:                              # Illegal value
        raise ValueError(f'{new} is not a valid time type must be one of since | between | before')
      
      #  Adjust the enables:
      
        self._adjustEnables()
      
    # Start, the start time:
      
    def start(self):
      ''' Returns the QDateTime of the start editor '''
      return self._begin.dateTime()
  
    def setStart(self, newdt):
      ''' sets the QDateTime of the start editor 
           newdt must be a QDateTime object.
      '''
      self._begin.setDateTime(newdt)
    
    # end, the end time.
    
    def end(self):
      ''' Returns the QDateTime of the end editor '''
      return self._end.dateTime()
    
    def setEnd(self, newdt):
      ''' Set tthe end date/time editor. 
              newdt - must be a QDateTime object.
      '''
      self._end.setDateTime(newdt)
      
class TimePlotWidget(FigureCanvasQTAgg):
  ''' This is a widget that will plot the time evolution of 
    One item of a time indexed pandas data frame.  The dataframe index
    is times in RFC3339 format:
    
    Public methods:
       plot - Plot a time series. given a dataframe and a data selector.
              See plot below.
    Attributes:
       title - The plot title.
       xlabel- The xaxis label.
       ylabel- The yaxis label.
              
    
  '''
  def __init__(self, parent=None):
    
    self._fig = matplotlib.figure.Figure()
    self._axis= None
    super().__init__(self._fig)

  def plot(self, frame, series):
    ''' Plots the requested series deleting any prior one
      frame is a time series data frame and series
      selects which of those will be plotted.
      The valid series are 
        volume - bytes sent to the ringbuffer.
        run_volum - Bytes sent to the ring buffer since the last BEGIN_RUN item.
        events - PHYSICS_EVENT items sent to the ringbuffer.
        run_events _ PHYSCIS_EVENT items sent to the ring buffer since the last BEGIN_RUN item.
        rate -  bytes/sec being sent to the ring.
        event_rate - PHYSICS_EVENT items/sec being sent to the ring.
        
        Naturally if the data frame is ever expanded, any column can be selected for plotting.
    '''
    
    #  Kill off any old data
    
    if self._axis is not None:
      self._fig.delaxis(self._axis)
      self._axis = None
    
    self._axis = self._fig.add_subplot()
    self._axis.plot(frame[series].index, frame[series].values)
    self.draw()                        # Draw the plot seems needed.
    
  #  Title attribute:
    
  def title(self):
    if self._axis is None:
      return None                     # There's no axis.
    else:
      return self._axis.get_title()
    
  def setTitle(self, title):
    if self._axis is None:
      raise RuntimeError("Attempted to set the title of a plot with no data")
    else:
      self._axis.set_title(title)
      self.draw()
  
  # xlabel attribute:
  
  def xlabel(self):
    if self._axis is None:
      return None
    else:
      return self._axis.get_xlabel()   
  
  def setXLabel(self, label):
    if self._axis is None:
      raise RuntimeError("Attemped to set x label of a plot with no data")
    else:
      self._axis.set_xlabel(label)
      
  # Ylabel attribute
  
  def ylabel(self):
    if self._axis is None:
      return None
    else:
      return self._axis.get_ylabel()
    
  def setYLabel(self, label):
    if self._axis is None:
      raise RuntimeError("Attempted to set y label of  plot with no data")      
    else:
      self._axis.set_ylabel(label)
      
        
# Main entry:

def initTimeChooser(window):
  # Set begin to today and the end to now.
  
  now = datetime.datetime.now()    # Beginning of today.
  
  
  today = QDate(now.year, now.month, now.day)
  
  nowtime = QTime(now.hour, now.minute, now.second, int(now.microsecond/1000))
  midnight = QTime(0, 0, 0, 0)
  
  tz = QTimeZone.systemTimeZone()
  
  begin = QDateTime(today, midnight, tz)
  end   = QDateTime(today, nowtime, tz)
  
  window.setStart(begin)
  window.setEnd(end)

def formatDateTime(dt) :
  # Given a QDateTime formats it to match
  # the format in the database which is
  # an ISODateWithMs that also needs more decimal precision
  # ns from rust 
  
  result = dt.toString(Qt.ISODateWithMs) + '000000'  # needs a timezone now.
  tzsecs = dt.timeZone().offsetFromUtc(dt)  # Seconds offset from UTC.
  hrs    = tzsecs/3600                    # Keeps the sign.
  abstzsecs = abs(tzsecs)                 # Strip the sign.
  min    = abstzsecs/60 - abs(hrs*60)
  tz = f'{int(hrs):+03d}:{int(min):02d}'
  result += tz
  
  return result
def dumpTimeChooser():
  selectionType = window.type()
  if selectionType == 'since':
    print('Select times since: ', formatDateTime(window.start()))
  elif selectionType == 'between':
    print("Select time between ", 
          window.start().toString(Qt.ISODateWithMs), " and ", 
          window.end().toString(Qt.ISODateWithMs))
  elif selectionType == 'before':
    print("Select time before ", window.end().toString(Qt.ISODateWithMs))

def plot(w, dbfile, ring):
  db = sqlite3.connect('file:' + dbfile + '?mode=ro', uri=True)
  
  # Just use all the data from the 'ron' ring for tests:
  
  stats = []
  cursor = db.cursor()
  for row in cursor.execute('''
                              SELECT timestamp, volume, run_vol, events, run_events, rate, event_rate 
                              FROM statistics
                              INNER JOIN ring_names ON statistics.ring_id = ring_names.id
                              WHERE ring_names.name = ?
                              ''', (ring,)):
      record = {'time': row[0], 'volume': row[1], 'run_volume': row[2], 
                'events' : row[3], 'run_events': row[4], 'rate': row[5], 'event_rate': row[6]
                }
      stats.append(record)
        
  # Make the times, series and data frame:
  
  
  times = pd.DatetimeIndex([ t['time'] for t in stats ])

  # Make a Series for each statistic:

  volume = pd.Series([s['volume'] for s in stats], index=times)
  run_volume = pd.Series([s['run_volume'] for s in stats], index=times)
  events = pd.Series([s['events'] for s in stats], index=times)
  run_events  = pd.Series([s['run_events'] for s in stats], index=times)
  rate  = pd.Series([s['rate'] for s in stats], index=times)
  event_rate = pd.Series([s['event_rate'] for s in stats], index=times)

  series = {'volume': volume, 
            'run_volume': run_volume, 
            'events': events, 
            'run_events': run_events, 
            'rate': rate, 
            'event_rate': event_rate}

  frame = pd.DataFrame(series)
  
  # Plot the run volume data:
  
  w.plot(frame, 'run_volume')
  w.setTitle("Run volume")
  w.setXLabel("Time")
  w.setYLabel("Bytes")
  
if __name__ == "__main__":

  app = QApplication(sys.argv)
  window = TimePlotWidget()
  
  plot(window, 'statistics.db',  'ron')
  
  window.show()
  sys.exit(app.exec())
  
  
      
      
