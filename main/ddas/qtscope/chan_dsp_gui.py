import inspect
import logging
import os

from PyQt5.QtCore import Qt
from PyQt5.QtGui import QPixmap
from PyQt5.QtWidgets import QMainWindow, QLabel

from chan_dsp_layout import ChanDSPLayout
from extensions import MyTabWidget


class ChanDSPGUI(QMainWindow):
    """Channel DSP GUI class.

    DSP GUI for configuring channel parameters. Settings are displayed in a
    nested tabbed widget where each module tab has a series of tabs assigned
    to it corresponding to the channel DSP settings for that module.

    Attributes
    ----------
    pool_mgr : ThreadPoolManager
        Global thread pool manager.
    chan_params : MyTabWidget
        Tabbed widget of module DSP settings.
    chan_dsp_factory : WidgetFactory
        Factory for implemented channel DSP widgets.
    toolbar : DSPToolBar
        Toolbar for manipulating DSP settings.
    dsp_mgr : DSPManager
        Manager for internal DSP and interface for XIA API read/write
        operations.
    channel_map : list
        Number of channels in each module.
    mod_idx : int
        Currently selected module index.
    par_idx : int
        Currently selected DSP tab index.
    tab : QWidget
        Currently selected DSP tab widget.
    tab_name : str
        Currently selected DSP tab widget name.
    timing_diagram : QLabel
        Diagram of coincidence timing using the pixmap feature of QLabel.

    Methods
    -------
    configure(dsp_manager, num_modules, msps_list, channel_map)
        Initialize tabbed layout.
    apply_dsp()
        Apply DSP settings for a given module and DSP settings.
    load_dsp()
        Load DSP settings for a given module and DSP settings.
    copy_mod_dsp()
        Copy DSP settings from another module.
    copy_chan_dsp()
        Copy DSP settings from another channel on this module.
    adjust_offsets()
        Adjust DC offsets for a single module.
    print_masks()
        Print channel multiplicity masks and coincidence settings.
    show_diagram()
        Show coincidence timing help diagram describing the settings.
    cancel()
        Close the manager window.
    closeEvent(event)
        Overridden QWidget closeEvent.
    """

    def __init__(self, chan_dsp_factory, toolbar_factory, pool_mgr, *args, **kwargs):
        """ChanDSPGUI class constructor.

        Parameters
        ----------
        chan_dsp_factory : WidgetFactory
            Factory for implemented channel DSP widgets.
        toolbar_factory : WidgetFactory
            Factory for implemented toolbar widgets.
        pool_mgr : ThreadPoolManager
            Management for global thread pool.
        """
        super().__init__(*args, **kwargs)

        self.setWindowTitle("Channel DSP manager")

        # Access to global thread pool for this application:

        self.pool_mgr = pool_mgr

        ##
        # Main layout
        #

        self.chan_params = MyTabWidget()
        self.chan_params.setMinimumSize(840, 620)
        self.chan_dsp_factory = chan_dsp_factory

        self.toolbar = toolbar_factory.create("dsp")

        self.addToolBar(Qt.BottomToolBarArea, self.toolbar)
        self.setCentralWidget(self.chan_params)

        # Timing diagram. QLabel has a Pixmap, so we'll use that to display
        # the diagram and add some padding to the edges of the image because
        # its very tightly cropped:

        fig_path = os.environ["DAQROOT"] + "/ddas/qtscope/figures/timing_diagram.png"
        fig = QPixmap(fig_path)
        self.timing_diagram = QLabel()
        self.timing_diagram.setWindowTitle("Timing diagram")
        self.timing_diagram.setPixmap(fig)
        self.timing_diagram.setStyleSheet("padding :15px")

        ##
        # Signal connections
        #

        self.toolbar.b_apply.clicked.connect(self.apply_dsp)
        self.toolbar.b_load.clicked.connect(self.load_dsp)
        self.toolbar.b_copy_mod.clicked.connect(self.copy_mod_dsp)
        self.toolbar.b_copy_chan.clicked.connect(self.copy_chan_dsp)
        self.toolbar.b_cancel.clicked.connect(self.cancel)
        self.toolbar.copy_mod.valueChanged.connect(
            lambda m: self.toolbar.set_channel_spinbox_range(self.channel_map[m])
        )

        self.chan_params.currentChanged.connect(self._display_new_tab)

    def configure(self, dsp_manager, num_modules, msps_list, channel_map):
        """Configure channel DSP manager.

        Setup the toolbar, get the DSP, and create the tabbed widget. This
        configure step must come after the system has been booted and the
        DSP storage has been initialized.

        Parameters
        ----------
        dsp_manager : DSPManager
            Manager for internal DSP and interface for XIA API read/write
            operations.
        num_modules : int
            Number of modules in the system
        msps_list : list
            List of module ADC MSPS values.
        channel_map : list
            List of channels per module.
        """
        self.dsp_mgr = dsp_manager
        self.channel_map = channel_map

        logging.getLogger("qtscope_logger").debug(
            f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: Configuring GUI for {num_modules} modules using {self.dsp_mgr}"
        )

        # Initialize tab indices and widget:

        self.mod_idx = 0
        self.par_idx = 0
        self.tab = None
        self.tab_name = ""

        # Set range on module spin box in DSP GUI:

        self.toolbar.set_module_spinbox_range(num_modules)

        for i in range(num_modules):

            # DSP tab layout for each module in the system:

            self.chan_params.insertTab(
                i,
                ChanDSPLayout(self.chan_dsp_factory, self.channel_map[i]),
                "Mod. %i" % i,
            )

            # DSP tabs load from dataframe when switching. Just added the
            # module tabbed widget so add the signal here as well:

            self.chan_params[i].currentChanged.connect(self._display_new_tab)

            # Configure each DSP tab. Module number is the dictionary key:
            # @todo (ASC 3/20/23): MyTabWidget does not keep a container with
            # the child widgets, so we use a C-style for loop indexed by j.
            # Can do something like add lists of widgets to the module and
            # channel dsp layouts and iterate over _those_ if something that
            # feels more Pythonic is desired.

            for j in range(len(self.chan_params[i])):
                tab = self.chan_params[i][j].widget()
                tab.configure(self.dsp_mgr, i)

                # Extra configuration for channel parameter widgets. Disable
                # CFD settings for 500 MSPS modules, hook up some tab-specific
                # signals e.g. adjust offsets:

                tab_name = self.chan_params[i].tabText(j)
                if tab_name == "AnalogSignal":
                    tab.b_adjust_offsets.clicked.connect(self.adjust_offsets)
                if tab_name == "MultCoincidence":
                    tab.b_print.clicked.connect(self.print_masks)
                if tab_name == "TimingControl":
                    tab.b_show_diagram.clicked.connect(self.show_diagram)

    def apply_dsp(self):
        """Apply the channel DSP settings for the selected tab.

        Updates internal DSP from the GUI values then writes the internal DSP
        to the module.
        """
        self._set_current_tab_info()

        reason = getattr(self.tab, "unsupported_reason", None)
        if reason:
            print(f"{self.tab_name}: {reason}. Nothing was applied.")
            return

        self.tab.update_dsp(self.dsp_mgr, self.mod_idx)

        self.toolbar.disable()
        _finished = [self.toolbar.enable]

        if self.tab_name == "AnalogSignal":
            self.tab.b_adjust_offsets.setEnabled(False)
            _finished.append(lambda: self.tab.b_adjust_offsets.setEnabled(True))

        self.pool_mgr.start_thread(
            fcn=lambda: self._write_chan_dsp(self.mod_idx, self.tab),
            results=[lambda: self.tab.display_dsp(self.dsp_mgr, self.mod_idx)],
            finished=_finished,
        )

    def load_dsp(self):
        """Load the channel DSP settings for the selected tab.

        Reads values from module into the internal DSP, then updates the GUI
        values from the internal DSP. Reconfigure the tab toolbar if necessary.
        """
        self._set_current_tab_info()

        reason = getattr(self.tab, "unsupported_reason", None)
        if reason:
            print(f"{self.tab_name}: {reason}. Nothing was read.")
            return

        self.toolbar.disable()
        _finished = [self.toolbar.enable]

        if self.tab_name == "AnalogSignal":
            self.tab.b_adjust_offsets.setEnabled(False)
            _finished.append(lambda: self.tab.b_adjust_offsets.setEnabled(True))

        self.pool_mgr.start_thread(
            fcn=lambda: self._read_chan_dsp(self.mod_idx, self.tab),
            results=[lambda: self.tab.display_dsp(self.dsp_mgr, self.mod_idx)],
            finished=_finished,
        )

    def copy_mod_dsp(self):
        """Copy DSP from one module to another."""
        self._set_current_tab_info()
        copy_mod = self.toolbar.copy_mod.value()
        if self.channel_map[self.mod_idx] != self.channel_map[copy_mod]:
            print(
                f"WARNING: Cannot copy parameter values from module with "
                f"{self.channel_map[copy_mod]} channels to module with "
                f"{self.channel_map[self.mod_idx]} channels!"
            )
        else:
            self.tab.display_dsp(self.dsp_mgr, self.toolbar.copy_mod.value())

    def copy_chan_dsp(self):
        """Copy DSP from one channel to all others on the same module."""
        self._set_current_tab_info()
        self.tab.copy_chan_dsp(self.toolbar.copy_chan.value())

    def adjust_offsets(self):
        """Adjust DC offsets for the selected module.

        Calls API function to automatically set DC offsets then updates GUI.
        """
        self._set_current_tab_info()
        self.tab.b_adjust_offsets.setEnabled(False)
        self.pool_mgr.start_thread(
            fcn=lambda: self.dsp_mgr.adjust_offsets(self.mod_idx),
            running=[self.toolbar.disable],
            results=[self.load_dsp],
            finished=[
                lambda: self.tab.b_adjust_offsets.setEnabled(True),
                self.toolbar.enable,
            ],
        )

    def print_masks(self):
        """Print the multiplicity and coincidence masks."""
        self._set_current_tab_info()
        self.tab.print_masks(self.dsp_mgr, self.mod_idx)

    def show_diagram(self):
        """Shows the timing diagram in a new window.

        If the diagram is already open do not open another one.
        """
        if not self.timing_diagram.isVisible():
            self.timing_diagram.show()

    def cancel(self):
        """Close the ChanDSPGUI window.

        Requests a close through Qt, which delivers a QCloseEvent to
        closeEvent() -- the same path the window [X] button takes.
        """
        self.close()

    def closeEvent(self, event):
        """Override default QWidget closeEvent to handle closing child windows.

        Checks to see if the timing diagram window is open, if so closes it
        and closes itself. Accepts the QCloseEvent.

        Parameters
        ----------
        event : QCloseEvent
            Signal to intercept, always accepted.
        """
        if self.timing_diagram.isVisible():
            self.timing_diagram.close()
        event.accept()

    ##
    # Private methods
    #

    def _set_current_tab_info(self):
        """Set current tab information: module idx, tab, tab name, tab index."""
        m = self.chan_params.currentIndex()
        p = self.chan_params[m].currentIndex()
        self.mod_idx = m
        self.par_idx = p
        self.tab = self.chan_params[m][p].widget()
        self.tab_name = self.chan_params[m].tabText(p)

    def _display_new_tab(self):
        """Display channel DSP from the dataframe when switching tabs.

        If a new module tab is selected, switch to the same channel DSP tab
        on the new module. Configure the toolbar for the new tab if necessary.
        """
        m = self.chan_params.currentIndex()  # Currently selected module.
        if m != self.mod_idx:
            self.chan_params[m].setCurrentIndex(self.par_idx)
        self._set_current_tab_info()
        self.tab.display_dsp(self.dsp_mgr, self.mod_idx)
        self._configure_toolbar(
            m,
            self.chan_params[self.mod_idx].tabText(self.par_idx),
        )

    def _configure_toolbar(self, mod, name):
        """Display tab-specific buttons. Set correct channel spinbox range if
        the module tab is changed.

        Parameters
        ----------
        mod : int
            Module index.
        name : str
            DSP parameter tab name.
        """
        self.toolbar.set_channel_spinbox_range(self.channel_map[mod])
        self.toolbar.set_visible(name)

    def _write_chan_dsp(self, mod, tab):
        """Write channel parameters to the dataframe for specified module.

        Parameters
        ----------
        mod : int
            Currently selected module tab index.
        tab : QWidget
            Currently selected DSP tab.
        """
        if tab.has_extra_params:
            self.dsp_mgr.write(mod, tab.extra_params)
        self.dsp_mgr.write(mod, tab.param_names)

    def _read_chan_dsp(self, mod, tab):
        """Read channel parameters from the dataframe for a specified module.

        Parameters
        ----------
        mod : int
            Currently selected module tab index.
        tab : QWidget
            Currently selected DSP tab.
        """
        if tab.has_extra_params:
            self.dsp_mgr.read(mod, tab.extra_params)
        self.dsp_mgr.read(mod, tab.param_names)
