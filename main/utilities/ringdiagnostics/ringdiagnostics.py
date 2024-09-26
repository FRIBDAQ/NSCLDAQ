#!/usr/bin/env python3


'''
  ringdiagnostics
   
   Usage
      $DAQBIN/ringdiagnostics [option...]
    Options:
        --update -u - takes an integer parameter that is the
           seconds between updates.  Defaults to 5.  Should be
           greater than 0.
           
           Note that
           the timer used to do the update is reset after an update
           finishes so it should not be possible to be 'too small'.
           
           
'''
from nscldaq import RingUsage
from nscldaq import  RingView
from nscldaq import RingModel
import argparse
import sys

from PyQt5.QtWidgets import QApplication, QMainWindow
from PyQt5.QtCore import QTimer

##
#  Updater
#   This class updates the display.  While a singleshot
#   timer it reschedules itself periodically.
#
#  Instantiate with the update rate in ms and
#  the model we need to put the data in.
# 

class Updater(QTimer):
    def __init__(self, update, model, *args):
        super().__init__(*args)

        self._update = update
        self._model  = model
        
        self.timeout.connect(self._update_model)

        self._schedule()
        
    def _update_model(self):
        usage = RingUsage.systemUsage()
        self._model.update(usage)
        self._schedule()

    def _schedule(self):
        self.setInterval(self._update)
        self.start()
        
        
parser = argparse.ArgumentParser(
    prog='ringdiagnostics',
    description='Diagnoses dataflow bottlenecks in NSCLDAQ',
)
parser.add_argument(
    '-u', '--update', type=int, 
    dest='update',
    default='2',
    help='Seconds between updates (defaults to 2)'
)
parser.add_argument(
    '-p', '--alarm_percentage', type=int, 
    dest='alarm',
    default='90',
    help='Backlog percentage that constitutes a stall (default to 90)'
)

args = parser.parse_args()

update_ms = args.update * 1000   # Milliseconds between updates
alarm_pct = args.alarm

if alarm_pct >= 100 or alarm_pct <= 0:
    print(
        f"Invalid alarm percentage must in the range (0,100) was {alarm_pct}",
        file=sys.stderr
    )
    sys.exit(-1)
    

app = QApplication(['ringview test'])
mw = QMainWindow()
tree = RingView.RingView()
contents = RingModel.RingModel(tree, alarm_pct)
usage = RingUsage.systemUsage()
contents.update(usage)
auto_update = Updater(update_ms, contents)
mw.setCentralWidget(tree)
mw.show()
app.exec()