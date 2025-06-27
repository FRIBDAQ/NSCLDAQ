import matplotlib
matplotlib.use("Qt5Agg")
import matplotlib.pyplot as plt
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg
import numpy as np
import inspect
import copy
import sys
import logging
from time import sleep
from math import ceil, floor

from PyQt5.QtWidgets import QWidget, QVBoxLayout, QLabel, QMessageBox
from PyQt5.QtGui import QPaintEvent

from fit_panel import FitPanel

import xia_constants as xia
from run_type import RunType

##
# @todo (ASC 8/21/24): Remove the custom binning. There is DSP for energy
# histogram binning, most people do not use it. Allow users to configure
# that via a tab or spinbox when starting a run? What effect does it have
# if set to some strage value for production data (should be none, as it
# effects onboard histos only)?
#

class Plot(QWidget):
    """Plotting widget for the GUI utilizing the matplotlib Qt5 backend.

    Attributes
    ----------
    figure : pyplot figure 
        matplotlib pyplot figure instance.
    canvas : FigureCanvasQTAgg 
        Canvas the figure renders onto imported from Qt5 backend.
    fit_panel : QWidget 
        Trace and histogram fitting GUI widget.
    fit_factory : FitFactory 
        Factory method for fitting plot data.
    toolbar : NavigationToolbar2QT
        Figure navigation toolbar imported from Qt5 backend.
    logger : Logger
        QtScope Logger object.
    raw_data : dict
        Dictionary of default-binned, raw data from the digitizers keyed by 
        the subplot index on which it is displayed. This data maintains the 
        same size as the data array returned from the digitizers if the 
        histograms are rebinned.
    bin_width : int
        Bin width in ADC units/bin.
    mgr : DSPManager
        DSP manager for calls to XIA API.

    Methods
    -------
    draw_trace_data(data, nrows=1, ncols=1, idx=1)
        Draw single- or multuple-channel ADC trace data.
    draw_analyzed_trace(trace, fast_filter, cfd, slow_filter)
        Draw a single-channel ADC trace and its filter outputs. 
    draw_run_data(data, run_type, nrows=1, ncols=1, idx=1)
        Draw single- or multuple-channel energy histogram or baseline data.
    on_begin_run(run_type)
        Draw a blank histogram-style canvas when starting a histogram run.
    update_canvas()
        Redraw the entire canvas and all its subplots.
    rebin()
        Rebin existing single-channel run histogram data and redraw the plot.
    draw_test_data(idx)
        Draw a test figure with a random number of subplots.

    """
    
    def __init__(self, mgr, toolbar_factory, fit_factory, *args, **kwargs):
        """Plot class constructor.

        Arguments
        ---------
        mgr : DSPManager
            DSP manager for calls to XIA API.
        toolbar_factory : ToolBarFactory
            Factory for implemented toolbars.
        fit_factory : FitFactory 
            Factory method for fitting plot data.

        """
        super().__init__(*args, **kwargs)

        # Get the logger instance:

        self.logger = logging.getLogger("qtscope_logger")

        # We need the manager to read the XDT value which sets the
        # default trace bin width:

        self.mgr = mgr

        # Data storage and presentation:
        
        self.raw_data = {}
        self.bin_width = 1 # In samples.
        
        ##
        # Main layout
        #
        
        self.figure = plt.figure(tight_layout=True)
        self.canvas = FigureCanvasQTAgg(self.figure)
        self.fit_panel = FitPanel()
        self.toolbar = toolbar_factory.create("plot", self.canvas, self)

        # Initialize the factory with the known methods:
        
        self.fit_factory = fit_factory
        self.fit_factory.initialize(self.fit_panel.function_list)

        # Once initialized, we assume the config information contains some
        # valid text field giving the fit function formula:

        self._update_formula()

        # Disable toolbar on creation (enabled on boot):
        
        self.toolbar.disable()
        
        # Blank canvas with axes to show that this is the plot area:
        
        ax = self.figure.add_subplot(1, 1, 1)
        ax.set_xlim(0, 1)
        ax.set_ylim(0, 1)
 
        layout = QVBoxLayout()
        layout.addWidget(self.toolbar)
        layout.addWidget(self.canvas)
        self.setLayout(layout)

        ##
        # Signal connections
        #
        
        self.toolbar.logscale.clicked.connect(
            lambda a=None: self._set_yscale(ax=a)
        )
        self.toolbar.b_fit_panel.clicked.connect(self._show_fit_panel)

        self.fit_panel.b_fit.clicked.connect(self._fit)
        self.fit_panel.b_clear.clicked.connect(self._clear_fits)
        self.fit_panel.b_cancel.clicked.connect(self._close_fit_panel)
        self.fit_panel.function_list.currentTextChanged.connect(
            self._update_formula
        )
        
    def draw_trace_data(self, data, mod, chan, nrows=1, ncols=1, idx=1):
        """Draws an ADC trace on the plot canvas.

        Parameters
        ----------
        data : list
            List of trace data values.
        mod : int
            Trace data from this module.
        chan : int
            Channel number on the module.
        nrows : int  
            Number of subplot rows (optional, default=1).
        ncols : int  
            Number of subplot columns (optional, default=1).
        idx : int 
            Subplot index in [1, nrows*ncols] (optional).

        """
        self.raw_data[idx-1] = data
        self.bin_width = 1 # Trace data isn't binned.
        ax = self.figure.add_subplot(nrows, ncols, idx)
        ax.plot(self.raw_data[idx-1], drawstyle="steps-post")
        xdt = self.mgr.get_chan_par(mod, chan, "XDT")*1000 # in ns.
        ax.set_xlabel(f"Sample number ({xdt} ns/sample)")
        ax.set_ylabel("Voltage (ADC units)")
        ax.set_xlim(0, xia.MAX_ADC_TRACE_LEN)
        self._set_yscale(ax)

        if (nrows*ncols) > 1:
            ax.set_title("Channel {}".format(idx-1)) # Channels are 0-indexed.
            ax.tick_params(axis='x', labelsize=6)
            ax.tick_params(axis='y', labelsize=6)
            ax.xaxis.label.set_size(6)
            ax.yaxis.label.set_size(6)
        
        self.canvas.draw_idle()
        
    def draw_analyzed_trace(self, trace, fast_filter, cfd, slow_filter):
        """Draw a trace and its filter outputs.

        Parameters
        ----------
        trace : list
            Single-channel ADC trace data.
        fast_filter : list
            Computed fast filter output.
        cfd : list
            Computed CFD output.
        slow_filter : list
            Computed slow filter output.

        """
        self.raw_data[0] = trace
        ax1 = self.figure.add_subplot(3, 1, 1)
        ax1.set_title("Trace")
        ax1.plot(trace, drawstyle="steps-post")
        self._set_yscale(ax1, pad=0.1)
        
        ax2 = self.figure.add_subplot(3, 1, 2)
        ax2.set_title("Timing filters")
        ax2.plot(fast_filter, drawstyle="steps-post")
        ax2.plot(cfd, drawstyle="steps-post")
        self._set_yscale(ax2, pad=0.1)
        ax2.legend(["Trigger filter", "CFD"], loc="upper right")
        
        ax3 = self.figure.add_subplot(3, 1, 3)
        ax3.set_title("Energy filter")
        ax3.plot(slow_filter, drawstyle="steps-post")
        self._set_yscale(ax3, pad=0.1)

        for ax in self.figure.get_axes():
            ax.set_xlim(0, xia.MAX_ADC_TRACE_LEN)
            
        self.canvas.draw_idle()
        
    def draw_run_data(
            self, data, run_type, bin_width, nrows=1, ncols=1, idx=1
    ):
        """Draws a data histogram on the plot canvas.

        Parameters
        ----------
        data : array 
            Single-channel run data.
        run_type : Enum member 
            Type of run data to draw.
        bin_width : int
            Width of each bin in ADC units.
        nrows : int, optional, default=1
            Number of subplot rows.
        ncols : int, optional, default=1
            Number of subplot columns.
        idx : int, optional, default=1
            Subplot index in [1, nrows*ncols].

        """
        self.raw_data[idx-1] = data
        self.bin_width = bin_width
        ax = self.figure.add_subplot(nrows, ncols, idx)
        nbins = int(xia.MAX_HISTOGRAM_LENGTH/self.bin_width)        
        self._plot_histogram(ax, idx-1, nbins)
                
        if run_type == RunType.HISTOGRAM:
            ax.set_xlabel("Energy (ADC units)")
        elif run_type == RunType.BASELINE:
            ax.set_xlabel("Baseline value (ADC units)")
            
        ax.set_ylabel("Counts/bin")
        ax.set_xlim(0, xia.MAX_HISTOGRAM_LENGTH)
        self._set_yscale(ax)
        
        if (nrows*ncols) > 1:
            ax.set_title("Channel {}".format(idx-1))
            ax.tick_params(axis='x', labelsize=6)
            ax.tick_params(axis='y', labelsize=6)
            ax.xaxis.label.set_size(6)
            ax.yaxis.label.set_size(6)

        self.canvas.draw_idle()
                        
    def on_begin_run(self, run_type):
        """Draws a blank histogram canvas when beginning a new run.
        
        Parameters
        ----------
        run_type : Enum member
            Type of run data to draw.

        """
        self.figure.clear()
        try:
            if run_type != RunType.HISTOGRAM and run_type != RunType.BASELINE:
                raise ValueError(f"Encountered unexpected run type {run_type}, select a valid run type and begin a new run")  
        except ValueError as e:
            self.logger.exception("Encountered unknown run type")
            print(e)            
        else:
            ax = self.figure.add_subplot(1, 1, 1)
            if run_type == RunType.HISTOGRAM:
                ax.set_xlabel("ADC energy")
            elif run_type == RunType.BASELINE:
                ax.set_xlabel("Baseline value")
            ax.set_ylabel("Counts/bin")
            self._set_yscale(ax)        
            self.canvas.draw_idle()

    def update_canvas(self):
        """Wait and redraw the whole canvas.

        draw_idle() occasionally does not render the last ~few plots on 
        the main canvas. Introduce a short wait to ensure all draw_idle() 
        operations have completed and redraw the entire canvas one time.
        """
        sleep(0.5)
        self.canvas.draw()

    def rebin(self, bin_width):
        """Rebin histogrammed data.

        Interactive rebinning is only supported for single channel plots, as 
        the plotter keeps no history of all the data it is displaying.

        Parameters
        ----------
        bin_width : int
            Histogramm bin width in ADC values/bin.

        """
        self.bin_width = bin_width
        axs = self.figure.get_axes()
        for idx, ax in zip(range(len(axs)), axs):
            npts = len(self.raw_data[idx]) if self.raw_data else 0
            if npts == xia.MAX_HISTOGRAM_LENGTH:
                nbins = int(xia.MAX_HISTOGRAM_LENGTH/self.bin_width)
                # Remove all drawn histograms but not any axis labeling, etc.
                # and reset the fit panel before plotting the rebinned data:
                for line in ax.get_lines():
                    line.remove()
                self.fit_panel.reset()
                self._plot_histogram(ax, idx, nbins)
            self._set_yscale(ax)
            self.canvas.draw_idle()

    def get_subplot_data(self, chan):
        """Get data from a subplot (channel).

        Arguments
        ---------
        chan : int
            Channel number.

        Returns
        -------
        NumPy array
            Data on the subplot or empty if none.
        """
        # If there are more than one set of data on the plot this probably
        # does not work as intended, but that should be impossible (hopefully)
        axs = self.figure.get_axes()
        if axs[chan].get_lines():
            return np.array(axs[chan].lines[0].get_ydata())
        else:
            return np.empty(0)  
            
    def draw_test_data(self):
        """Draw test data. 

        Draw data points in (-1, 1) on a test canvas with a random number of 
        rows and columns with rows, columns in [1, 4].

        """        
        self.figure.clear()        
        rng = np.random.default_rng()
        nrows = rng.integers(1, 5)
        ncols = rng.integers(1, 5)        
        for i in range(nrows):
            for j in range(ncols):
                idx = ncols*i + j + 1
                ax = self.figure.add_subplot(nrows, ncols, idx)
                data = 2*rng.random(size=5) - 1
                ax.plot(data, "-")                
        self.canvas.draw_idle()

    ##
    # Private methods
    #
    
    def _set_yscale(self, ax, pad=0.05):
        """Configurs the axes display on the canvas.

        If the log scale checkbox is selected while displaying data, 
        immidiately redraw all subplots.

        Parameters
        ----------
        ax : matplotlib Axes
            matplotlib class containing the figure elements.
        pad : float, optional, default=0.05
            Padding as a percent of the full data range added to the y-axis 
            limits when drawing the canvas.

        """        
        if self.sender():
            for ax in self.figure.get_axes():
                self._set_subplot_yscale(ax, pad)
            self.canvas.draw_idle()
        else:
            self._set_subplot_yscale(ax, pad)
                    
    def _set_subplot_yscale(self, ax, pad):
        """Sets the y-axis display for a single figure (linear or log).
    
        The y-axis is autoscaled in log for trace data because all values are 
        greater than zero. For histogram data the axis scaling in log is 
        handled automatically, but in linear the y-axis is set to start at zero.

        Parameters
        ----------
        ax : matplotlib Axes
            matplotlib class containing the figure elements.
        pad : float
            Padding as a percent of the full data range added to the y-axis 
            limits when drawing the canvas.

        """
        ymin, ymax = self._get_ylimits(ax, pad)        
                
        ax.autoscale(axis="y")
        
        if self.toolbar.logscale.isChecked():
            # Symmetric log (should?) handle possible zeroes nicely in the
            # case of empty histograms or inverted traces.
            if ymax > 0:
                ax.set_yscale("log")
            else:
                ax.set_yscale("symlog")
        else:
            ax.set_yscale("linear")
            # Take advantage of a couple of things:
            #   - If there is data, idx 0 will always contain *something*
            #   - All the data is the same type as contained in idx 0
            npts = len(self.raw_data[0]) if self.raw_data else 0
            if npts == xia.MAX_HISTOGRAM_LENGTH:
                ax.set_ylim(0, ymax)
            elif npts == xia.MAX_ADC_TRACE_LEN:
                ax.set_ylim(ymin, ymax)
            else:
                ax.set_ylim(0, 1)

    def _plot_histogram(self, ax, idx, nbins):
        """Create and plot histogrammed data.

        Raw histograms with default binning read are used as data weights 
        to construct a NumPy histogram which is plotted on the provided axes 
        using Axes.plot.

        Parameters
        ----------
        ax : matplotlib Axes
            matplotlib class containing the figure elements.
        idx : int
            Raw data index containing the weights.
        nbins : int
            Number of histogram bins.

        Raises
        ------
        ValueError
            If the data type argument is an invalid value.

        """
        data, bins = np.histogram(
            [i for i in range(xia.MAX_HISTOGRAM_LENGTH)], bins=nbins,
            range=(0, xia.MAX_HISTOGRAM_LENGTH), weights=self.raw_data[idx]
        )
        # Drop the rightmost bin edge for the x-axis data. y-value is the left
        # edge of the bin, as in, e.g. ROOT.
        ax.plot(bins[:-1], data, drawstyle="steps-post", color="tab:blue")

    def _get_ylimits(self, ax, pad):
        """Get the y-axis display limits from the subplot data.

        Parameters
        ----------
        ax : Axes
            Plot axes containing data to define the display range.
        pad : float
            Percentage of the full data range to pad the upper and lower 
            display limits.

        Returns
        -------
        ymin, ymax : float, float
            Lower and upper y-axis display limits. If there is no plotted 
            data, returns (0.0, 1.0).

        """
        lines = ax.get_lines()
        if ax.get_lines():
            ymin = sys.float_info.max
            ymax = sys.float_info.min
            for line in ax.get_lines():
                if min(line.get_ydata()) < ymin:
                    ymin = min(line.get_ydata())
                if max(line.get_ydata()) > ymax:
                    ymax = max(line.get_ydata())
            yrange = ymax - ymin
            ymax = ymax + pad*yrange
            ymin = ymin - pad*yrange            
            return ymin, ymax
        else:
            return 0.0, 1.0  
                
    def _show_fit_panel(self):
        """Show the fit panel GUI in a popup window."""
        self.fit_panel.show()
            
    def _fit(self):
        """Perform the fit based on the current fit panel settings."""        
        if self.raw_data and len(self.figure.get_axes()) == 1:
            ax = plt.gca()
            fit = self.fit_factory.create(
                self.fit_panel.function_list.currentText()
            )
            limits = self._get_fit_limits(ax)
            params = [
                float(self.fit_panel.p0.text()),
                float(self.fit_panel.p1.text()),
                float(self.fit_panel.p2.text()),
                float(self.fit_panel.p3.text()),
                float(self.fit_panel.p4.text()),
                float(self.fit_panel.p5.text())
            ]

            # Get the indices of the x-data array corresponding to the limits.
            # Greatly simplified by the following:
            #     - Data comes with default binning 1 unit/bin,
            #     - Data length always a power of 2,
            #     - Traces cannot be rebinned,
            #     - Histogram binning always a factor of 2.
            # So we can simply use the bin width to reconstruct the index of
            # potentially rebinned data by rounding, otherwise we just get the
            # indices back.
            idx_min = ceil(int(limits[0]/self.bin_width))
            idx_max = floor(int(limits[1]/self.bin_width))
                
            self.logger.debug(f"Fit limits: {limits[0]}, {limits[1]}")
            self.logger.debug(f"Fit limit indices: {idx_min}, {idx_max}")
            self.logger.debug(f"Fit panel guess params: {params}")
            self.logger.debug(f"Data binning factor: {self.bin_width}")

            # If the current subplot has data, get the fit limits and call the
            # fit function's start() rountine to perform the fit.
            if ax.get_lines():
                x = ax.lines[0].get_xdata()[idx_min:idx_max]
                y = ax.lines[0].get_ydata()[idx_min:idx_max]

                result = fit.start(x+self.bin_width/2, y, params, ax)

                # Update the canvas with the results:
                x_fit = np.linspace(limits[0], limits[1], 10000)
                y_fit = fit.model(x_fit, result.x)
                ax.plot(x_fit, y_fit, 'r-')

                # Print the fitted parameters and uncertainties:
                for i in range(len(result.x)):
                    s = "p[{}]: {:.6e} +/- {:.6e}".format(
                        i, result.x[i], np.sqrt(result.hess_inv[i][i])
                    )
                    self.fit_panel.results.append(s)
                    if i == (len(result.x) - 1):
                        self.fit_panel.results.append("\n")
            else:
                QMessageBox.about(
                    self, "Warning",
                    "Unable to get data from the current subplot axis. "     \
                    "Please acquire valid single-channel trace or run data " \
                    "and try again."
                )
            self.canvas.draw_idle()            
        else:
            QMessageBox.about(
                self, "Warning",
                "Cannot perform a fit. Please acquire valid single-channel " \
                "trace or run data and try again."
            )
            
    def _get_fit_limits(self, ax):
        """Get the fit limits on the x-axis based on the selected range. 

        If no range is provided the currently displayed axes limits are assumed
        to be the fit range. Fit limits are rounded to the nearest bin edge 
        within the fitting range using the ceil (left limit) and floor (right 
        limit) functions.

        Parameters
        ----------
        ax : PyPlot axes
            Currently displayed single channel data plot axis.
        
        Returns
        -------
        (int, int)
            Tuple of left and right axis limits.
        """        
        left, right = ax.get_xlim()
        if self.fit_panel.range_min.text():
            left = ceil(float(self.fit_panel.range_min.text()))
        else:
            left = ceil(ax.get_xlim()[0]) 
        if self.fit_panel.range_max.text():
            right = floor(float(self.fit_panel.range_max.text()))
        else:
            right = floor(ax.get_xlim()[1])
            
        return (left, right)

    def _clear_fits(self):
        """Clear previous fits and reset the fit panel GUI."""
        if len(self.figure.get_axes()) == 1:
            ax = plt.gca()
            # First line is the plotted data, remove everything else:
            lines = ax.get_lines()
            for idx, line in zip(range(len(lines)), lines):
                if idx != 0:
                    line.remove()
            self.canvas.draw_idle()
            self.fit_panel.reset()

    def _update_formula(self):
        """Update the fit function formula when a new function is selected."""
        fcn = self.fit_panel.function_list.currentText()
        config = self.fit_factory.configs.get(fcn)
        self.fit_panel.function_formula.setText(config["form"])

    def _close_fit_panel(self):
        """Close the fit panel window."""
        self.fit_panel.close()
