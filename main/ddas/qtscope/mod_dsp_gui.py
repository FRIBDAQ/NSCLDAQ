import inspect
import logging

from PyQt5.QtCore import Qt
from PyQt5.QtGui import QCloseEvent
from PyQt5.QtWidgets import QMainWindow

from mod_dsp_layout import ModDSPLayout


class ModDSPGUI(QMainWindow):
    """Module DSP GUI class.

    DSP manager GUI for configuring module parameters. Module settings are
    displayed in group boxes of similar settings.

    Attributes
    ----------
    pool_mgr : ThreadPoolManager
        Global thread pool manager.
    mod_dsp_factory : ModDSPFactory
        Factory for implemented module DSP parameters.
    toolbar : DSPToolBar
        Toolbar for manipulating DSP settings.
    nmodules : int
        Number of installed modules in the crate.
    dsp : DSPParameters
        DSP parameter dictionary.
    mod_params : ModDSPLayout
        Layout of module DSP GUI and constituent parts.
    debug : bool
        True to enable debugging output.

    Methods
    -------
    configure(mgr, nmodules)
        Initialize module DSP layout.
    apply_dsp()
        Apply DSP settings for all modules and module parameters.
    load_dsp()
        Load DSP settings for all modules and module parameters.
    cancel()
        Close the manager window.
    closeEvent(event)
        Overridden QWidget closeEvent called by cancel().
    """

    def __init__(self, mod_dsp_factory, toolbar_factory, pool_mgr, *args, **kwargs):
        """ModDSPGUI class constructor.

        Parameters
        ----------
        mod_dsp_factroy (WidgetFactory):
            Factory for implemented module DSP parameters.
        toolbar_factory (ToolBarFactory):
            Factory for implemented toolbars.
        pool_mgr : ThreadPoolManager
            Management for global thread pool.
        """
        super().__init__(*args, **kwargs)

        self.setWindowTitle("Module DSP configuration")

        # Access to global thread pool for this applicaition:

        self.pool_mgr = pool_mgr

        ##
        # Main layout
        #

        self.mod_dsp_factory = mod_dsp_factory

        self.toolbar = toolbar_factory.create("dsp")
        self.toolbar.enable_mod_dsp()

        # Define layout:

        self.addToolBar(Qt.BottomToolBarArea, self.toolbar)

        ##
        # Signal connections
        #

        self.toolbar.b_apply.clicked.connect(self.apply_dsp)
        self.toolbar.b_load.clicked.connect(self.load_dsp)
        self.toolbar.b_cancel.clicked.connect(self.cancel)

    def configure(self, mgr, nmodules, channel_map):
        """Configure module DSP manager.

        Setup the toolbar, get the DSP, and create the top-level widget. This
        configure step must come after the system has been booted and the
        DSP storage has been initialized.

        Parameters
        ----------
        mgr : DSPManager
            Manager for internal DSP and interface for XIA API read/write
            operations.
        nmodules : int
            Number of installed modules.
        channel_map : list
            Map of channels on each module.
        """
        self.nmodules = nmodules
        self.dsp_mgr = mgr

        logging.getLogger("qtscope_logger").debug(
            f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: Configuring GUI for {self.nmodules} modules using {self.dsp_mgr}"
        )

        # Crate the module DSP layout and insert it on top of the
        # manager layout above the toolbar:

        self.mod_params = ModDSPLayout(self.mod_dsp_factory, self.nmodules, channel_map)
        self.setCentralWidget(self.mod_params)

        # Configure the module parameter widgets. Connect signals for the CSRB
        # and TrigConfig0 radio button quick config functions which need access
        # to the DSP.

        for w in self.mod_params.param_widgets:
            w.configure(self.dsp_mgr)
            name = type(w).__name__
            if name == "CSRB":
                csrb = w
                csrb.rbgroup.buttonClicked.connect(
                    lambda: csrb.set_param_grid(self.dsp_mgr)
                )
            if name == "TrigConfig0":
                tc0 = w
                tc0.rbgroup.buttonClicked.connect(
                    lambda: tc0.set_param_grid(self.dsp_mgr)
                )

    def apply_dsp(self):
        """Apply the module DSP settings.

        Updates internal DSP from the GUI values, then writes the internal DSP
        to the module.
        """
        for w in self.mod_params.param_widgets:
            w.update_dsp(self.dsp_mgr)

        self.pool_mgr.start_thread(
            fcn=self._write_mod_dsp,
            running=[self.toolbar.disable],
            results=[self._display_mod_dsp],
            finished=[self.toolbar.enable_mod_dsp],
        )

    def load_dsp(self):
        """Load the module DSP settings.

        Reads values from module into the internal DSP, then updates the GUI
        from the internal DSP.
        """
        self.pool_mgr.start_thread(
            fcn=self._read_mod_dsp,
            running=[self.toolbar.disable],
            results=[self._display_mod_dsp],
            finished=[self.toolbar.enable_mod_dsp],
        )

    def cancel(self):
        """Close the ModDSPGUI window.

        Ensure other opened windows are closed when the manager is closed
        whether the cancel button or the window [X] button is used by passing
        a QCloseEvent to an overridden QWidget closeEvent function.
        """
        self.closeEvent(QCloseEvent())

    def closeEvent(self, event):
        """Override default QWidget closeEvent to handle closing child windows.

        Checks to see if the CSRB or TrigConfig0 windows are open, if so closes
        them and closes itself. Accepts the QCloseEvent.

        Parameters
        ----------
        event : QCloseEvent
            Signal to intercept, always accepted.
        """
        self.pool_mgr.wait()

        for w in self.mod_params.param_widgets:
            name = type(w).__name__
            if (
                (name == "CSRB")
                or (name == "TrigConfig0")
                or (name == "TrigConfigExtra")
            ):
                w.grid.close()

        self.close()
        event.accept()

    ##
    # Private methods
    #

    def _write_mod_dsp(self):
        """Write module parameters to the dataframe for all modules."""
        for i in range(self.nmodules):
            for w in self.mod_params.param_widgets:
                self.dsp_mgr.write(i, w.param_names)

    def _read_mod_dsp(self):
        """Read module parameters from the dataframe for all modules."""
        for i in range(self.nmodules):
            for w in self.mod_params.param_widgets:
                self.dsp_mgr.read(i, w.param_names)

    def _display_mod_dsp(self):
        """Display module DSP parameters in the GUI."""
        for w in self.mod_params.param_widgets:
            w.display_dsp(self.dsp_mgr, set_state=True)
