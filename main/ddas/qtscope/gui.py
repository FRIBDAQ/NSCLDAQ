import copy
import inspect
import json
import logging
import os
import sys
from time import sleep

import numpy as np
import pandas as pd

from PyQt5.QtCore import Qt
from PyQt5.QtGui import QCloseEvent
from PyQt5.QtWidgets import (
    QMainWindow, QVBoxLayout, QWidget, QApplication, QFileDialog
)

from chan_dsp_gui import ChanDSPGUI 
import colors
from dsp_manager import DSPManager
from mod_dsp_gui import ModDSPGUI
from pixie_utilities import SystemUtilities, RunUtilities, TraceUtilities
from plot import Plot
from run_type import RunType
from trace_analyzer import TraceAnalyzer
from thread_pool_manager import ThreadPoolManager

# @todo Would like to run the system as per custom DSP parameter formatted
# text file output -- as currently implemented the save DSP settings do not
# contain a full set of DSP parameters and are written using some standard
# python module functions. The XIA API call to boot modules requires a DSP
# settings file of a particular size and format which is inconsistent with
# our custom file format and differs by API version.

# @todo Toolbar disable shouldn't disable cancel button to close the windows.

class MainWindow(QMainWindow):
    """The main GUI window.

    Instances XIA API managers and internal DSP data storage as well as 
    toolbars for interacting with the managers and DSP. Tracks run status 
    to handle toolbar button functions. Contains plotting widget for displaying
    channel traces and histograms.

    Attributes
    ----------
    logger : Logger
        QtScope Logger object.
    xia_api_version : int 
        XIA API version QtScope was compiled against.
    pool_mgr : ThreadPoolManager
        Global thread pool manager. Manages worker threads from global pool.
    dsp_mgr : DSPManager
        Manager for internal DSP and interface for XIA API read/write 
        operations.
    sys_utils : SystemUtilities
        Interface to XIA API for system-level tasks.
    trace_utils : TraceUtilities 
        Interface to XIA API for trace acquisition.
    run_utils : RunUtilities 
        Interface to XIA API for MCA-mode run control.
    chan_gui : ChanDSPGUI 
       Channel DSP GUI.
    mod_gui : ModDSPGUI
        Module DSP GUI.
    sys_toolbar : SystemToolBar
        Toolbar for the SystemUtilities.
    acq_toolbar : AcquisitionToolBar
        Toolbar for the RunUtilities.
    mplplot : Plot
        matplotlib plotting widget.
    run_active : bool
         True when an energy histogram or baseline run is active, False 
         otherwise.
    active_type : Enum member
         The run type set at run start, INACTIVE if when no run is active.
    trace_info : dict
        Single channel ADC trace information from last single channel 
        acquisition.

    Methods
    -------
    closeEvent(event)
        Overridden QWidget closeEvent to close all popups.
    """
    
    def __init__(
            self,chan_dsp_factory, mod_dsp_factory, toolbar_factory,
            fit_factory, version, offline=0, *args, **kwargs
    ):
        """GUI MainWindow constructor.
        
        Arguments
        ---------
        chan_dsp_factroy : WidgetFactory 
            Factory for implemented channel DSP widgets.
        mod_dsp_factroy :WidgetFactory 
            Factory for implemented module DSP widgets.
        toolbar_factory : WidgetFactory 
            Factory for implemented toolbar widgets.
        fit_factory :FitFactory
            Factory for implemented fitting methods.
        version : int 
            XIA API major version number.
        offline : int, optional, default=0
            If any non-zero number, run in offline mode with no hardware and 
            simulated data.
        """        
        super().__init__(*args, **kwargs)
            
        self.setWindowTitle("QtScope -- ''Just the goods, bare and plain.''")
        self.setWindowFlag(Qt.WindowMinimizeButtonHint, True)
        self.setWindowFlag(Qt.WindowMaximizeButtonHint, True)
        self.setMouseTracking(True)

        # Get the Logger instance:
        
        self.logger = logging.getLogger("qtscope_logger")

        # Set XIA API version. This is needed to support XIA API 3 JSON
        # settings files with a .json extension iff QtScope was compiled
        # against API 3. @todo (ASC 6/9/23): may not be needed after we
        # fully migrate to API 3, but for now its an option to allow
        # user's some ability to distinguish between the two formats.
        
        self.xia_api_version = version
        
        # Access to global thread pool for this applicaition:
        
        self.pool_mgr = ThreadPoolManager()
            
        # XIA API managers:
        
        self.dsp_mgr = DSPManager()
        self.sys_utils = SystemUtilities()
        self.trace_utils = TraceUtilities()
        self.run_utils = RunUtilities()
        
        # Configure managers:
        
        if offline:
            self.sys_utils.boot_offline(True)         # No hardware.
            self.trace_utils.use_generator_data(True) # Use synthetic data.
            self.run_utils.use_generator_data(True)
            
        # DSP and trace analysis:
        
        self.trace_analyzer = TraceAnalyzer(self.dsp_mgr)
        self.trace_info = {
            "trace": np.empty(0),
            "module": None,
            "channel": None
        }

        # Create managers for manipulating DSP settings:
        
        self.mod_gui = ModDSPGUI(
            mod_dsp_factory, toolbar_factory, self.pool_mgr
        )
        self.chan_gui = ChanDSPGUI(
            chan_dsp_factory, toolbar_factory, self.pool_mgr
        )

        ##
        # Main layout GUI
        #

        self.sys_toolbar = toolbar_factory.create("sys")
        self.acq_toolbar = toolbar_factory.create("acq")
        self.mplplot = Plot(self.dsp_mgr, toolbar_factory, fit_factory)

        # Set initial run state information from the manager and toolbar:
        
        self.run_active = False
        self.active_type = RunType.INACTIVE

        # Define the main layout and add widgets:
        
        self.addToolBar(self.sys_toolbar)
        self.addToolBarBreak()
        self.addToolBar(self.acq_toolbar)     
        self.setCentralWidget(self.mplplot)
        
        self.adjustSize()
        
        ##
        # Signal connections
        #
        
        # System toolbar:
        
        self.sys_toolbar.b_boot.clicked.connect(self._boot)
        self.sys_toolbar.b_chan_gui.clicked.connect(self._show_chan_gui)
        self.sys_toolbar.b_mod_gui.clicked.connect(self._show_mod_gui)
        self.sys_toolbar.b_save.clicked.connect(self._save_settings)
        self.sys_toolbar.b_load.clicked.connect(self._load_settings)
        self.sys_toolbar.b_exit.clicked.connect(self._system_exit)
        
        # Acquisition toolbar:
        
        self.acq_toolbar.b_read_trace.clicked.connect(self._read_data)
        self.acq_toolbar.b_analyze_trace.clicked.connect(self._analyze_trace)
        self.acq_toolbar.b_read_data.clicked.connect(self._read_data)
        self.acq_toolbar.b_run_control.clicked.connect(self._run_control)
        
    ##
    # Public methods
    #

    def closeEvent(self, event):
        """Overridden QWidget closeEvent function.

        Called when the main window is exited via the [X] button rather than 
        exit. Calls the same system exit function as the button to close the 
        connection to the modules and exit gracefully.

        Parameters
        ----------
        event : QCloseEvent 
            The handled signal. Always accepted.
        """        
        event.accept()
        self._system_exit()
    
    ##
    # Private methods
    #
    
    def _boot(self):
        """Boot the system using the SystemUtilities. 

        SystemUtilities is a C++ interface tp call the relavent XIA API 
        functions. If the boot is successful, configure the DSP and DSP 
        GUIs. Only attempt to boot if the system has not been booted already.
        """        
        # Access thread from global thread pool to boot:
        
        if self.sys_utils.get_boot_status() == False:
            self.pool_mgr.start_thread(
                fcn=self.sys_utils.boot,
                running=[self.sys_toolbar.disable, self.acq_toolbar.disable],
                finished=[self._on_boot]
            )
            
        self.adjustSize()

    # @todo (ASC 10/31/23): Module MSPS information should be easily accessible
    # to other parts of the program, most notably the trace analyzer to set the
    # CFD values.
    def _on_boot(self):
        """Configure the system following a successful boot."""
        if self.sys_utils.get_boot_status() == True:            
            # Populate list of module MSPS and channel map.
            # Length of list == number of installed modules in the crate:

            num_modules = self.sys_utils.get_num_modules()
            msps_list = []
            channel_map = []
            for i in range(num_modules):
                msps_list.append(self.sys_utils.get_module_msps(i))
                channel_map.append(self.sys_utils.get_channel_count(i))
                
            # Configure DSP and managers. Performs first time load of DSP
            # settings from the Pixie modules.
            
            self.dsp_mgr.initialize_dsp(num_modules, channel_map)
            self.chan_gui.configure(self.dsp_mgr, msps_list, channel_map)
            self.mod_gui.configure(self.dsp_mgr, num_modules)
            
            # Repaint boot button, configure spinboxes, enable widgets:
            
            self.sys_toolbar.b_boot.setText("Booted")
            self.sys_toolbar.b_boot.setStyleSheet(colors.GREEN)
            self.acq_toolbar.current_mod.setRange(0, num_modules-1)
            nchan = self.sys_utils.get_channel_count(0) # First module
            self.acq_toolbar.current_chan.setRange(0, nchan-1)
            
            self.sys_toolbar.enable()
            self.acq_toolbar.enable()            
            self.mplplot.toolbar.enable()

            print("QtScope system configuration complete!")
            self.logger.info("System configuration successful")

    # @todo (ASC 3/21/23): Define another custom human-readable text format
    # independent of the XIA API version.
    # @todo (ASC 6/9/23): Add some GUI blocking to save/load to prevent
    # settings file corruption.    
    def _save_settings(self):
        """Save DSP parameters to an XIA settings file. 

        XIA API 2 binary settings files are required to have the extension 
        .set while XIA API 3 JSON settings files are required to have either 
        one of .set or .json. Extensions are not case-sensitive.

        Raises
        ------
        RuntimeError  
            If the file extension options differ from what is expected from 
            the API version.
        RuntimeError
            If the file extension is invalid for the API version.
        """        
        fname, opt = self._save_dialog()
        fext = os.path.splitext(fname)[-1].lower()
        if fname and opt:
            try:
                if self.xia_api_version >= 3:
                    if opt != "XIA settings file (*.set, *.json)":
                        raise RuntimeError(f"Unrecognized option '{opt}'")
                    elif fext != ".set" and fext != ".json":
                        raise RuntimeError(
                            f"Unsupported extension for settings file: " \
                            f"'{fext}.'\n\tSupported extenstions are: .set" \
                            f"or .json. Your settings file has not been saved"
                        )
                else:
                    if opt != "XIA settings file (*.set)":
                        raise RuntimeError(f"Unrecognized option '{opt}'")
                    elif fext != ".set":
                        raise RuntimeError(
                            f"Unsupported extension for settings file:" \
                            f"'{fext}.'\n\tSupported extension are: .set." \
                            f"Your settings file has not been saved"
                        )                
            except RuntimeError as e:
                self.logger.exception("Error saving settings file")
                print(e)
            else:
                self.sys_utils.save_set_file(fname)
                print(f"DSP parameter file saved to: {fname}")
            
    def _load_settings(self):
        """Load DSP parameters from an XIA settings file.

        Raises
        ------
        RuntimeError
            If file format is unrecognized.
        """        
        fname, opt = self._load_dialog()
        fext = os.path.splitext(fname)[-1].lower()
        if fname and opt:
            try:
                if (opt == "XIA settings file (*.set)"
                    or "XIA settings file (*.set, *.json)"):
                    self.sys_utils.load_set_file(fname)
                else:
                    raise RuntimeError(f"Unrecognized option '{opt}'")
            except RuntimeError:
                self.logger.exception("Error loading settings file")
                print(e)

        # If the system has been booted, reload the DSP into the dataframe,
        # and reload the current channel DSP tab and module DSP (other channel
        # DSP loaded when a new tab is selected). Otherwise wait for system
        # boot (message issued by SystemMananager.cpp in this case).
        
        if self.sys_utils.get_boot_status() == True:
            self.dsp_mgr.load_new_dsp()
            self.chan_gui.load_dsp()
            self.mod_gui.load_dsp()
            
    def _save_dialog(self):
        """Get a file name and extension from QFileDialog.

        Returns
        -------
        fname : str 
            The file name from QFileDialog.getSaveFileName.
        opt : str 
            The file extension option from QFileDialog.getSaveFileName.
        """        
        options = QFileDialog.Options()
        options |= QFileDialog.DontUseNativeDialog
        if self.xia_api_version >= 3:
            fname, opt = QFileDialog.getSaveFileName(
                self, "Save file", "",
                "XIA settings file (*.set, *.json)",
                options=options
            )
        else:
            fname, opt = QFileDialog.getSaveFileName(
                self, "Save file", "",
                "XIA settings file (*.set)",
                options=options
            )
        if (fname, opt):
            return fname, opt
        else:
            return None, None

    def _load_dialog(self):
        """Get a file name and extension from QFileDialog.

        Returns
        -------
        fname : str
            The file name from QFileDialog.getSaveFileName.
        opt : str 
            The file extension option from QFileDialog.getSaveFileName.

        Note
        ----
        Will return None, None if the filename and extension do not exist
        which can occur if the user closes the QFileDialog window before
        selecting a file to load.
        """        
        options = QFileDialog.Options()
        options |= QFileDialog.DontUseNativeDialog
        if self.xia_api_version >= 3:
            fname, opt = QFileDialog.getOpenFileName(
                self,"Load file", "",
                "XIA settings file (*.set, *.json)",
                options=options
            )            
        else:
            fname, opt = QFileDialog.getOpenFileName(
                self,"Load file", "", "XIA settings file (*.set)",
                options=options
            )
            
        if (fname, opt):
            return fname, opt
        else:
            return None, None
         
    def _system_exit(self):
        """Closes connection to Pixie modules and exits the application."""
        self.sys_utils.exit_system()
        self.pool_mgr.exit()
        app = QApplication.instance()
        app.quit()

    ##
    # DSP management
    #
    
    def _show_chan_gui(self):
        """Show the channel DSP manager window."""        
        self.chan_gui.show()

    def _show_mod_gui(self):
        """Show the module DSP manager window."""        
        self.mod_gui.show()

    def _print_dsp(self):
        """Dump contents of DSP internal storage structure to the terminal."""
        self.dsp_mgr.print()

    ##
    # Acquisition management
    #
    
    def _run_control(self):
        """Start or stop a run depending on current run status."""
        
        # If there is a thread running, wait for it to exit:        

        nthreads = self.pool_mgr.get_active_thread_count()
        if nthreads > 0:
            print(
                f"{nthreads} threads are currently communicating with "
                f"the module(s). Waiting..."
            )
            self.pool_mgr.wait()
            
        # Access thread from global thread pool for the begin/end operation
        # If a run is active, end it, otherwise start a new one:
        
        if self.run_active:
            self.logger.debug(f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: Run active {self.run_active}, type {self.active_type}; Ending current run")           
            self.pool_mgr.start_thread(
                fcn=self._end_run,
                finished=[self.chan_gui.toolbar.enable,
                          self.mod_gui.toolbar.enable,
                          self.acq_toolbar.enable]
            )
        else:
            self.logger.debug(f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: Run active {self.run_active}, type {self.active_type}; Beginning new run")            
            self.pool_mgr.start_thread(
                fcn=self._begin_run,
                running=[self.chan_gui.toolbar.disable,
                         self.mod_gui.toolbar.disable,
                         self.acq_toolbar.enable_run_active]
            )
            
    def _begin_run(self):
        """Start a data run in the currently selected module. 

        Check and update the run status, set the current run type, and update 
        the acquisition toolbar button states.
        """        
        module = self.acq_toolbar.current_mod.value()
        self.active_type = RunType(self.acq_toolbar.run_type.currentIndex())    
        self.logger.debug(f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: Beginning {self.active_type} run in Mod. {module}")
        
        # XIA API call to begin run in the current module:

        nchannels = self.sys_utils.get_channel_count(module)
        self.run_utils.begin_run(module, nchannels, self.active_type)
        self.run_active = self.run_utils.get_run_active()
        self.logger.debug(f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: Started, run active {self.run_active}")        
            
        # Reconfigure the run control button and communicate run status:
        
        if self.run_active:
            self.acq_toolbar.b_run_control.setText("End run")
            self.mplplot.on_begin_run(self.active_type)
            self.mod_gui.setEnabled(False)
            self.chan_gui.setEnabled(False)
            
    def _end_run(self):
        """Stop an MCA run in the currently selected module. 

        Check and update the run status and update the acquisition toolbar 
        button states.
        """        
        module = self.acq_toolbar.current_mod.value()
        self.logger.debug(f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: Ending {self.active_type} run in Mod. {module}")

        # XIA API call to end run in the current module:
        
        self.run_utils.end_run(module, self.active_type)
        self.run_active = self.run_utils.get_run_active()
            
        # Print run stats for histogram run and reconfigure GUI:
        
        if not self.run_active:
            if self.active_type == RunType.HISTOGRAM:
                self.run_utils.read_stats(module)
            self.acq_toolbar.b_run_control.setText("Begin run")
            self.mod_gui.setEnabled(True)
            self.chan_gui.setEnabled(True)
            self.active_type = RunType.INACTIVE

        self.logger.debug(f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: End run finalized, run active {self.run_active}, type {self.active_type}")
        
    def _read_data(self):
        """Configure worker to read data from a module.

        If a run is active, read histogram or baseline data based on the 
        active run type, otherwise read a trace.
        """
        
        # If there is a thread running, wait for it to exit:
        
        nthreads = self.pool_mgr.get_active_thread_count()
        if nthreads > 0:
            print(nthreads, "threads are currently communicating with the module(s). Waiting...")
            self.pool_mgr.wait()

        # Access thread from global thread pool for the data read operation.
        # If a run is active, read either an energy histogram or baseline
        # depending on the run type, otherwise read a trace.
        
        if self.run_active: # Histogram run.
            self.pool_mgr.start_thread(
                fcn=self._read_run_data,
                running=[self.acq_toolbar.disable],
                finished=[self.acq_toolbar.enable_run_active]
            )           
        else: # Trace acquisition.
            self.pool_mgr.start_thread(
                fcn=self._read_trace_data,
                running=[self.chan_gui.toolbar.disable,
                         self.mod_gui.toolbar.disable,
                         lambda: self.chan_gui.setEnabled(False),
                         lambda: self.mod_gui.setEnabled(False)],
                finished=[self.chan_gui.toolbar.enable,
                          self.mod_gui.toolbar.enable,
                          lambda: self.chan_gui.setEnabled(True),
                          lambda: self.mod_gui.setEnabled(True)]
            )  

    def _read_run_data(self):
        """Read run (energy histogram or baseline) data. 

        Read data from the currently selected module and channel(s) and 
        update the main display.
        """        
        self.mplplot.figure.clear()
        module = self.acq_toolbar.current_mod.value()
        nchans = self.sys_utils.get_channel_count(module)
        channel = self.acq_toolbar.current_chan.value()

        # Read from module and get data, then draw:
        
        if self.acq_toolbar.read_all.isChecked():
            for i in range(nchans):
                self.run_utils.read_data(module, i, self.active_type)
                data = self.run_utils.get_data(self.active_type)
                # Expect either 16 or 32 channels, so 4x4 or 8x4:
                self.mplplot.draw_run_data(
                    data, self.active_type, nchans/4, 4, i+1
                )
            self.mplplot.update_canvas()
        else:           
            self.run_utils.read_data(module, channel, self.active_type)
            data = self.run_utils.get_data(self.active_type)
            self.mplplot.draw_run_data(data, self.active_type)
            
    def _read_trace_data(self):
        """Read trace data.

        Read trace(s) from the currently selected module and channel(s) and 
        update the main display. 
        """        
        self.mplplot.figure.clear()
        module = self.acq_toolbar.current_mod.value()
        nchans = self.sys_utils.get_channel_count(module)
        channel = self.acq_toolbar.current_chan.value()

        # Retrieve trace from this module and channel and get its data. If
        # signal validation is required (fast acquisition mode is not
        # selected), the read function will validate and reacquire trace
        # signals until it either finds a good trace or hits a retry limit.
        
        if self.acq_toolbar.read_all.isChecked(): # Read all.
            for i in range(nchans):             
                if self.acq_toolbar.fast_acq.isChecked():
                    self.trace_utils.read_fast_trace(module, i)
                else:
                    self.trace_utils.read_trace(module, i)

                data = self.trace_utils.get_trace_data()
                # Expect either 16 or 32 channels, so 4x4 or 8x4:
                self.mplplot.draw_trace_data(data, module, i, nchans/4, 4, i+1)
                    
                # Keep the single channel trace information:
                
                if i == channel:
                    self.trace_info.update({
                        "trace": copy.copy(data),
                        "module": module,
                        "channel": channel
                    })
                    
            self.mplplot.update_canvas()   
        else: # Read single channel.
            if self.acq_toolbar.fast_acq.isChecked():
                self.trace_utils.read_fast_trace(module, channel)
            else:
                self.trace_utils.read_trace(module, channel)

            data = self.trace_utils.get_trace_data()
            self.mplplot.draw_trace_data(data, module, channel)
            
            # Keep the single channel trace information:
            
            self.trace_info.update({
                "trace": copy.copy(data),
                "module": module,
                "channel": channel
            })
    
    def _analyze_trace(self):
        """Setup worker to analyze a single-channel ADC trace."""
        self.pool_mgr.start_thread(
            fcn=self._analyze_and_show_trace,
            running=[self.chan_gui.toolbar.disable,
                     self.mod_gui.toolbar.disable,
                     self.acq_toolbar.disable,
                     lambda: self.chan_gui.setEnabled(False),
                     lambda: self.mod_gui.setEnabled(False)],
            finished=[self.chan_gui.toolbar.enable,
                      self.mod_gui.toolbar.enable,
                      self.acq_toolbar.enable,
                      lambda: self.chan_gui.setEnabled(True),
                      lambda: self.mod_gui.setEnabled(True)]
        )
        
        
    # @todo Should analyze trace be an available feature if read all?    
    # @todo try-except block is pretty long for a single function.    
    def _analyze_and_show_trace(self):
        """Display single channel trace filter output.
        
        Raises
        ------
        ValueError
            If the module number is changed between acquisition and analyze 
            attempt.
            If the channel number for a single-channel read is changed 
            between acquisition and analysis.
        """        
        module = self.acq_toolbar.current_mod.value()
        channel = self.acq_toolbar.current_chan.value()
        
        # If there is no current trace data, acquire a trace. We have a whole
        # bunch of possibilities depending on whether or not the user has
        # toggled the module and channel selection boxes and what data was
        # read on the last trace acquisition (single channel or all channels).
        # Below we handle the various cases:
        
        try: 
            if not self.trace_info["trace"].size:
                # If there is no trace data, acquire a new trace: 
                if self.acq_toolbar.fast_acq.isChecked():
                    self.trace_utils.read_fast_trace(module, channel)
                else:
                    self.trace_utils.read_trace(module, channel)
                    
                self.trace_info.update({
                    "trace": copy.copy(self.trace_utils.get_trace_data()),
                    "module": module,
                    "channel": channel
                })
            elif module != self.trace_info["module"]:                
                # Module number changed between acquisition and
                # analysis, user needs to acquire new trace for
                # the currently selected channel:                
                raise ValueError(
                    f"Stored trace data for Mod. {self.trace_info['module']} "
                    f"Ch. {self.trace_info['channel']} does not match the "
                    f"current selection box Mod. {module} Ch. {channel}"
                )
            elif (
                    self.acq_toolbar.read_all.isChecked()
                    and channel != self.trace_info["channel"]
            ):
                # Channel number changed between acquisition and
                # analysis. We've read all channel trace data so
                # we just need to grab the appropriate data:
                self.trace_info.update({
                    "trace": copy.copy(self.mplplot.get_subplot_data(channel)),
                    "module": module,
                    "channel": channel
                })
            elif (
                    not self.acq_toolbar.read_all.isChecked()
                    and channel != self.trace_info["channel"]
            ):
                # Channel number changed between acquisition and
                # analysis. We have _not_ read all channel trace
                # data so user needs to re-acquire:
                raise ValueError(
                    f"Stored trace data for Mod. {self.trace_info['module']} "
                    f"Ch. {self.trace_info['channel']} does not match the "
                    f"current selection box Mod. {module} Ch. {channel}"
                )
        except ValueError as e:
            self.logger.exception(
                "Channel selection changed between acquisition and analysis"
            )
            print(
                f"{e}:\n\tNew trace data must be acquired by clicking "
                f"the 'Read trace' button prior to analysis."
            )
        else:            
            # No exceptions, analyze and draw:
            try:
                self.trace_analyzer.analyze(
                    self.trace_info["module"],
                    self.trace_info["channel"],
                    self.trace_info["trace"]
                )
            except Exception as e:
                self.logger.exception("Error analyzing acquired trace")
                print(e)
            else:
                self.mplplot.figure.clear()
                self.mplplot.draw_analyzed_trace(
                    self.trace_info["trace"],
                    self.trace_analyzer.fast_filter,
                    self.trace_analyzer.cfd,
                    self.trace_analyzer.slow_filter
                )
            finally:                
                # Reset the single channel trace information:            
                self.trace_info.update({
                    "trace": np.empty(0),
                    "module": None,
                    "channel": None 
                })            
            
    def _test(self):
        """A dummy function which can be hooked up to signals for testing."""
        print(f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: Hey I just wrote you, and this is crazy\nBut here's my purpose, you should call me, maybe")
