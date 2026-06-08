'''
    This module provides plotting for pyScaler.   An arbitrary number
    of plotlines can be shown on the same plot.  We are actually
    relatively agnostic about what the data mean.  However the
    rendition of the plot is a strip chart.
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

import numpy as np

from matplotlib.backends.backend_qtagg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qtagg import NavigationToolbar2QT as NavigationToolbar
from matplotlib.figure import Figure

# 2. Import Qt Widgets dynamically using Matplotlib's compatibility layer
from matplotlib.backends.qt_compat import QtWidgets

#
#  The plot line colors are selected from these:
#  If there are more plots than the colors listed:
#  1. the graph will be confusing.
#  2. but the colors will cycle back circularly.
#
_COLORS=['black', 'red', 'blue', 'green', 'cyan', 'yellow', 'magenta', 'purple', 'chocolate', 'olive']

class ScalerStripChart(QtWidgets.QWidget):
    '''
        This widget is a stripchart for multiple plotlines.
        Key methods:
        
        * add_plotline(name : str) -> self
            Add a new plotline to the widget.  The color will be chosen by 
            the widget.
              
        * add_point(name : str, seconds : float, value: float) -> self
            Add a time series point to the named plotline.
            Note that if needed the plot is scrolled so that the
            desired time window is maintained.
        * clear() -> self
            Clears all ploints from all plotlines.
        * setWindow(seconds -> int) -> self
            Defines the window of the strip chart.
        * setMaxPoints(max : int) -> self
            Defines the maximum number of points the plotlines can have.  Above this number,
            points to the left of the window are decimated.  Decimation picks every other
            point and removes it from the plot line.
            @todo make the decimation factor settable.
            @note the default decimation with 2 second intervals (the most popular) results in
            an undecimated plot set of 55.5 hours.  This should be adequate for many 
            experiments which usually have run lengths measured in a few hours.
        * plotlines() -> list[str] 
            Returns the list of plot line names
        * window() -> int
            Returns the window size.
        * maxPoints() -> int
            returns the decimation threshold.
        * decimationFactor() -> int
            Returns the decimation factor.  Currently this is hard coded to 2, meaning decimation
            involves removing every other point older than most_recent - window.
    
    '''
    
    def __init__(self, parent = None):
        super().__init__(parent)
        #
        # Make the stuff we need to create the widdget.
        #
        self._figure = Figure(figsize=(5,4), dpi=100)
        self._canvas = FigureCanvas(self._figure)
        self._toolbar = NavigationToolbar(self._canvas, self)
        
        #  Lay it out.
        
        layout = QtWidgets.QVBoxLayout(self)
        layout.addWidget(self._toolbar)
        layout.addWidget(self._canvas)
    
    
# Testing

if __name__ == "__main__":
    import sys
    app = QtWidgets.QApplication(sys.argv)
    
    # Create the custom widget and show it
    window = ScalerStripChart()
    window.setWindowTitle("Matplotlib in QWidget")
    window.resize(800, 600)
    window.show()
    
    sys.exit(app.exec())