import inspect
import os
import logging

from PyQt5.QtCore import Qt, QThreadPool
from PyQt5.QtGui import QCloseEvent, QPixmap
from PyQt5.QtWidgets import QMainWindow, QTabWidget, QVBoxLayout, QLabel

from chan_dsp_layout import ChanDSPLayout
from worker import Worker

# @todo Control draw width on widgets either by fixing sizes (not ideal) or by
# using separators which expand to fill the full window (better, probably).

class ChanDSPGUI(QMainWindow):
    """Channel DSP GUI class.

    DSP GUI for configuring channel parameters. Settings are dispalyed in a 
    nested tabbed widget where each module tab has a series of tabs assigned 
    to it corresponding to the channel DSP settings for that module.

    Attributes
    ----------
    pool : QThreadPool 
        Global thread pool.
    chan_params : QTabWidget 
        Tabbed widget of module DSP settings.
    chan_dsp_factory : WidgetFactory 
        Factory for implemented channel DSP widgets.
    toolbar : DSPToolBar 
        Toolbar for manipulating DSP settings.
    dsp_mgr : DSPManager 
        Manager for internal DSP and interface for XIA API read/write 
        operations.
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
    configure(dsp_manager, msps_list) 
        Initialize tabbed layout.
    apply_dsp() 
        Apply DSP settings for a given module and DSP settings.
    load_dsp() 
        Load DSP settings for a given module and DSP settings.
    copy_mod_par() 
        Copy DSP settings from another module.
    copy_chan_par() 
        Copy DSP settings from another channel on this module.
    adjust_offsets() 
        Adjust DC offsets for a single module.
    show_diagram() 
        Show coincidence timing help diagram descibing the settings.
    cancel() 
        Close the manager window.
    closeEvent(event) 
        Overridden QWidget closeEvent.
    """
    
    def __init__(self, chan_dsp_factory, toolbar_factory, *args, **kwargs):
        """ChanDSPGUI class constructor.

        Parameters
        ---------
        chan_dsp_factroy : WidgetFactory
            Factory for implemented channel DSP widgets.
        toolbar_factory : WidgetFactory
            Factory for implemented toolbar widgets.
        """        
        super().__init__(*args, **kwargs)
        
        self.setWindowTitle("Channel DSP manager")
        
        # Access to global thread pool for this applicaition:
        
        self.pool = QThreadPool.globalInstance()
        
        ##
        # Main layout
        #
        
        self.chan_params = QTabWidget()
        self.chan_dsp_factory = chan_dsp_factory
        
        self.toolbar = toolbar_factory.create("dsp")

        self.addToolBar(Qt.BottomToolBarArea, self.toolbar)
        self.setCentralWidget(self.chan_params)

        # Timing diagram. QLabel has a Pixmap, so we'll use that to display
        # the diagram and add some padding to the edges of the image because
        # its very tightly cropped:

        fig = QPixmap(str(os.environ.get("DAQROOT"))
                      + "/ddas/qtscope/figures/timing_diagram.png")
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

        # @todo This appears to load twice. Really want to reload parameters
        # on _currently displayed_ widget when we switch _any_ widget, module
        # or not. Try to do with a single function or in a way that doesn't
        # load multiple times.
        
        self.chan_params.currentChanged.connect(self._display_new_tab)
        
    def configure(self, dsp_manager, msps_list):
        """Configure channel DSP manager.

        Setup the toolbar, get the DSP, and create the tabbed widget. This 
        configure step must come after the system has been booted and the 
        DSP storage has been initialized.

        Parameters
        ----------
        dsp_manager : DSPManager
            Manager for internal DSP and interface for XIA API read/write 
            operations.
        msps_list : list
            List of module ADC MSPS values.
        """        
        self.dsp_mgr = dsp_manager

        # Length of msps_list == number of modules:
        logging.getLogger("qtscope_logger").debug(
            "{}.{}: Configuring GUI for {} modules using {}".format(
                self.__class__.__name__,
                inspect.currentframe().f_code.co_name,
                len(msps_list),
                self.dsp_mgr
            )
        )

        # Configure toolbar:   
        self.toolbar.copy_mod.setRange(0, len(msps_list)-1)
        self.toolbar.copy_chan.setRange(0, 15)

        # Initialize tab indices and widget:        
        self.mod_idx = 0
        self.par_idx = 0
        self.tab = None
        self.tab_name = ""
        
        for i, msps in enumerate(msps_list):                  
            # DSP tab layout for each module in the system:            
            self.chan_params.insertTab(
                i, ChanDSPLayout(self.chan_dsp_factory, i), "Mod. %i" %i
            )
            # DSP tabs load from dataframe when switching. Just added the
            # module tabbed widget so add the signal here as well:            
            self.chan_params.widget(i).currentChanged.connect(
                self._display_new_tab
            )                     
            # Configure each DSP tab. Module number is the dictionary key:
            # @todo (ASC 3/20/23): QTabWidget does not keep a container with
            # the child widgets, so we use a C-style for loop indexed by j.
            # Can do something like add lists of widgets to the module and
            # channel dsp layouts and iterate over _those_ if something that
            # feels more Pythonic is desired.            
            for j in range(self.chan_params.widget(i).count()):                 
                tab = self.chan_params.widget(i).widget(j)
                tab.configure(self.dsp_mgr, i)
                # Extra configuration for channel parameter widgets. Disable
                # CFD settings for 500 MSPS modules, connect signals for
                # adjusting DC offsets and dispalying the timing diagram.
                tab_name = self.chan_params.widget(i).tabText(j)
                if tab_name == "CFD" and msps == 500:
                    tab.disable_settings() 
                if tab_name == "AnalogSignal":
                    tab.b_adjust_offsets.clicked.connect(self.adjust_offsets)
                if tab_name == "TimingControl":
                    tab.b_show_diagram.clicked.connect(self.show_diagram)

    def apply_dsp(self):
        """Apply the channel DSP settings for the selected tab.

        Updates internal DSP from the GUI values, starts a new thread, then 
        writes the internal DSP storage to the module.
        """        
        self._get_current_tab()        
        self.tab.update_dsp(self.dsp_mgr, self.mod_idx)
        worker = Worker(
            lambda m=self.mod_idx, t=self.tab: self._write_chan_dsp(m, t)
        )
        if self.tab_name == "AnalogSignal":
            worker.signals.running.connect(
                lambda enb=False: self.tab.b_adjust_offsets.setEnabled(enb)
            )
        worker.signals.running.connect(self.toolbar.disable)
        worker.signals.finished.connect(
            lambda mgr=self.dsp_mgr, m=self.mod_idx:
            self.tab.display_dsp(mgr, m)
        )
        if self.tab_name == "AnalogSignal":
            worker.signals.finished.connect(
                lambda enb=True: self.tab.b_adjust_offsets.setEnabled(enb)
            )
        worker.signals.finished.connect(self.toolbar.enable)
        self.pool.start(worker)
        
    def load_dsp(self):
        """Load the channel DSP settings for the selected tab.

        Accesses a thread from the global thread pool, reads values from module
        into the internal DSP, then updates the GUI values from the internal 
        DSP. Reconfigure the tab toolbar if necessary.
        """
        self._get_current_tab()            
        worker = Worker(
            lambda m=self.mod_idx, t=self.tab: self._read_chan_dsp(m, t)
        )
        if self.tab_name == "AnalogSignal":
            worker.signals.running.connect(
                lambda enb=False: self.tab.b_adjust_offsets.setEnabled(enb)
            )
        worker.signals.running.connect(self.toolbar.disable)
        worker.signals.finished.connect(
            lambda mgr=self.dsp_mgr, m=self.mod_idx:
            self.tab.display_dsp(mgr, m)
        )
        if self.tab_name == "AnalogSignal":
            worker.signals.finished.connect(
                lambda enb=True: self.tab.b_adjust_offsets.setEnabled(enb)
            )
        worker.signals.finished.connect(self.toolbar.enable)
        self.pool.start(worker)
    
    def copy_mod_dsp(self):
        """Copy DSP from one module to another."""        
        self._get_current_tab()
        self.tab.display_dsp(self.dsp_mgr, self.toolbar.copy_mod.value())
            
    def copy_chan_dsp(self):
        """Copy DSP from one channel to all others on the same module."""
        self._get_current_tab()
        cchan_idx = self.toolbar.copy_chan.value() # Copy from here.
        self.tab.copy_chan_dsp(cchan_idx)

    def adjust_offsets(self):
        """Adjust DC offsets for the selected module.

        Accesses a thread from the global thread pool, calls the API function 
        to adjust offsets, updates the dataframe and the GUI.
        """
        self._get_current_tab()        
        worker = Worker(lambda m=self.mod_idx: self.dsp_mgr.adjust_offsets(m))
        worker.signals.running.connect(
            lambda enb=False: self.tab.b_adjust_offsets.setEnabled(enb)
        )
        worker.signals.running.connect(self.toolbar.disable)
        worker.signals.finished.connect(self.load_dsp)
        worker.signals.finished.connect(
            lambda enb=True: self.tab.b_adjust_offsets.setEnabled(enb)
        )
        worker.signals.finished.connect(self.toolbar.enable)
        self.pool.start(worker)     

    def show_diagram(self):
        """Shows the timing diagram in a new window. 

        If the diagram is already open do not open another one.
        """                
        if not self.timing_diagram.isVisible():
            self.timing_diagram.show()
        
    def cancel(self):
        """Close the ChanDSPGUI window.

        Ensure other opened windows are closed when the manager is closed 
        whether the cancel button or the window [X] button is used by passing 
        a QCloseEvent to an overridden QWidget closeEvent function.
        """        
        self.closeEvent(QCloseEvent())

    def closeEvent(self, event):
        """Override default QWidget closeEvent to handle closing child windows.
        
        Checks to see if the timing diagram window is open, if so closes it 
        and closes itself. Accepts the QCloseEvent.

        Parameters
        ----------
        event : QCloseEvent
            Signal to intercept, always accepted.
        """        
        self.pool.waitForDone(10000)
        if self.timing_diagram.isVisible():
            self.timing_diagram.close()
        self.close()
        event.accept()

    ##
    # Private methods
    #

    def _get_current_tab(self):
        """Get the current module, parameter index and tab widget."""        
        self.mod_idx = self.chan_params.currentIndex()
        self.par_idx = self.chan_params.widget(self.mod_idx).currentIndex()
        self.tab = self.chan_params.widget(self.mod_idx).widget(self.par_idx)
        self.tab_name = self.chan_params.widget(self.mod_idx).tabText(self.par_idx)
        
    def _display_new_tab(self):
        """Display channel DSP from the dataframe when switching tabs. 

        If a new module tab is selected, switch to the same channel DSP tab 
        on the new module. Configure the toolbar for the new tab if necessary.
        """
        m = self.chan_params.currentIndex()  # Currently selected module
        if m != self.mod_idx:
            self.chan_params.widget(m).setCurrentIndex(self.par_idx)
        self._get_current_tab()
        self.tab.display_dsp(self.dsp_mgr, self.mod_idx)        
        self._configure_toolbar(
            self.chan_params.widget(self.mod_idx).tabText(self.par_idx)
        ) 
    
    def _configure_toolbar(self, name):
        """Display tab-specific buttons.

        Parameters
        ----------
        name : str
            DSP parameter tab name.
        """        
        if name == "MultCoincidence":
            self.toolbar.copy_chan_action.setVisible(False)
            self.toolbar.copy_chan_sb_action.setVisible(False)
        else:
            self.toolbar.copy_chan_action.setVisible(True)
            self.toolbar.copy_chan_sb_action.setVisible(True)

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
