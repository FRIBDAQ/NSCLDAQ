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
    QPushButton, QRadioButton, QLabel, QTabWidget, QMessageBox,
    QVBoxLayout,  QGridLayout
)
from PyQt5.QtCore import (Qt, QDate, QTime, QDateTime, QTimeZone,
  pyqtSignal)


import sqlite3
import datetime

import matplotlib.figure
import matplotlib.pyplot as plt
import matplotlib.dates as mdate
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg
import pandas as pd
import numpy as np
from zoneinfo import ZoneInfo


#matplotlib.rcParams['timezone'] = 'EST'
matplotlib.use('Qt5Agg')
plt.style.use('seaborn-v0_8-whitegrid')

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
    
  #  private methods:
  
  
  # Public methods

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
      self._fig.delaxes(self._axis)
      self._axis = None
    
    self._axis = self._fig.add_subplot()
    
    tz =  ZoneInfo('localtime')
    
    self._axis.xaxis_date(tz)
    
    self._xformat = mdate.DateFormatter('%m/%d/%Y\n%I:%M', tz=tz)
    spacing = np.round(np.linspace(0, len(frame[series].index)-1, 5)).astype(int)
    date_ticks = frame[series].iloc[spacing].index
    
    frame[series].plot(ax=self._axis,  xticks=date_ticks, use_index=True)
    self._axis.xaxis.set_major_formatter(self._xformat)
    
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
      plt.subplots_adjust()
      self.draw()
      
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
      self.draw()
      
class RingStatisticsPlot(QWidget):
  ''' 
  This class encapsulates the 6 TimePlotWidgets needed to
  plot ring statistics for one ring buffer.  It is laid out like this:
  
  +---------------------------------------------------------------+
  |                Ring buffer name  label                        |
  |   Plot area with 6 TimePlotWidgets.                           |
  |                                           |
  +---------------------------------------------------------------+
  
  The intent, in this application, is that the widget be put in a 
  page of the tabbed notebook so strictly speaking the ringbuffer name
  may not be required.  For the contents of the TimePlotWidgets,
  see the update method documentation.
  
  Attributes:
      ringbuffer:  Ring buffer name that is displayed inthe ring buffer label and 
                    should be used in database queries.
    Public Methods:
    update       - Plots are regenerated from a new dataframe of series.
  '''
  
  def __init__(self, parent=None):
    super(RingStatisticsPlot, self).__init__(parent)
    
    # Make the widgets:
    
    self._ringname = QLabel(self)
    
    self._volume_plot  = TimePlotWidget(self)
    self._run_vol_plot = TimePlotWidget(self)
    self._events_plot  = TimePlotWidget(self)
    self._run_events_plot = TimePlotWidget(self)
    self._data_rate_plot  = TimePlotWidget(self)
    self._event_rate_plot = TimePlotWidget(self)
    
  
    
    # Layout with an overall VBox layout but the
    # plots are in a  grid layout.
    
    layout = QVBoxLayout()
    self.setLayout(layout)
    layout.addWidget(self._ringname)
    
    plot_layout = QGridLayout()
    plot_layout.addWidget(self._run_events_plot, 0, 0)
    plot_layout.addWidget(self._data_rate_plot, 0, 1)
    plot_layout.addWidget(self._event_rate_plot, 0, 2)
    
    plot_layout.addWidget(self._volume_plot, 1, 0)
    plot_layout.addWidget(self._run_vol_plot, 1, 1)
    plot_layout.addWidget(self._events_plot, 1, 2)
    
    layout.addLayout(plot_layout)
    
  # Slots:
  
  def _updateRelay(self):
    self.refresh.emit(self)
    
  # Implement attributes:
  
  # rinbuffer attribute
  def ringbuffer(self):
    ''' return the contents of the ring name label: '''
    return self._ringname.text()
  
  def setRingbuffer(self, ring):
    ''' set value of ring name label to the 'ring' parameter value '''
    self._ringname.setText(ring)
    
  
  # Public methods:
  
  def update(self, frame):
    ''' 
       Updates the plots in accordance with the data frame passed in.  All 6 plots are 
       updated.
       
       frame is a data frame of an dict of series indexed by date/time.  The keys are:
        'volume'     - the data volume
        'run_volume' - the data volume since the last BEGIN_RUN item.
        'events'     - Number of events.
        'run_events  - Number of events since the last BEGIN_RUN item.
        'rate'       - data rate in bytes/sec.
        'event_rate  - event rate.
        
        
        Note event is defined as PHYSICS_EVENT ring items.

    '''
    
    self._run_events_plot.plot(frame, 'run_events')
    self._data_rate_plot.plot(frame, 'rate')         # Top row.
    self._event_rate_plot.plot(frame, 'event_rate')
    
    self._volume_plot.plot(frame, 'volume')
    self._run_vol_plot.plot(frame, 'run_volume')
    self._events_plot.plot(frame, 'events')
    
    #  Now label the plots and their axes:
    
    self._run_events_plot.setTitle('Events in the run')
    self._run_events_plot.setXLabel("Date/Time")
    self._run_events_plot.setYLabel('Events')
    
    self._data_rate_plot.setTitle("Data rate")
    self._data_rate_plot.setXLabel('Date/Time')
    self._data_rate_plot.setYLabel('Bytes/sec')
    
    self._event_rate_plot.setTitle("Event rate")
    self._event_rate_plot.setXLabel("Date/Time")
    self._event_rate_plot.setYLabel("Events/sec")
    
    self._volume_plot.setTitle("Data Volume")
    self._volume_plot.setXLabel('Date/Time')
    self._volume_plot.setYLabel('Bytes')
    
    self._run_vol_plot.setTitle('Data Volume in run')
    self._run_vol_plot.setXLabel('Date/Time')
    self._run_vol_plot.setYLabel('Bytes')
    
    self._events_plot.setTitle("Total Events")
    self._events_plot.setXLabel('Date/Time')
    self._events_plot.setYLabel('Events')
    

# Global level functios

def createInitialTimes():
  ''' Create an initial start/stop time string for the DateRange widget
    start will be midnight today and end will be 'now'.
  '''
  now = datetime.datetime.now()    # Beginning of today.
  
  
  today = QDate(now.year, now.month, now.day)
  
  nowtime = QTime(now.hour, now.minute, now.second, int(now.microsecond/1000))
  midnight = QTime(0, 0, 0, 0)
  
  tz = QTimeZone.systemTimeZone()
  
  begin = QDateTime(today, midnight, tz)
  end   = QDateTime(today, nowtime, tz)
  
  return (begin, end)


def formatDateTime(dt) :
  '''
   Given a QDateTime formats it to match
   the format in the database which is
   an ISODateWithMs that also needs more decimal precision
   ns from rust 
  '''
  result = dt.toString(Qt.ISODateWithMs) + '000000'  # needs a timezone now.
  tzsecs = dt.timeZone().offsetFromUtc(dt)  # Seconds offset from UTC.
  hrs    = tzsecs/3600                    # Keeps the sign.
  abstzsecs = abs(tzsecs)                 # Strip the sign.
  min    = abstzsecs/60 - abs(hrs*60)
  tz = f'{int(hrs):+03d}:{int(min):02d}'
  result += tz
  
  return result
   
def connect_database(name):
  ''' connect to the database 'name' and return the connection object. '''
  db = sqlite3.connect('file:' + name + '?mode=ro', uri=True)
  return db

#  Generic plot function, given the database connection,
#  The SQL and its parameters, run the query
#  Create the data frame and plot:

def doPlot(w, db, sql, params):
  '''
    Given some sql query from the statistics table do a plot for the results of that query
    in the widget w.
  '''
  cursor = db.cursor()
  stats = []
  for row in cursor.execute(sql, params):
    record = {'time': row[0], 'volume': row[1], 'run_volume': row[2], 
                'events' : row[3], 'run_events': row[4], 'rate': row[5], 'event_rate': row[6]
                }
    stats.append(record)
    
  
  # Make the times, series and data frame. Note
  # THe timstamps are in seconds since the unix epoch.
  
  if len(stats) < 2 :
    return
  
  times =  [datetime.datetime.fromtimestamp(t['time'], ZoneInfo('UTC'))  for t in stats ] #t['time'][:32] + t['time'][33:]
  #times = pd.to_datetime(times, unit='s').tz_localize('UTC')
  
  

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
  w.update(frame)

#  Plot data since a time: 

def plotFrom(w, db, ring_name, fromDate):
  ''' Update a pot from a specified date.'''
  sql = '''
    SELECT timestamp, volume, run_vol, events, run_events, rate, event_rate 
    FROM statistics
    INNER JOIN ring_names ON statistics.ring_id = ring_names.id
    WHERE ring_names.name =? AND timestamp > unixepoch(?)
  '''
  params = (ring_name, fromDate)
  doPlot(w, db, sql, params)
  
  
# Plot data before an end time:

def plotBefore(w, db, ring_name, beforeDate):
  ''' Update a plot with data prior to an end date  '''
  sql = '''
    SELECT timestamp, volume, run_vol, events, run_events, rate, event_rate 
    FROM statistics
    INNER JOIN ring_names ON statistics.ring_id = ring_names.id
    WHERE ring_names.name =? AND timestamp < unixepoch(?)
  '''
  params = (ring_name, beforeDate)
  doPlot(w, db, sql, params)
  
  
def plotBetween(w, db, ring_name, startDate, endDate):
  ''' Update a plot between two dates '''
  sql = '''
  SELECT timestamp,  volume, run_vol, events, run_events, rate, event_rate 
    FROM statistics
    INNER JOIN ring_names ON statistics.ring_id = ring_names.id
    WHERE ring_names.name =? AND 
          timestamp BETWEEN unixepoch(?) AND unixepoch(?)
  '''
  params=(ring_name, startDate, endDate)
  doPlot(w, db, sql, params)
  
def refreshPlots():
  ''' Refresh all plots with a new date/time range.  '''
  tab_no = tabs.currentIndex()
  
  if tab_no != -1:
    cursor = tabs.cursor()
    tabs.setCursor(Qt.WaitCursor)
    db = sqlite3.connect('file:' + dbFile + '?mode=ro', uri=True)
    range_type = dateTimeRange.type()
    
    # Iterate over the tabs now -1 means no widget.
  
  
    tab_widget = tabs.widget(tab_no)
    ring_name = tab_widget.ringbuffer()  
    if range_type == 'since':
      plotFrom(tab_widget, db, ring_name, formatDateTime(dateTimeRange.start()))
    elif range_type == 'before':
      plotBefore(tab_widget, db, ring_name, formatDateTime(dateTimeRange.end()))
    elif range_type == 'between':
      w = dateTimeRange       # For notational convenience.
      plotBetween(tab_widget, db, ring_name, formatDateTime(w.start()),  formatDateTime(w.end()))
    else:
      print("Invalid type range type: ", range_type)
      
      
    db.close()
    tabs.setCursor(cursor)
  
# For the signal handler:

def updateplot(index):
  refreshPlots()
  
# Entry point.
  
if __name__ == "__main__":
  
  if len(sys.argv) != 2:
    print("Specify a data base file and a ring")
    sys.exit(-1)
  
  dbFile = sys.argv[1]
  
  #  Get a list of the ring buffers in the database.
  
  
  app = QApplication(sys.argv)
  
  # The top level window is a QTabWidget with tabs containing
  # RingStatisticsPlot widgets for each ringbuffer:
  
  window = QWidget()
  window.setWindowTitle("Ring buffer history plots powered by Qt5")
  layout = QVBoxLayout()
  window.setLayout(layout)
  
  
  tabs = QTabWidget(window)
  layout.addWidget(tabs)
  
  (begin, end) = createInitialTimes()
  dateTimeRange = DateRange(window)
  dateTimeRange.setStart(begin)
  dateTimeRange.setEnd(end)
  
  layout.addWidget(dateTimeRange)
  
  
  
  db = connect_database(dbFile)
  cursor = db.cursor()
  for row  in cursor.execute(
    ''' SELECT name FROM ring_names ORDER BY name ASC
    ''', []
    ):
    ring = row[0]
    plot = RingStatisticsPlot(tabs)
    tabs.addTab(plot, ring)
    plot.setRingbuffer(ring)
    
  refreshPlots()       # initial refresh.
  dateTimeRange.refresh.connect(refreshPlots)
  
  # If the tab changes, then refresh the current widget too:
  
  tabs.currentChanged.connect(updateplot)   # index expected as a parameter.
  
  db.close()

  window.show()
  sys.exit(app.exec())
  
  
      
      
