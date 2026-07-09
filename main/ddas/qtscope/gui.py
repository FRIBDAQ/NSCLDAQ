import copy
import inspect
import logging
import os
import math

import numpy as np

from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import QMainWindow, QApplication, QFileDialog

from chan_dsp_gui import ChanDSPGUI
import colors
from dsp_manager import DSPManager
from mod_dsp_gui import ModDSPGUI
from pixie_utilities import SystemUtilities, RunUtilities, TraceUtilities
from plot import Plot
from run_type import RunType
from trace_analyzer import TraceAnalyzer
from thread_pool_manager import ThreadPoolManager

##
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
        self,
        chan_dsp_factory,
        mod_dsp_factory,
        toolbar_factory,
        fit_factory,
        version,
        offline=0,
        *args,
        **kwargs,
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

        self.setWindowTitle(
            "QtScope -- ''Just the goods, bare and plain.'' Powered by Qt5"
        )
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
            self.sys_utils.boot_offline(True)  # No hardware.
            self.trace_utils.use_generator_data(True)  # Use synthetic data.
            self.run_utils.use_generator_data(True)

        # DSP and trace analysis:

        self.trace_analyzer = TraceAnalyzer(self.dsp_mgr)
        self.trace_info = {"trace": np.empty(0), "module": None, "channel": None}

        # Create managers for manipulating DSP settings:

        self.mod_gui = ModDSPGUI(mod_dsp_factory, toolbar_factory, self.pool_mgr)
        self.chan_gui = ChanDSPGUI(chan_dsp_factory, toolbar_factory, self.pool_mgr)

        ##
        # Main layout GUI
        #

        self.sys_toolbar = toolbar_factory.create("sys")
        self.acq_toolbar = toolbar_factory.create("acq")
        self.mplplot = Plot(self.dsp_mgr, toolbar_factory, fit_factory)

        # Set initial state information from the manager and toolbar:

        self.run_active = False
        self.active_type = RunType.INACTIVE
        self.channel_map = []

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
        self.acq_toolbar.current_mod.valueChanged.connect(
            lambda m: self.acq_toolbar.set_channel_spinbox_range(self.channel_map[m])
        )

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
                finished=[self._on_boot],
            )

        self.adjustSize()

    def _on_boot(self):
        """Configure the system following a successful boot."""
        if self.sys_utils.get_boot_status() == True:
            num_modules = self.sys_utils.get_num_modules()

            # Populate list of module MSPS and channel map. In principle
            # could try and grab the whole array at once... but this works
            # well enough for now.

            msps_list = []
            channel_map = []
            histogram_lengths = []
            trace_lengths = []

            for i in range(num_modules):
                msps_list.append(self.sys_utils.get_module_msps(i))
                channel_map.append(self.sys_utils.get_module_channel_count(i))
                histogram_lengths.append(self.run_utils.get_histogram_length(i))
                trace_lengths.append(self.trace_utils.get_trace_length(i))

            # Configure DSP and managers. Performs first time load of DSP
            # settings from the Pixie modules. DSP toolbar spinbox ranges
            # are set in ChanDSPGUI::configure(). Also set the max histogram
            # and trace lengths in the plot widget for proper axis scaling.

            self.channel_map = channel_map

            self.mplplot.set_histogram_length(histogram_lengths)
            self.mplplot.set_trace_length(trace_lengths)

            self.dsp_mgr.initialize_dsp(num_modules, self.channel_map)
            self.chan_gui.configure(
                self.dsp_mgr, num_modules, msps_list, self.channel_map
            )
            self.mod_gui.configure(self.dsp_mgr, num_modules, self.channel_map)

            # Configure toolbars, enable widgets:

            self.sys_toolbar.b_boot.setText("Booted")
            self.sys_toolbar.b_boot.setStyleSheet(colors.GREEN)
            self.sys_toolbar.enable()

            self.acq_toolbar.set_module_spinbox_range(num_modules)
            self.acq_toolbar.set_channel_spinbox_range(self.channel_map[0])
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
                            f"Unsupported extension for settings file: "
                            f"'{fext}.'\n\tSupported extenstions are: .set"
                            f"or .json. Your settings file has not been saved"
                        )
                else:
                    if opt != "XIA settings file (*.set)":
                        raise RuntimeError(f"Unrecognized option '{opt}'")
                    elif fext != ".set":
                        raise RuntimeError(
                            f"Unsupported extension for settings file:"
                            f"'{fext}.'\n\tSupported extension are: .set."
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
        if fname and opt:
            try:
                if (
                    opt == "XIA settings file (*.set)"
                    or "XIA settings file (*.set, *.json)"
                ):
                    self.sys_utils.load_set_file(fname)
                else:
                    raise RuntimeError(f"Unrecognized option '{opt}'")
            except RuntimeError as e:
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
                self,
                "Save file",
                "",
                "XIA settings file (*.set, *.json)",
                options=options,
            )
        else:
            fname, opt = QFileDialog.getSaveFileName(
                self, "Save file", "", "XIA settings file (*.set)", options=options
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
                self,
                "Load file",
                "",
                "XIA settings file (*.set, *.json)",
                options=options,
            )
        else:
            fname, opt = QFileDialog.getOpenFileName(
                self, "Load file", "", "XIA settings file (*.set)", options=options
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
        if self.run_active:
            self._end_run()
        else:
            self._begin_run()

    def _begin_run(self):
        """Start a data run in the currently selected module."""
        module = self.acq_toolbar.current_mod.value()
        run_type = RunType(self.acq_toolbar.run_type.currentIndex())
        nchannels = self.channel_map[module]
        self.logger.debug(
            f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: "
            f"Beginning {run_type} run in Mod. {module} with {nchannels} channels"
        )

        self.pool_mgr.start_thread(
            fcn=lambda: self._start_run(module, nchannels, run_type),
            running=[
                lambda: self.acq_toolbar.b_run_control.setEnabled(False),
                self.chan_gui.toolbar.disable,
                self.mod_gui.toolbar.disable,
            ],
            results=[self._on_run_started],
            errors=[self._show_worker_error],
            finished=[self._on_begin_finished],
        )

    def _start_run(self, module, nchannels, run_type):
        """Worker: Make the API call to start the run.

        Parameters
        ----------
        module : int
            The module number.
        nchannels : int
            The number of channels.
        run_type : RunType
            The type of the run.

        Returns
        -------
        tuple
            A tuple containing the module, run type, and active status.
        """
        self.run_utils.begin_run(module, nchannels, run_type)
        return module, run_type, self.run_utils.get_run_active()

    def _on_run_started(self, result):
        """Slot (GUI thread): all widget and state changes for active runs.

        Parameters
        ----------
        result : tuple
            A tuple containing the module, run type, and active status.
        """
        module, run_type, active = result
        self.run_active = active
        self.active_type = run_type if active else RunType.INACTIVE
        if active:
            self.acq_toolbar.b_run_control.setText("End run")
            self.acq_toolbar.enable_run_active()
            self.mplplot.on_begin_run(module, run_type)
            self.mod_gui.setEnabled(False)
            self.chan_gui.setEnabled(False)

    def _on_begin_finished(self):
        """Slot (GUI thread): recovery floor. Runs after results/errors,
        success or failure, so the GUI can never be left locked."""
        self.acq_toolbar.b_run_control.setEnabled(True)
        if not self.run_active:
            self.acq_toolbar.enable()
            self.chan_gui.toolbar.enable()
            self.mod_gui.toolbar.enable()

    def _end_run(self):
        """Stop a data run in the currently selected module."""
        module = self.acq_toolbar.current_mod.value()
        run_type = self.active_type
        self.logger.debug(
            f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: "
            f"Ending run type {run_type} in Mod. {module}"
        )

        self.pool_mgr.start_thread(
            fcn=lambda: self._stop_run(module, run_type),
            running=[lambda: self.acq_toolbar.b_run_control.setEnabled(False)],
            results=[self._on_run_stopped],
            errors=[self._show_worker_error],
            finished=[lambda: self.acq_toolbar.b_run_control.setEnabled(True)],
        )

    def _stop_run(self, module, run_type):
        """Worker: Make API calls to end the run and read run statistics.
        Reads run stats only after a confirmed stop.

        Parameters
        ----------
        module : int
            The module number.
        run_type : RunType
            The type of the run.

        Returns
        -------
        tuple
            A tuple containing the module and active status.
        """
        self.run_utils.end_run(module, run_type)
        run_active = self.run_utils.get_run_active()
        if not run_active and run_type == RunType.HISTOGRAM:
            self.run_utils.read_stats(module)
        return module, run_active

    def _on_run_stopped(self, result):
        """Slot (GUI thread): all widget and state changes for stopped runs.
        Returns the GUI to the state it was in before the run was started.

        Parameters
        ----------
        result : tuple
            A tuple containing the module and active status.
        """
        module, run_active = result
        self.run_active = run_active
        if not self.run_active:
            self.acq_toolbar.b_run_control.setText("Begin run")
            self.acq_toolbar.enable()
            self.chan_gui.toolbar.enable()
            self.mod_gui.toolbar.enable()
            self.mod_gui.setEnabled(True)
            self.chan_gui.setEnabled(True)
            self.active_type = RunType.INACTIVE
        self.logger.debug(
            f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: "
            f"End run type {self.active_type} in Mod. {module} finalized, "
            f"run active status: {self.run_active}"
        )

    def _read_data(self):
        """Configure a worker to read data from a module.

        If a run is active, read histogram or baseline data for the active
        run type, otherwise read trace(s). All GUI state is read here, on
        the GUI thread, before the worker starts.
        """
        module = self.acq_toolbar.current_mod.value()
        selected_channel = self.acq_toolbar.current_chan.value()
        read_all = self.acq_toolbar.read_all.isChecked()
        channels = (
            list(range(self.channel_map[module])) if read_all else [selected_channel]
        )
        self.logger.debug(f"Reading data from Mod. {module}, Ch. {channels}")

        if self.run_active:  # Histogram or baseline run.
            run_type = self.active_type
            self.pool_mgr.start_thread(
                fcn=lambda: self._acquire_run_data(module, channels, run_type),
                running=[
                    self.acq_toolbar.disable,
                    self.chan_gui.toolbar.disable,
                    self.mod_gui.toolbar.disable,
                    lambda: self.chan_gui.setEnabled(False),
                    lambda: self.mod_gui.setEnabled(False),
                ],
                results=[self._show_run_data],
                errors=[self._show_worker_error],
                finished=[
                    self.acq_toolbar.enable_run_active,
                    self.chan_gui.toolbar.enable,
                    self.mod_gui.toolbar.enable,
                    lambda: self.chan_gui.setEnabled(True),
                    lambda: self.mod_gui.setEnabled(True),
                ],
            )
        else:  # Trace acquisition.
            fast = self.acq_toolbar.fast_acq.isChecked()
            self.pool_mgr.start_thread(
                fcn=lambda: self._acquire_trace_data(
                    module, channels, selected_channel, fast
                ),
                running=[
                    self.acq_toolbar.disable,
                    self.chan_gui.toolbar.disable,
                    self.mod_gui.toolbar.disable,
                    lambda: self.chan_gui.setEnabled(False),
                    lambda: self.mod_gui.setEnabled(False),
                ],
                results=[self._show_trace_data],
                errors=[self._show_worker_error],
                finished=[
                    self.acq_toolbar.enable,
                    self.chan_gui.toolbar.enable,
                    self.mod_gui.toolbar.enable,
                    lambda: self.chan_gui.setEnabled(True),
                    lambda: self.mod_gui.setEnabled(True),
                ],
            )

    def _acquire_run_data(self, module, channels, run_type):
        """Worker: read run data for the given channels. Hardware only.

        Parameters
        ----------
        module : int
            The module number.
        channels : list
            Channel numbers to read.
        run_type : RunType
            The type of the run, snapshotted at click time.

        Returns
        -------
        tuple
            (module, run_type, list of (channel, data) pairs).
        """
        acquired = []
        for ch in channels:
            self.run_utils.read_data(module, ch, run_type)
            acquired.append((ch, self.run_utils.get_data(module, run_type)))
        return module, run_type, acquired

    def _show_run_data(self, result):
        """Slot (GUI thread): draw one channel or the full grid.

        Parameters
        ----------
        result : tuple
            (module, run_type, list of (channel, data) pairs).
        """
        module, run_type, acquired = result
        self.mplplot.figure.clear()
        if len(acquired) == 1:
            _, data = acquired[0]
            self.mplplot.draw_run_data(data, run_type)
        else:
            nrows = math.ceil(len(acquired) / 4)
            for ch, data in acquired:
                self.mplplot.draw_run_data(data, run_type, nrows, 4, ch + 1)

    def _acquire_trace_data(self, module, channels, selected, fast):
        """Worker: acquire traces for the given channels. Hardware only.

        Parameters
        ----------
        module : int
            The module number.
        channels : list
            Channel numbers to read.
        selected : int
            The channel selected on the toolbar at click time, kept for
            single-channel trace info used by the analyzer.
        fast : bool
            Whether to skip signal validation.

        Returns
        -------
        tuple
            (module, selected, list of (channel, data) pairs).
        """
        acquired = []
        for ch in channels:
            if fast:
                self.trace_utils.read_fast_trace(module, ch)
            else:
                self.trace_utils.read_trace(module, ch)
            acquired.append((ch, self.trace_utils.get_trace_data(module)))
        return module, selected, acquired

    def _show_trace_data(self, result):
        """Slot (GUI thread): draw one channel or the full grid and keep
        the selected channel's trace for analysis.

        Parameters
        ----------
        result : tuple
            (module, selected, list of (channel, data) pairs).
        """
        module, selected, acquired = result
        self.mplplot.figure.clear()
        if len(acquired) == 1:
            ch, data = acquired[0]
            self.mplplot.draw_trace_data(data, module, ch)
        else:
            nrows = math.ceil(len(acquired) / 4)
            for ch, data in acquired:
                self.mplplot.draw_trace_data(data, module, ch, nrows, 4, ch + 1)
        for ch, data in acquired:
            if ch == selected:
                self.trace_info.update(
                    {"trace": copy.copy(data), "module": module, "channel": ch}
                )
                break

    def _analyze_trace(self):
        """Analyze the trace for the currently selected channel.

        Always single-channel, regardless of the "Read all" checkbox.
        We have to handle a few different cases, in order:
         - Acquire a new trace if nothing is stored
         - Warn if the selection no longer matches the stored trace
         - Pull from the appropriate channel if "Read all" is checked
        """
        module = self.acq_toolbar.current_mod.value()
        channel = self.acq_toolbar.current_chan.value()
        read_all = self.acq_toolbar.read_all.isChecked()

        # Nothing stored: acquire this channel, then analyze:
        if not self.trace_info["trace"].size:
            self._acquire_then_analyze(module, channel)
            return

        # Module changed since acquisition, stale trace:
        if module != self.trace_info["module"]:
            self._warn_stale_trace(module, channel)
            return

        # Channel changed: read-all has the data for it is on
        # the canvas; a single-channel read for another channel
        # is stale:
        if channel != self.trace_info["channel"]:
            if not read_all:
                self._warn_stale_trace(module, channel)
                return
            self.trace_info.update(
                {
                    "trace": copy.copy(self.mplplot.get_subplot_data(channel)),
                    "module": module,
                    "channel": channel,
                }
            )

        # Stored trace matches the selection, analyze and draw:
        self._analyze_and_plot()

    def _acquire_then_analyze(self, module, channel):
        """Acquire a single-channel trace, then analyze it in the slot.

        Parameters
        ----------
        module : int
            The module number.
        channel : int
            The channel number.
        """
        fast = self.acq_toolbar.fast_acq.isChecked()
        self.acq_toolbar.disable()
        self.pool_mgr.start_thread(
            fcn=lambda: self._acquire_trace_data(module, [channel], channel, fast),
            running=[self.chan_gui.toolbar.disable, self.mod_gui.toolbar.disable],
            results=[self._store_trace_and_analyze],
            errors=[self._show_worker_error],
            finished=[
                self.acq_toolbar.enable,
                self.chan_gui.toolbar.enable,
                self.mod_gui.toolbar.enable,
            ],
        )

    def _store_trace_and_analyze(self, result):
        """Slot (GUI thread): store the acquired trace, then analyze.

        Parameters
        ----------
        result : tuple
            (module, selected, list of (channel, data) pairs).
        """
        module, selected, acquired = result
        ch, data = acquired[0]
        self.trace_info.update(
            {"trace": copy.copy(data), "module": module, "channel": ch}
        )
        self._analyze_and_plot()

    def _warn_stale_trace(self, module, channel):
        """Tell the user the stored trace does not match the selection."""
        msg = (
            f"Stored trace data for Mod. {self.trace_info['module']} "
            f"Ch. {self.trace_info['channel']} does not match the current "
            f"selection Mod. {module} Ch. {channel}"
        )
        self.logger.warning(msg)  # warning, not exception: no traceback needed
        print(
            f"{msg}:\n\tNew trace data must be acquired by clicking the "
            "'Read trace' button prior to analysis."
        )

    def _analyze_and_plot(self):
        """Compute filters for the stored trace and draw them.

        trace_info is reset after any analysis attempt.
        """
        try:
            self.trace_analyzer.analyze(
                self.trace_info["module"],
                self.trace_info["channel"],
                self.trace_info["trace"],
            )
        except Exception as e:
            self.logger.exception(
                f"Error analyzing acquired trace from Mod. {self.trace_info['module']} "
                f"Ch. {self.trace_info['channel']}"
            )
            print(e)
        else:
            self.mplplot.figure.clear()
            self.mplplot.draw_analyzed_trace(
                self.trace_info["trace"],
                self.trace_analyzer.fast_filter,
                self.trace_analyzer.cfd,
                self.trace_analyzer.slow_filter,
            )
        finally:
            self.trace_info.update(
                {"trace": np.empty(0), "module": None, "channel": None}
            )

    def _show_worker_error(self, err):
        """General-purpose user-facing error presenter (GUI tier).

        MainWindow owns this; the manager's default handler only logs.
        """
        exctype, value, tb = err
        self.logger.error(f"Worker failed: {value}\n{tb}")
        print(f"Operation failed: {value}")
